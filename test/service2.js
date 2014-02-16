var ESB = require('esb-node-driver');
var esb = new ESB({
	publisherPort: 7782
});

esb.on('ready', function(){
	console.log('ESB service 2 is ready');
	esb.register('/math/plus', 1, function(data, cb){
		//console.log(process.pid+' 1')
		cb(null, data.a + data.b);
	});
	esb.register('/math/plus', 1, function(data, cb){
		//console.log(process.pid+' 2')
		cb(null, data.a + data.b);
	});
	esb.register('/math/plus', 1, function(data, cb){
		//console.log(process.pid+' 3')
		cb(null, data.a + data.b);
	});
	
	setTimeout(function(){
		process.send({});
	}, 1000);
});

setInterval(function(){},1000)

process.on('exit quit', function(){
	console.log('exit event in service2, quit');
	process.exit();
});