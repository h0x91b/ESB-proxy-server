var ESB = require('../../Node/NodeJS/main.js');
var esb = new ESB({
	publisherPort: 7781
});

esb.on('ready', function(){
	console.log('ESB service is ready');
	esb.register('/math/plus', 1, function(data, cb){
		cb(null, data.a + data.b);
	});
	
	setTimeout(function(){
		process.send({});
	}, 1000);
});

setInterval(function(){},1000)