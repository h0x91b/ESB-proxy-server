var fs = require('fs');
var Proxy;

if(fs.existsSync(__dirname+'/build/Debug/proxy.node'))
	Proxy = require('./build/Debug/proxy');
else
	Proxy = require('./build/Release/proxy');

var p = new Proxy.Proxy('plt-esb01', 'esb-redis', 6379);

setInterval(function(){}, 1000); //main loop must not be empty...