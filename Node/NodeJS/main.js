var extend = require('extend');
var zmq = require('zmq');
var uuid = require('uuid');

var _config = {
	host: 'localhost',
	port: 7770
};

function ESB(config) {
	this.config = extend(true, {}, _config, config);
	this.guid = uuid.v4();
	console.log('new ESB %s', this.guid);
	this.responseCallbacks = [];
	var socket = zmq.socket('req');
	this.requestSocket = socket;
	
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
		var fs = require("fs")
		var proto = require("node-protobuf").Protobuf;
		var pb = new proto(fs.readFileSync(__dirname+"/../../Proxy/src/command.desc"));
		var cmdGuid = ('{'+uuid.v4()+'}').toUpperCase();
		
		var obj = {
			cmd: 'NODE_HELLO',
			payload: 'tcp://127.0.0.1:7780',
			guid_from: cmdGuid
		}
		var buf = pb.Serialize(obj, "ESB.Command") // you get Buffer here, send it via socket.write, etc.
		var self = this;
		this.requestSocket.once('message', function(data){
			self.requestSocket.close();
			var respObj = pb.Parse(data, "ESB.Command");
			console.log('got response from Proxy', respObj);
			if(respObj.cmd === 'ERROR') {
				throw new Error(respObj.payload);
			}
			var t = respObj.payload.split('#');
			console.log(t);
		});
		this.requestSocket.send(buf);
	}
};

module.exports = ESB;
