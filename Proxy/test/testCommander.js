require('../main.js');
var zmq = require('zmq');
var uuid = require('uuid');

var socket = zmq.socket('req');


var socket = zmq.socket('req');

socket.identity = 'client' + process.pid;

socket.connect('tcp://127.0.0.1:7770');
console.log('connected!');

var fs = require("fs")
var proto = require("node-protobuf").Protobuf;
// WARNING: next call will throw if desc file is invalid
var pb = new proto(fs.readFileSync(__dirname+"/../src/command.desc"));

var responseCallbacks = {};

setInterval(function() {
	var value = Math.floor(Math.random()*100);

	//socket.send('hello'+value);
	console.log(socket.identity + ': asking ');
	var guid = ('{'+uuid.v4()+'}').toUpperCase();
	
	var obj = {
		cmd: 'NODE_HELLO',
		payload: 'hello world',
		guid_from: guid
	}
	console.log('Serialize', obj);
	try {
		var buf = pb.Serialize(obj, "ESB.Command") // you get Buffer here, send it via socket.write, etc.
		console.log('buf length: %s', buf.length);
		responseCallbacks[guid] = function(err, data, rawObj){
			console.log('received response from Proxy', err, data, rawObj );
		}
		socket.send(buf);
		
	} catch (e) {
		console.log('exception', e);
	}
	
	
}, 2500);

socket.on('message', function(data) {
	console.log(socket.identity + ': answer data ' + data);
	try {
		var respObj = pb.Parse(data, "ESB.Command");
		//console.log('respObj', respObj);
		var err = null;
		var data = respObj.payload;
		if(respObj.cmd === 'ERROR') {
			err = data;
		}
		if(!responseCallbacks[respObj.guid_to])
		{
			console.log('unmanaged response...', respObj);
		}
		var fn = responseCallbacks[respObj.guid_to];
		responseCallbacks[respObj.guid_to] = null;
		fn(err, data, respObj);
	} catch (e) {
		console.log('exception on message', e);
	}
});