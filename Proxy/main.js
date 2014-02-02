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
var redis = require("redis"),
		redisClient = redis.createClient();

var os=require('os');

if (argv.help) {
	require('optimist').showHelp();
	process.exit(0);
}

var Proxy = require('./build/Debug/proxy');
var uuid = require('uuid');
var guid = uuid.v4();

var p = new Proxy.Proxy({
	commanderPort: argv.c,
	publisherPort: argv.p,
	guid: guid,
	addMeToRegistry: addMeToRegistry
});

function addMeToRegistry() {
	var ifaces=os.networkInterfaces();
	var ips = 0;
	for (var dev in ifaces) {
		ifaces[dev].forEach(function(details){
			if (details.family=='IPv4' && details.internal === false) {
				var connectStr = guid+'#tcp://'+details.address+':'+argv.c;
				redisClient.zadd('ZSET:PROXIES',~~(+new Date/1000), connectStr);
				ips++;
			}
		});
	}
	console.log('renew %s ips for this proxy', ips);
}

//setInterval(function(){}, 1000); //wait fore
