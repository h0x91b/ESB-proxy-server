var Proxy = require('./build/Release/proxy');

console.log('proxy', Proxy);

var p = new Proxy.Proxy(1);
function a() {
	
}

setInterval(a, 1000);