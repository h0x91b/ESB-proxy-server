var extend = require('extend');
var zmq = require('zmq');
var uuid = require('uuid');
var fs = require("fs")
var proto = require("node-protobuf").Protobuf;
var pb = new proto(fs.readFileSync(__dirname+"/../../Proxy/src/command.desc"));


var _config = {
	host: 'localhost',
	port: 7770,
	publisherHost: 'localhost',
	publisherPort: 7780
};

function ESB(config) {
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

ESB.prototype = {
	connect: function(){
		var connectStr = 'tcp://'+this.config.host+':'+this.config.port
		console.log('ESB %s connecting to: %s', this.guid, connectStr);
		this.requestSocket.connect(connectStr);
		console.log('ESB %s connected', this.guid);
		this.sendHello();
	},
	sendHello: function() {
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
		});
		this.requestSocket.send(buf);
	},
	onMessage: function(data) {
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
			default:
				console.log("unknown operation", respObj.cmd);
			}
		} catch(e){
			console.log('ERROR while processing message', e);
		}
	}
};

module.exports = ESB;
