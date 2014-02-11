var PROXY = require('../main.js');
var ESB = require('../../Node/NodeJS/main.js');

var responses = 0;

var esb1 = new ESB({
	publisherPort: process.argv[2] || 7781
});
esb1.on('ready', function(){
	console.log('esb1 is ready');
	esb1.register('/math/plus', 1, function(data, cb){
		cb(null, data.a + data.b);
	});
});

var esb2 = new ESB({
	publisherPort: process.argv[3] || 7782
});
esb2.on('ready', function(){
	console.log('esb2 is ready');
	esb2.register('/math/plus', 1, function(data, cb){
		cb(null, data.a + data.b);
	});
	setInterval(function(){
	for(var i=0;i<250;i++)
		esb2.invoke('/math/plus', {a: 2, b: 2}, function(err, resp, errStr){
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
	console.log('%s per second', responses);
	responses=0;
},1000);
