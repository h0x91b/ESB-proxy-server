var argv = require('optimist')
	.usage('ESB')
	.options('c', {
		alias: 'commander-port',
		default: 7770,
		describe: 'Commander port'
	})
    .options('p', {
         alias: 'publisher-port',
         default: 7771,
         describe: 'Publisher port'
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
var p = new Proxy.Proxy({
    commanderPort: argv.c,
    publisherPort: argv.p
});

function a() {
	
}

setInterval(a, 1000);