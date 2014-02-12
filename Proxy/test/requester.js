var PROXY = require('../main.js');
var ESB = require('../../Node/NodeJS/main.js');

responses = 0;

var esb = new ESB({
	publisherPort: process.argv[3] || 7782
});

esb.on('ready', function(){
	console.log('esb is ready');
	setInterval(function(){
	for(var i=0;i<1;i++)
		esb.invoke('/math/plus', {a: 2, b: 2}, function(err, resp, errStr){
			if(err){
				console.log(err, errStr);
				return;
			}
			console.assert(resp == 4);
			responses++;
		});
	},500);
});

setInterval(function(){
	console.log('%s invokes per second', responses);
	responses=0;
},1000);
