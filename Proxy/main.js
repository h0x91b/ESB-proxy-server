var Proxy = require('./build/Release/proxy');

console.log('proxy', Proxy);

var p = new Proxy.Proxy(5555);



function a() {
	
}

setInterval(a, 1000);