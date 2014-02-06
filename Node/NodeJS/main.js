var extend = require('extend');
var zmq = require('zmq');
var uuid = require('uuid');
var fs = require("fs")
var proto = require("node-protobuf").Protobuf;
var pb = new proto(fs.readFileSync(__dirname+"/../../Proxy/src/command.desc"));
var util = require("util");
var events = require("events");


var _config = {
	host: 'localhost',
	port: 7770,
	publisherHost: 'localhost',
	publisherPort: 7780
};

function ESB(config) {
	events.EventEmitter.call(this);
	this.config = extend(true, {}, _config, config);
	this.guid = ('{'+uuid.v4()+'}').toUpperCase();
	this.proxyGuid = '';
	console.log('new ESB %s', this.guid);
	this.responseCallbacks = [];
	this.invokeMethods = [];
	var socket = zmq.socket('req');
	this.requestSocket = socket;
	this.requestSocket.on('error', function(err){
		console.log('requestSocket error', err);
	});
	this.subscribeSocket = zmq.socket('sub');
	this.subscribeSocket.on('error', function(err){
		console.log('subscribeSocket error', err);
	});
	this.publisherSocket = zmq.socket('pub');
	this.publisherSocket.on('error', function(err){
		console.log('publisherSocket error', err);
	});
	this.publisherSocket.bindSync('tcp://*:'+this.config.publisherPort);
	
	this.connect();
}

util.inherits(ESB, events.EventEmitter);


ESB.prototype.connect= function(){
	var connectStr = 'tcp://'+this.config.host+':'+this.config.port
	console.log('ESB %s connecting to: %s', this.guid, connectStr);
	this.requestSocket.connect(connectStr);
	console.log('ESB %s connected', this.guid);
	this.sendHello();
};

ESB.prototype.sendHello= function() {
	var cmdGuid = ('{'+uuid.v4()+'}').toUpperCase();
	
	var obj = {
		cmd: 'NODE_HELLO',
		payload: this.guid+'#tcp://'+this.config.publisherHost+':'+this.config.publisherPort,
		guid_from: cmdGuid
	}
	var buf = pb.Serialize(obj, "ESB.Command");
	var self = this;
	this.requestSocket.once('message', function(data){
		self.requestSocket.close();
		var respObj = pb.Parse(data, "ESB.Command");
		console.log('got response from Proxy', respObj);
		if(respObj.cmd === 'ERROR') {
			throw new Error(respObj.payload);
		}
		var t = respObj.payload.split('#');
		t = 'tcp://'+t[0]+':'+t[1];
		console.log('connecting to: '+t);
		self.subscribeSocket.on('message', function(data){
			self.onMessage.call(self, data);
		});
		self.subscribeSocket.connect(t);
		console.log('connected');
		self.subscribeSocket.subscribe(self.guid);
		self.proxyGuid = respObj.source_proxy_guid;
		
		self.emit('ready');
	});
	this.requestSocket.send(buf);
	console.log('NODE_HELLO sended');
};

ESB.prototype.onMessage= function(data) {
	try {
		//console.log('ESB.prototype.onMessage', data);
		data = data.slice(38); //38 sizeof guid in bytes
		var respObj = pb.Parse(data, "ESB.Command");
		//console.log('suscriber got message: ', respObj);
		switch(respObj.cmd)
		{
		case 'PING':
			console.log('got PING request');
			//
			var obj = {
				cmd: 'PONG',
				payload: +new Date,
				guid_to: respObj.guid_from,
				guid_from: this.guid,
			}
			var buf = pb.Serialize(obj, "ESB.Command");
			this.publisherSocket.send(this.guid+buf);
			break;
		case 'ERROR':
			console.log('got ERROR response');
			if(this.responseCallbacks[respObj.guid_to]){
				var fn = this.responseCallbacks[respObj.guid_to];
				fn(respObj.cmd, null, respObj.payload);
			}
			break;
		case 'REGISTER_INVOKE_OK':
			console.log("REGISTER_INVOKE_OK for %s", respObj.payload);
			break;
		case 'INVOKE':
			//console.log('got INVOKE request');
			var fn = this.invokeMethods[respObj.guid_to];
			if(!fn) {
				console.log('can not find such invoke method', respObj, Object.keys(this.invokeMethods));
				break;
			}
			fn(respObj);
			break;
		case 'RESPONSE':
			//console.log('got RESPONSE');
			var fn = this.responseCallbacks[respObj.guid_to];
			if(fn){
				fn(null, JSON.parse(respObj.payload), null);
				delete this.responseCallbacks[respObj.guid_to];
			} else {
				console.log('callback %s for response not found', respObj.guid_to);
			}
			break;
		default:
			console.log("unknown operation", respObj);
		}
	} catch(e){
		console.log('ERROR while processing message', e);
	}
};

ESB.prototype.invoke = function(identifier, data, cb, options){
	options = extend(true, {
		version: 1,
		timeout: 3000
	}, options);
	identifier = identifier+'/v'+options.version;
	//console.log('invoke()', identifier, options, data);
	var isCalled = false;
	var id = null;
	var self = this;
	if(options.timeout>0){
		id = setTimeout(timeoutCb, options.timeout);
		function timeoutCb(){
			if(isCalled) return;
			isCalled = true;
			delete self.responseCallbacks[cmdGuid];
			cb('Timeout', null, 'Timeout triggered by nodeConnector');
		};
	} else
		options.timeout = 0;
	
	var cmdGuid = ('{'+uuid.v4()+'}').toUpperCase();
	//console.log('invoke temp guid', cmdGuid);
	
	this.responseCallbacks[cmdGuid] = function(err, data, errString){
		if(isCalled){
			console.log('got response from Proxy, but callback already was called');
			return;
		}
		isCalled = true;
		if(id) clearTimeout(id);
		delete self.responseCallbacks[cmdGuid];
		//console.log('call response callback');
		cb(err, data, errString);
	}
	
	try {
		var obj = {
			cmd: 'INVOKE',
			identifier: identifier,
			payload: JSON.stringify(data, null, '\t'),
			guid_from: cmdGuid,
			target_proxy_guid: '',
			source_proxy_guid: this.guid,
			start_time: +new Date,
			timeout_ms: options.timeout
		}
		var buf = pb.Serialize(obj, "ESB.Command");
		this.publisherSocket.send(this.guid+buf)
	} catch(e){
		isCalled = true;
		if(id) clearTimeout(id);
		delete self.responseCallbacks[cmdGuid];
		cb('Exception', null, 'Exception while encoding/sending message: '+e.toString());
	}
	
	return cmdGuid;
};

ESB.prototype.register = function(identifier, version, cb, options) {
	console.log('register', identifier, version);
	options = extend(true, {
		version: 1,
		timeout: 3000
	}, options);
	identifier = identifier+'/v'+options.version;
	
	var cmdGuid = ('{'+uuid.v4()+'}').toUpperCase();
	console.log('registerInvoke guid:',cmdGuid);
	var self = this;
	this.invokeMethods[cmdGuid] = function(data){
		//console.log('invoke method ', cmdGuid, data);
		cb(JSON.parse(data.payload), function(err, resp){
			//console.log('got response from method...', err, resp);
			var obj = {
				cmd: 'RESPONSE',
				payload: JSON.stringify(resp, null, '\t'),
				guid_from: cmdGuid,
				guid_to: data.guid_from,
				target_proxy_guid: data.source_proxy_guid,
				source_proxy_guid: self.guid,
				start_time: +new Date,
			}
			
			try {
				if(err) {
					obj.cmd = 'ERROR';
					obj.payload = err;
				}
				//console.log('invoke response',obj);
				var buf = pb.Serialize(obj, "ESB.Command");
				self.publisherSocket.send(self.guid+buf);
			} catch(e){
				cb('Exception', null, 'Exception while encoding/sending message: '+e.toString(), resp);
			}
			
		});
	};
	
	try {
		var obj = {
			cmd: 'REGISTER_INVOKE',
			identifier: identifier,
			payload: cmdGuid,
			guid_from: cmdGuid,
			target_proxy_guid: '',
			source_proxy_guid: this.guid,
			start_time: +new Date,
		}
		console.log('register',obj);
		var buf = pb.Serialize(obj, "ESB.Command");
		this.publisherSocket.send(this.guid+buf);
	} catch(e){
		cb('Exception', null, 'Exception while encoding/sending message: '+e.toString());
	}
	
	return cmdGuid;
};

module.exports = ESB;
