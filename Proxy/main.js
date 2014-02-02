var argv = require('optimist')
	.usage('ESB')
	.options('p', {
		alias: 'port',
		default: 7770,
		describe: 'Commander port'
	})
	.options('h', {
		alias: 'help'
	})
	.argv;

if (argv.help) {
	require('optimist').showHelp();
	process.exit(0);
}

var Proxy = require('./build/Debug/proxy');
var p = new Proxy.Proxy(argv.port);

function a() {
	
}

setInterval(a, 1000);