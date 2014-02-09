var PROXY = require('../main.js');
var ESB = require('../../Node/NodeJS/main.js');
var fork = require('child_process').fork;

var ch = fork('service.js');
fork('service2.js');

ch.on('message', function(m) {
	console.log('service ready');

	var esb = new ESB;
	esb.on('ready', function(){
		console.log('ESB ready for use');
		
		esb.invoke('/dfsdfdsfdsf', {a: 1}, function(err, resp, errStr){
			console.log(errStr);
		});
		
		var start = +new Date;
		var cnt = 50000;
		var total = cnt;
		var errors = 0;
		function doit(){
			process.nextTick(function(){
				for(var i=0;i<250;i++){
					(function(){
						var a = ~~(Math.random()*50);
						var b = ~~(Math.random()*50);
						esb.invoke('/math/plus', {a: a, b: b}, function(err, resp, errStr){
							if(err){
								console.log('Error on invoke (%s): %s', err, errStr);
								errors++;
								if(--total == 0) {
									console.log('the end, %s ms, %s errors, speed %s ops/sec', +new Date-start, errors, cnt/(+new Date-start)*1000);
									//process.exit();
								}
								return;
							}
							//console.log('%s+%s= %s', a, b, resp);
							console.assert(resp == a+b);
							if(--total == 0) {
								console.log('the end, %s ms, %s errors, speed %s ops/sec', +new Date-start, errors, cnt/(+new Date-start)*1000);
								//process.exit();
							}
							if(total % 500 == 0){
								console.log("processed %s, remain: %s",cnt-total, total);
							}
						}, {timeout: 60000});
					})();
				}
			});
			if(total>0) setTimeout(doit, 15);
		}
		doit();
		// setInterval(function(){
		// 	esb.invoke('/math/plus', {a: 2, b: 2}, function(err, resp, errStr){
		// 		console.log('aaaaa', err, resp, errStr);
		// 	});
		// },20);
	});
});

setInterval(function(){},1000);

process.on('exit quit', function(){
	console.log('exit event in testCommander, quit');
	process.exit();
});