var PROXY = require('../main.js');
var ESB = require('../../Node/NodeJS/main.js');

var esb = new ESB;


esb.on('ready', function(){
	console.log('ESB ready for use');
});