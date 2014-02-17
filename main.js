//var Proxy = require('./build/Release/proxy');
var Proxy = require('./build/Debug/proxy');

var p = new Proxy.Proxy('plt-esb01', 'esb-redis', 6379);

setInterval(function(){}, 1000); //main loop must nit be empty...