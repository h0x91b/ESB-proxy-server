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
	console.log('new ESB %s', this.guid);
	this.responseCallbacks = [];
	var socket = zmq.socket('req');
	this.requestSocket = socket;
	this.subscribeSocket = zmq.socket('sub');
	this.publisherSocket = zmq.socket('pub');
	this.publisherSocket.bind('tcp://*:'+this.config.publisherPort);
	
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
		
		self.emit('ready');
	});
	this.requestSocket.send(buf);
};

ESB.prototype.onMessage= function(data) {
	try {
		console.log('data', data);
		data = data.slice(38); //38 sizeof guid in bytes
		var respObj = pb.Parse(data, "ESB.Command");
		console.log('suscriber got message: ', respObj);
		switch(respObj.cmd)
		{
		case 'PING':
			console.log('got PING request');
			//
			var obj = {
				cmd: 'PONG',
				payload: ~~(+new Date/1000),
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
		default:
			console.log("unknown operation", respObj.cmd);
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
	console.log('invoke()', identifier, options, data);
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
	
	this.responseCallbacks[cmdGuid] = function(err, data, errString){
		if(isCalled){
			console.log('got response from Proxy, but callback already was called');
			return;
		}
		isCalled = true;
		if(id) clearTimeout(id);
		delete self.responseCallbacks[cmdGuid];
		cb(err, data, errString);
	}
	
	try {
		var obj = {
			cmd: 'INVOKE',
			identifier: identifier,
			payload: data,
			guid_from: cmdGuid,
			target_proxy_guid: '',
			source_proxy_guid: this.guid,
			start_time: ~~(+new Date/1000),
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
}

module.exports = ESB;
