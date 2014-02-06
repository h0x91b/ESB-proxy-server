var PROXY = require('../main.js');
var ESB = require('../../Node/NodeJS/main.js');

var esb = new ESB;


esb.on('ready', function(){
	console.log('ESB ready for use');
	
	esb.register('/math/plus', 1, function(data, cb){
		cb(null, data.a + data.b);
	});
	
	esb.invoke('/math/plus', {a: 2, b: 2}, function(err, resp, errStr){
		if(err){
			console.log('Error on invoke (%s): %s', err, errStr);
		}
		console.log('2+2=%s', resp);
	});
});