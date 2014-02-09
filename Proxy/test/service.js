var ESB = require('../../Node/NodeJS/main.js');
var esb = new ESB({
	publisherPort: 7781
});

esb.on('ready', function(){
	console.log('ESB service is ready');
	for(var i=0;i<10;i++){
		(function(i){
			esb.register('/math/plus', 1, function(data, cb){
				console.log('/math/plus #'+i+' call');
				cb(null, data.a + data.b);
			});
		})(i);
	}
	
	setTimeout(function(){
		process.send({});
	}, 1000);
});

setInterval(function(){},1000)

process.on('exit quit', function(){
	console.log('exit event in service, quit');
	process.exit();
});