var PROXY = require('../main.js');
var ESB = require('../../Node/NodeJS/main.js');

var esb = new ESB;


esb.on('ready', function(){
	console.log('ESB ready for use');
	
	esb.invoke('/math/plus', {a: 2, b: 2}, function(err, resp, errStr){
		if(err){
			console.log('Error on invoke (%s): %s', err, errStr);
		}
	});
});