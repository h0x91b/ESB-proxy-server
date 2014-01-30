require('../main.js');
var zmq = require('zmq');

var socket = zmq.socket('req');


var socket = zmq.socket('req');

socket.identity = 'client' + process.pid;

socket.connect('tcp://127.0.0.1:5555');
console.log('connected!');

setInterval(function() {
	var value = Math.floor(Math.random()*100);

	socket.send('hello'+value);
	console.log(socket.identity + ': asking ' + value);
}, 100);

socket.on('message', function(data) {
	console.log(socket.identity + ': answer data ' + data);
});