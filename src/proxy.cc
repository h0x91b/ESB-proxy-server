#include "globals.h"
#include "proxy.h"
#include <ctime>

using namespace v8;

void InitAll(Handle<Object> exports) {
	Proxy::Init(exports);
}

Persistent<Function> Proxy::constructor;

Proxy::Proxy(const v8::Arguments& args)
{
	srand(mix(clock(), time(NULL), getpid()));
	GenerateGuid(guid, GUID_SIZE);
	info("guid: %s", guid);
	
	invokeCalls = 0;
	invokeErrors = 0;
	registerInvoke = 0;
	lastRedisPing = 0;
	lastRegistryExchange = 0;
	
	redisPort = 6379;
	responderPort = 7770;
	publisherPort = 7771;

	v8::String::Utf8Value esbhost(args[0]->ToString());
	strcpy(host, *esbhost);
	
	v8::String::Utf8Value redishost(args[1]->ToString());
	strcpy(redisHost, *redishost);
	
	redisPort = args[2]->ToUint32()->Value();
	info("Init new ESB %s on host '%s' with redis '%s:%lu'", guid, host, redisHost, redisPort);
	
	responder = new Responder(responderPort, guid, this);
	responderPort = responder->port;
	
	publisher = new Publisher(publisherPort);
	publisherPort = publisher->port;
	
	redisCtx = redisConnect(redisHost, redisPort);
	if (redisCtx != NULL && redisCtx->err) {
		err("Redis connection error: %s", redisCtx->errstr);
		redisCtx = NULL;
	}
	isWork = true;
	pthread_create(&thread, NULL, &MainLoop, this);
#ifdef __linux__
	pthread_setname_np(thread, "MainLoop of ESB");
#endif
}

void Proxy::ProxyHello(ESB::Command &cmdReq, ESB::Command &cmdResp)
{
	info("Received hello from another proxy");
	char response[256];
	sprintf(response, "tcp://%s:%i", host, publisherPort);
	cmdResp.set_payload(response);
	cmdResp.set_cmd(ESB::Command::RESPONSE);
}

void Proxy::NodeHello(ESB::Command &cmdReq, ESB::Command &cmdResp)
{
	info("Received hello from another Node");
	auto payload = cmdReq.payload().c_str();
	char response[256];
	sprintf(response, "%s#%i", host, publisherPort);
	info("Payload of request: %s", payload);
	info("Response: %s", response);
	auto tmp = split(payload, '#');
	
	char *connectionString = (char*)tmp[1].c_str();
		
	auto subscriber = new Subscriber(connectionString, tmp[0].c_str(), this);
	if(subscriber->Connect()) {
		info("connected successfull to Node %s %s", connectionString, tmp[0].c_str());
		
		for(auto i = subscribers.begin(); i != subscribers.end(); i++){
			auto& pair = *i;
		//for (auto& pair: subscribers) {
			auto sub = pair.second;
			if(strcmp(sub->connectString, connectionString)==0)
			{
				warn("Duplicate connection to %s, kill the previous one", connectionString);
				delete sub;
				subscribers.erase(pair.first);
				break;
			}
		}
		
		subscribers.insert(std::pair<std::string,Subscriber*> {connectionString, subscriber});
		nodesGuids[tmp[0]] = tmp[0];
		
		cmdResp.set_cmd(ESB::Command::RESPONSE);
		cmdResp.set_payload(response);
	} else {
		err("Can not connect to Node '%s' '%s'", tmp[1].c_str(), tmp[0].c_str());
		cmdResp.set_cmd(ESB::Command::ERROR);
		char errBuf[512];
		sprintf(errBuf, "ESB Proxy can not connect to your Node, check the firewall, connectionString: `%s`", connectionString);
		cmdResp.set_payload(errBuf);
		delete subscriber;
	}
}

void Proxy::Invoke(ESB::Command &cmdReq)
{
	dbg("Invoke(%s) from node %s", cmdReq.identifier().c_str(), cmdReq.source_proxy_guid().c_str());
	invokeCalls++;
	ESB::Command cmdResp;
	char buf[1024];
	
	auto vec = localInvokeMethods[cmdReq.identifier()];
	if(vec.size() > 0){
		int rand = 0;
		if(vec.size()>0)
			rand = std::rand() % vec.size();
		auto entry = vec.at(rand);
		dbg("%s found in local methods", entry->identifier);
		
		cmdResp.set_cmd(ESB::Command::INVOKE);
		
		cmdResp.set_guid_to(entry->methodGuid);
		cmdResp.set_source_proxy_guid(guid);
		cmdResp.set_identifier(entry->identifier);
		//cmdResp.set_version(cmdReq.version());
		cmdResp.set_payload(cmdReq.payload());
		
		invokeResponses.insert(std::pair<std::string,std::string> {cmdReq.guid_from(), cmdReq.source_proxy_guid()});
		invokeResponsesCallTime.insert(std::pair<std::string,int> {cmdReq.guid_from(), time(NULL)});
		cmdResp.set_guid_from(cmdReq.guid_from());
		
		publisher->Publish(entry->nodeGuid, cmdResp);
		
		return;
	}
	auto remoteEntryVec = remoteInvokeMethods[cmdReq.identifier()];
	if(remoteEntryVec.size()<1) {
		warn("identifier '%s' not found", cmdReq.identifier().c_str());
		invokeErrors++;
		cmdResp.set_cmd(ESB::Command::ERROR);
		sprintf(buf, "Identifier '%s' not found in registry", cmdReq.identifier().c_str());
		cmdResp.set_payload(buf);
	}
	else
	{
		dbg("identifier '%s' found in remote proxy", cmdReq.identifier().c_str());
		int rand = std::rand() % remoteEntryVec.size();
		auto entry = remoteEntryVec.at(rand);
		
		cmdResp.set_cmd(ESB::Command::INVOKE);
		
		cmdResp.set_guid_to(entry->methodGuid);
		cmdResp.set_guid_from(cmdReq.guid_from());
		cmdResp.set_source_proxy_guid(guid);
		cmdResp.set_identifier(entry->identifier);
		//cmdResp.set_version(cmdReq.version());
		cmdResp.set_payload(cmdReq.payload());
		
		invokeResponses.insert(std::pair<std::string,std::string> {cmdReq.guid_from(), cmdReq.source_proxy_guid()});
		invokeResponsesCallTime.insert(std::pair<std::string,int> {cmdReq.guid_from(), time(NULL)});
		
		publisher->Publish(entry->proxyGuid, cmdResp);
		return;
	}
	
	cmdResp.set_guid_to(cmdReq.guid_from());
	cmdResp.set_source_proxy_guid(guid);
	cmdResp.set_guid_from(guid);
	
	publisher->Publish(cmdReq.source_proxy_guid().c_str(), cmdResp);
}

void Proxy::CleanUpResponsesMap()
{
	int now = time(NULL);
	int found = 0;
	for (auto it = invokeResponsesCallTime.begin(); it != invokeResponsesCallTime.end();) {
		auto t = it->second;
		if (now-t > 3) {
			found++;
			invokeResponses.erase(it->first);
			it = invokeResponsesCallTime.erase(it);
			if(it == invokeResponsesCallTime.end() || found >= 10000) break;
		} else it++;
	}
	if(found > 0) {
		info("found %i dead callbacks, clean up them, total size of container: %lu", found, invokeResponsesCallTime.size());
	}
}

void Proxy::RegisterInvoke(ESB::Command &cmdReq)
{
	dbg(
		 "RegisterInvoke(%s) from node %s method guid %s",
		 cmdReq.identifier().c_str(),
		 cmdReq.source_proxy_guid().c_str(),
		 cmdReq.payload().c_str()
	);
	
	auto vec = localInvokeMethods[cmdReq.identifier().c_str()];
	bool found = false;
	if(vec.size()>0)
	{
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			auto tEntry = *it;
			if(strcmp(tEntry->methodGuid, cmdReq.payload().c_str())==0)
			{
				found = true;
				tEntry->lastCheckTime = time(NULL);
				break;
			}
		}
	}
	
	if(!found)
	{
		info(
			 "RegisterInvoke(%s) from node %s method guid %s",
			 cmdReq.identifier().c_str(),
			 cmdReq.source_proxy_guid().c_str(),
			 cmdReq.payload().c_str()
		);
		registerInvoke++;
		
		LocalInvokeMethod *entry = (LocalInvokeMethod*)calloc(sizeof(LocalInvokeMethod), sizeof(LocalInvokeMethod));
		strcpy(entry->nodeGuid, cmdReq.source_proxy_guid().c_str());
		strcpy(entry->methodGuid, cmdReq.payload().c_str());
		
		entry->identifier = (char*)malloc((cmdReq.payload().length()*2));
		strcpy(entry->identifier, cmdReq.identifier().c_str());
		
		entry->lastCheckTime = time(NULL);

		auto vec = localInvokeMethods[entry->identifier];
		vec.push_back(entry);
		localInvokeMethods[entry->identifier] = vec;
	}
	
	ESB::Command cmdResp;
	
	cmdResp.set_cmd(ESB::Command::REGISTER_INVOKE_OK);
	
	cmdResp.set_guid_to(cmdReq.source_proxy_guid());
	cmdResp.set_source_proxy_guid(guid);
	
	cmdResp.set_payload(cmdReq.identifier().c_str());
	
	publisher->Publish(cmdReq.source_proxy_guid().c_str(), cmdResp);
}


ESB::Command Proxy::ResponderCallback(ESB::Command &cmdReq)
{
	ESB::Command cmdResp;
	
	dbg("callback happen on guid %s", guid);
	switch (cmdReq.cmd()) {
		case ESB::Command::NODE_HELLO:
			dbg("get request for NODE_HELLO");
			NodeHello(cmdReq, cmdResp);
			break;
		case ESB::Command::PROXY_HELLO:
			dbg("get request for PROXY_HELLO");
			ProxyHello(cmdReq, cmdResp);
			break;
		case ESB::Command::ERROR:
			err("error: %s", cmdReq.payload().c_str());
			break;
		default:
			err("Error, received unknown cmd: %i, payload: %s", cmdReq.cmd(), cmdReq.payload().c_str());
			cmdResp.set_cmd(ESB::Command::ERROR);
			cmdResp.set_payload("Responder unknown command");
			break;
	}
	
	cmdResp.set_guid_from(guid);
	cmdResp.set_guid_to(cmdReq.guid_from());
	cmdResp.set_source_proxy_guid(guid);
		
	return cmdResp;
}

void Proxy::InvokeResponse(ESB::Command &cmdReq, const char *sourceNodeGuid)
{
	ESB::Command cmdResp;
	auto targetNode = invokeResponses.find(cmdReq.guid_to());
	if(targetNode == invokeResponses.end())
	{
		cmdResp.set_cmd(ESB::Command::ERROR);
		char buf[1024];
		sprintf(buf, "Response callback %s not found", cmdReq.guid_to().c_str());
		cmdResp.set_payload(buf);
		cmdResp.set_guid_from(guid);
		cmdResp.set_guid_to(cmdReq.guid_from());
		cmdResp.set_source_proxy_guid(guid);
		publisher->Publish(sourceNodeGuid, cmdResp);
		return;
	}
	
	cmdResp.set_cmd(ESB::Command::RESPONSE);
	cmdResp.set_payload(cmdReq.payload());
	
	cmdResp.set_guid_from(cmdReq.guid_from());
	cmdResp.set_guid_to(cmdReq.guid_to());
	cmdResp.set_source_proxy_guid(guid);
	
	publisher->Publish(targetNode->second.c_str(), cmdResp);
	invokeResponses.erase(cmdReq.guid_to());
	invokeResponsesCallTime.erase(cmdReq.guid_to());
}

void Proxy::RegistryExchangeResponder(ESB::Command &cmdReq)
{
	dbg("send local registry");
	
	ESB::Command cmdResp;
	
	cmdResp.set_cmd(ESB::Command::REGISTRY_EXCHANGE_RESPONSE);
	cmdResp.set_source_proxy_guid(guid);
	cmdResp.set_payload("here is my registry");
	
	int total = 0;
	for (auto it = localInvokeMethods.begin(); it != localInvokeMethods.end(); ++it)
	{
		for (size_t i=0; i < it->second.size(); i++){
			auto entry = cmdResp.add_reg_entry();
			entry->set_type(ESB::Command::INVOKE_METHOD);
			entry->set_identifier(it->first);
			entry->set_method_guid(it->second.at(i)->methodGuid);
			entry->set_proxy_guid(guid);
			total++;
		}
	}
	
	dbg("responce generated, total %i methods", total);
	publisher->Publish(cmdReq.source_proxy_guid().c_str(), cmdResp);
}

void Proxy::PingRequest(ESB::Command &cmdReq)
{
	dbg("ping request");
	ESB::Command cmdResp;
	cmdResp.set_cmd(ESB::Command::RESPONSE);
	cmdResp.set_payload("\"Pong\"");
	
	cmdResp.set_guid_to(cmdReq.guid_from());
	cmdResp.set_source_proxy_guid(guid);
	
	publisher->Publish(cmdReq.source_proxy_guid().c_str(), cmdResp);
}

void Proxy::RemoteRegistryUpdate(ESB::Command &cmdReq)
{
	verb("getted list from %s, %i methods", cmdReq.source_proxy_guid().c_str(), cmdReq.reg_entry_size());
	
	for( int i=0; i < cmdReq.reg_entry_size(); i++)
	{
		auto entry = cmdReq.reg_entry(i);
		if(entry.type() == ESB::Command::INVOKE_METHOD)
		{
			auto identifier = (char*)malloc(entry.identifier().length());
			strcpy(identifier, entry.identifier().c_str());
			auto vec = remoteInvokeMethods[identifier];
			dbg("vec size: %lu, add identifier '%s' method_guid '%s' proxyGuid '%s'", remoteInvokeMethods.at(identifier).size(), identifier, entry.method_guid().c_str(), entry.proxy_guid().c_str());

			bool found = false;
			for (auto it = vec.begin(); it != vec.end(); ++it)
			{
				auto tEntry = *it;
				if(strcmp(tEntry->methodGuid, entry.method_guid().c_str())==0)
				{
					found = true;
					tEntry->lastCheckTime = time(NULL);
					break;
				}
			}
			if(found) {
				free(identifier);
				continue;
			}
			
			auto entryStruct = (RemoteInvokeMethod*)malloc(sizeof(RemoteInvokeMethod));
			entryStruct->identifier = identifier;
			strcpy(entryStruct->methodGuid, entry.method_guid().c_str());
			strcpy(entryStruct->proxyGuid, entry.proxy_guid().c_str());
			entryStruct->lastCheckTime = time(NULL);
			remoteInvokeMethods.at(entry.identifier()).push_back(entryStruct);
			info("push back remote method %s %s", entryStruct->identifier, entryStruct->methodGuid);
		}
	}
}

void Proxy::PublishReq(ESB::Command &cmdReq)
{
	if(cmdReq.recursion() > 0) return;
	
	ESB::Command cmdPub;
	cmdPub.set_cmd(ESB::Command::PUBLISH);
	cmdPub.set_identifier(cmdReq.identifier());
	cmdPub.set_source_proxy_guid(guid);
	cmdPub.set_payload(cmdReq.payload());
	cmdPub.set_recursion(1);
	
	publisher->Publish(cmdReq.identifier().c_str(), cmdReq.identifier().length(), cmdPub);
}

void Proxy::SubscribeReq(ESB::Command &cmdReq)
{
	verb("subscribe request for channel '%s'", cmdReq.identifier().c_str());
	subscribeChannels[cmdReq.identifier()] = cmdReq.identifier();
	
	for ( auto local_it = subscribers.begin(); local_it!= subscribers.end(); ++local_it )
	{
		auto sub = local_it->second;
		sub->Subscribe(cmdReq.identifier());
	}
	
}

void Proxy::SubscriberCallback(ESB::Command &cmdReq, const char *nodeGuid)
{
	dbg("subscriber callback from node: %s", nodeGuid);
	ESB::Command cmdResp;
	char errBuf[512];
	switch (cmdReq.cmd()) {
		case ESB::Command::RESPONSE:
			dbg("get response for %s", cmdReq.guid_to().c_str());
			InvokeResponse(cmdReq, nodeGuid);
			return;
		case ESB::Command::PONG:
			dbg("get pong from %s", nodeGuid);
			return;
		case ESB::Command::REGISTER_INVOKE:
			dbg("get RegisterInvoke from %s", nodeGuid);
			//sometime crash here...
			RegisterInvoke(cmdReq);
			return;
		case ESB::Command::INVOKE:
			dbg("get invoke from node %s method %s to identifier: %s", nodeGuid, cmdReq.guid_from().c_str(), cmdReq.identifier().c_str());
			Invoke(cmdReq);
			return;
		case ESB::Command::REGISTRY_EXCHANGE_REQUEST:
			RegistryExchangeResponder(cmdReq);
			return;
		case ESB::Command::ERROR:
			err("Subscriber %s got error '%s' from %s", guid, cmdReq.payload().c_str(), nodeGuid);
			return;
		case ESB::Command::REGISTRY_EXCHANGE_RESPONSE:
			dbg("getted registry from %s", cmdReq.source_proxy_guid().c_str());
			RemoteRegistryUpdate(cmdReq);
			return;
		case ESB::Command::PING:
			dbg("getted ping request from %s", cmdReq.source_proxy_guid().c_str());
			PingRequest(cmdReq);
			return;
		case ESB::Command::PUBLISH:
			dbg("getted publish request from %s", cmdReq.source_proxy_guid().c_str());
			PublishReq(cmdReq);
			return;
		case ESB::Command::SUBSCRIBE:
			SubscribeReq(cmdReq);
			return;
		default:
			err("Error, received unknown cmd: %i, payload: %s", cmdReq.cmd(), cmdReq.payload().c_str());
			cmdResp.set_cmd(ESB::Command::ERROR);
			sprintf(errBuf,"Subscriber get unknown command: %i, payload: %s", cmdReq.cmd(), cmdReq.payload().c_str());
			cmdResp.set_payload(errBuf);
			break;
	}
	
	cmdResp.set_guid_from(guid);
	cmdResp.set_guid_to(cmdReq.guid_from());
	cmdResp.set_source_proxy_guid(guid);
	//cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
	
	publisher->Publish(nodeGuid, cmdResp);
}

Proxy::~Proxy()
{
	dbg("Destructor");
	isWork = false;
}

void Proxy::RequestRegistryExchange()
{
	if(time(NULL)-lastRegistryExchange < 3) return;
	dbg("here");
	for ( auto local_it = proxiesGuids.begin(); local_it!= proxiesGuids.end(); ++local_it )
	{
		auto s = local_it->second;
		dbg("request registry from %s", s);
		ESB::Command cmdReq;
		cmdReq.set_cmd(ESB::Command::REGISTRY_EXCHANGE_REQUEST);
		cmdReq.set_source_proxy_guid(guid);
		//cmdReq.set_target_proxy_guid(s);
		cmdReq.set_payload("REGISTRY_EXCHANGE_REQUEST please :)");
		publisher->Publish(s.c_str(), cmdReq);
	}
	lastRegistryExchange = time(NULL);
}

bool Proxy::RemoteRegistryHealthCheck()
{
	int now = time(NULL);
	for(auto i = remoteInvokeMethods.begin(); i != remoteInvokeMethods.end(); i++){
		auto& pair = *i;
	//for (auto& pair: remoteInvokeMethods) {
		auto vec = pair.second;
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			auto tEntry = *it;
			if( now > tEntry->lastCheckTime + 5)
			{
				warn("delete remote method `%s` with guid %s", tEntry->identifier, tEntry->methodGuid);
				free(tEntry->identifier);
				free(tEntry);
				it = vec.erase(it);
				remoteInvokeMethods[pair.first] = vec;
				return true;
			}
		}
	}
	return false;
}

bool Proxy::LocalRegistryHealthCheck()
{
	int now = time(NULL);
	for(auto i = localInvokeMethods.begin(); i != localInvokeMethods.end(); i++){
		auto& pair = *i;
	//for (auto& pair: localInvokeMethods) {
		auto vec = pair.second;
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			auto tEntry = *it;
			if( now > tEntry->lastCheckTime + 5)
			{
				warn("delete local method `%s` with guid %s", tEntry->identifier, tEntry->methodGuid);
				free(tEntry->identifier);
				free(tEntry);
				it = vec.erase(it);
				localInvokeMethods[pair.first] = vec;
				return true;
			}
		}
	}
	return false;
}

void *Proxy::MainLoop(void *p)
{
	info("MainLoop started");
#ifdef __linux__
	
#endif
#ifdef __APPLE__
	pthread_setname_np("MainLoop of ESB");
#endif

	auto self = (Proxy*)p;
	int loop = 0;
	while (self->isWork) {
		bool needToSleep = true;
		
		self->responder->Poll();
		auto now = time(NULL);
		for ( auto local_it = self->subscribers.begin(); local_it != self->subscribers.end();)
		{
			auto s = local_it->second;
			for(auto r=0;r<1000;r++)
			{
				auto msg = s->Poll();
				if(msg==NULL)
				{
					if(now - s->lastActiveTime > MAX_INACTIVE_SUBSCRIBER)
					{
						warn("subscriber %s was active last time more then %i sec ago, kill it", s->targetGuid, MAX_INACTIVE_SUBSCRIBER);
						delete local_it->second;
						local_it = self->subscribers.erase(local_it);
						if(local_it == self->subscribers.end()) break;
					}
					break;
				}
				needToSleep = false;
				
				auto nodeGuid = local_it->first;
				self->SubscriberCallback(*msg->cmdReq, nodeGuid.c_str());
				
				zmq_msg_close(&msg->msg);
				delete msg->cmdReq;
				free(msg);
			}
			if(local_it == self->subscribers.end()) break;
			local_it++;
		}
		
		if(loop++ % 25 == 0){
			self->PingRedis();
			while(self->RemoteRegistryHealthCheck());
			while(self->LocalRegistryHealthCheck());
			self->RequestRegistryExchange();
		}
		
		if(loop % 1000 == 0) {
			self->CleanUpResponsesMap();
		}
		
		if(needToSleep) usleep(10000);
	}
	
	return 0;
}

void Proxy::ConnectToAnotherProxy(const char *proxyGuid, const char *connectionString)
{
	char buf[512];
	sprintf(buf, "tcp://%s:%i", host, responderPort);
	char redisKey[1024];
	sprintf(redisKey, "%s#%s", proxyGuid, connectionString);
	if(strcmp(buf, connectionString) == 0) {
		err("found me in registry, but with wrong guid, will delete this entry");
		redisCommand(redisCtx, "ZREM ZSET:PROXIES %s", redisKey);
		return;
	}
	info("connecting to '%s' '%s'", proxyGuid, connectionString);
	
	auto requester = new Requester(connectionString, proxyGuid, this);
	if(requester->Connect()) {
		info("connected successfull to %s %s", connectionString, proxyGuid);
		char connectionString[512];
		if(!requester->SendProxyHello(connectionString)){
			err("hello failed");
			redisCommand(redisCtx, "ZREM ZSET:PROXIES %s", redisKey);
		} else {
			info("hello ok: '%s'", connectionString);
			
			auto subscriber = new Subscriber(connectionString, proxyGuid, this);
			if(subscriber->Connect()) {
				info("connected successfull");
				
				for(auto i = subscribers.begin(); i != subscribers.end(); i++){
					auto& pair = *i;
				//for (auto& pair: subscribers) {
					auto sub = pair.second;
					if(strcmp(sub->connectString, connectionString)==0)
					{
						warn("Duplicate connection to %s, kill the previous one", connectionString);
						delete sub;
						subscribers.erase(pair.first);
						break;
					}
				}
				
				subscribers.insert(std::pair<std::string,Subscriber*> {proxyGuid, subscriber});
				proxiesGuids[proxyGuid] = proxyGuid;
			} else {
				err("can not connect to publisher");
				delete subscriber;
				proxiesGuids.erase(proxyGuid);
			}
		}
	} else {
		err("can not connect to responder %s, remove it from redis", redisKey);
		redisCommand(redisCtx, "ZREM ZSET:PROXIES %s", redisKey);
	}
	delete requester;
}

void Proxy::PingRedis()
{
	if(time(NULL)-lastRedisPing<2) return;
	dbg("ping redis");
	auto reply = (redisReply*)redisCommand(
										   redisCtx,
										   "ZADD ZSET:PROXIES %i %s#tcp://%s:%i",
										   time(NULL),
										   guid,
										   host,
										   responderPort
										   );
	freeReplyObject(reply);
	reply = (redisReply*)redisCommand(redisCtx, "ZREVRANGE ZSET:PROXIES 0 -1");
	if(reply->type == REDIS_REPLY_ARRAY) {
		dbg("get %zu proxies from redis", reply->elements);
		for(size_t n=0;n<reply->elements;n++) {
			auto proxy = (redisReply*)reply->element[n];
			auto vector = split(proxy->str, '#');
			auto guid2 = vector[0].c_str();
			auto connectionString = vector[1].c_str();
			dbg("guid2: %s\nconnect string: %s", guid2, connectionString);
			if(strcmp(guid2, guid) == 0) {
				dbg("found me in proxies, skip");
				continue;
			}
			
			//check if connected here...
			auto got = subscribers.find(guid2);
			if(got != subscribers.end()){
				continue;
			}
			ConnectToAnotherProxy(guid2, connectionString);
		}
	} else {
		err("Redis return weird answer... '%i'", reply->type);
	}
	freeReplyObject(reply);
	lastRedisPing = time(NULL);
}

void Proxy::Init(Handle<Object> exports) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("Proxy"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	// Prototype
	// tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
	// FunctionTemplate::New(PlusOne)->GetFunction());
	constructor = Persistent<Function>::New(tpl->GetFunction());
	exports->Set(String::NewSymbol("Proxy"), constructor);
}

Handle<Value> Proxy::New(const Arguments& args) {
	HandleScope scope;

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new Proxy(...)`
		if(args.Length() != 3)
		{
			return v8::ThrowException(v8::String::New("3 arguments required! (esb-proxy-host, redis-host, redis-port)"));
		}
		Proxy* obj = new Proxy(args);
		obj->Wrap(args.This());
		obj->Ref();
		return args.This();
	} else {
		// Invoked as plain function `Proxy(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		return scope.Close(constructor->NewInstance(argc, argv));
	}
}

void GenerateGuid(char *guidStr, const size_t len)
{
	char *pGuidStr = guidStr;
	size_t i;
	
	for(i = 0; i < len; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;

}

void GenerateLongGuid(char *guidStr)
{
	char *pGuidStr = guidStr;
	int i;
	
	srand(static_cast<unsigned int> (time(NULL)));  /*Randomize based on time.*/
	
	/*Data1 - 8 characters.*/
	*pGuidStr++ = '{';
	for(i = 0; i < 8; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data2 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data3 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data4 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data5 - 12 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 12; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	*pGuidStr++ = '}';
	*pGuidStr = '\0';
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

inline double timestamp()
{
	struct timeval tv;   // see gettimeofday(2)
	gettimeofday(&tv, NULL);
	double t = (double) tv.tv_sec + (double) 1e-6 * tv.tv_usec;
	// return seconds.microseconds since epoch
	return (double)(t*1000);
}

unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}
NODE_MODULE(proxy, InitAll)