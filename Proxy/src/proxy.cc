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
	GenerateGuid(guid);
	info("guid: %s", guid);
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
	isWork = TRUE;
	pthread_create(&thread, NULL, &MainLoop, this);
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
	auto payload = cmdReq.payload().c_str();
	char response[256];
	sprintf(response, "%s#%i", host, publisherPort);
	
	dbg("payload: %s", payload);
	auto tmp = split(payload, '#');
	
	char *connectionString = (char*)tmp[1].c_str();
		
	auto subscriber = new Subscriber(connectionString, tmp[0].c_str(), this);
	if(subscriber->Connect()) {
		info("connected successfull");
		subscribers.insert(std::pair<std::string,Subscriber*> {tmp[0], subscriber});
		nodesGuids.emplace(tmp[0], tmp[0]);
		
		cmdResp.set_cmd(ESB::Command::RESPONSE);
		cmdResp.set_payload(response);
	} else {
		err("can not connect");
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
		info("%s found in local methods", entry->identifier);
		
		cmdResp.set_cmd(ESB::Command::INVOKE);
		
		cmdResp.set_guid_to(entry->methodGuid);
		cmdResp.set_source_proxy_guid(guid);
		cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
		cmdResp.set_identifier(entry->identifier);
		cmdResp.set_version(cmdReq.version());
		cmdResp.set_payload(cmdReq.payload());
		
		invokeResponses.insert(std::pair<std::string,std::string> {cmdReq.guid_from(), std::string(cmdReq.source_proxy_guid())});
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
		info("identifier '%s' found in remote proxy", cmdReq.identifier().c_str());
		int rand = std::rand() % remoteEntryVec.size();
		auto entry = remoteEntryVec.at(rand);
		
		cmdResp.set_cmd(ESB::Command::INVOKE);
		
		cmdResp.set_guid_to(entry->methodGuid);
		cmdResp.set_guid_from(cmdReq.guid_from());
		cmdResp.set_source_proxy_guid(guid);
		cmdResp.set_target_proxy_guid(entry->proxyGuid);
		cmdResp.set_identifier(entry->identifier);
		cmdResp.set_version(cmdReq.version());
		cmdResp.set_payload(cmdReq.payload());
		
		invokeResponses.insert(std::pair<std::string,std::string> {cmdReq.guid_from(), std::string(cmdReq.source_proxy_guid())});
		
		publisher->Publish(entry->proxyGuid, cmdResp);
		return;
	}
	
	cmdResp.set_guid_to(cmdReq.guid_from());
	cmdResp.set_source_proxy_guid(guid);
	cmdResp.set_guid_from(guid);
	
	publisher->Publish(cmdReq.source_proxy_guid().c_str(), cmdResp);
}

void Proxy::RegisterInvoke(ESB::Command &cmdReq)
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
	
	entry->identifier = (char*)malloc((cmdReq.payload().length()+1)*sizeof(char));
	strcpy(entry->identifier, cmdReq.identifier().c_str());

	localInvokeMethods.emplace(entry->identifier, std::vector<LocalInvokeMethod*>());
	localInvokeMethods.at(entry->identifier).push_back(entry);
	
	
	ESB::Command cmdResp;
	
	cmdResp.set_cmd(ESB::Command::REGISTER_INVOKE_OK);
	
	cmdResp.set_guid_to(cmdReq.source_proxy_guid());
	cmdResp.set_source_proxy_guid(guid);
	cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
	cmdResp.set_guid_from(guid);
	
	cmdResp.set_payload(entry->identifier);
	
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
	cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
		
	return cmdResp;
}

void Proxy::InvokeResponse(ESB::Command &cmdReq, const char *sourceNodeGuid)
{
	ESB::Command cmdResp;
	assert(strcmp(cmdReq.target_proxy_guid().c_str(), guid) == 0);
	auto targetNode = invokeResponses.find(cmdReq.guid_to());
	if(targetNode == invokeResponses.end())
	{
		cmdResp.set_cmd(ESB::Command::ERROR);
		cmdResp.set_payload("Response callback not found");
		cmdResp.set_guid_from(guid);
		cmdResp.set_guid_to(cmdReq.guid_from());
		cmdResp.set_source_proxy_guid(guid);
		cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
		publisher->Publish(sourceNodeGuid, cmdResp);
		return;
	}
	
	cmdResp.set_cmd(ESB::Command::RESPONSE);
	cmdResp.set_payload(cmdReq.payload());
	
	cmdResp.set_guid_from(cmdReq.guid_from());
	cmdResp.set_guid_to(cmdReq.guid_to());
	cmdResp.set_source_proxy_guid(guid);
	cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
	
	publisher->Publish(targetNode->second.c_str(), cmdResp);
	invokeResponses.erase(cmdReq.guid_to());

}

void Proxy::RegistryExchangeResponder(ESB::Command &cmdReq)
{
	dbg("send local registry");
	
	ESB::Command cmdResp;
	
	cmdResp.set_cmd(ESB::Command::REGISTRY_EXCHANGE_RESPONSE);
	cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
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

void Proxy::RemoteRegistryUpdate(ESB::Command &cmdReq)
{
	info("getted list from %s, %i methods", cmdReq.source_proxy_guid().c_str(), cmdReq.reg_entry_size());
	
	for( int i=0; i < cmdReq.reg_entry_size(); i++)
	{
		auto entry = cmdReq.reg_entry(i);
		if(entry.type() == ESB::Command::INVOKE_METHOD)
		{
			auto identifier = (char*)malloc(entry.identifier().length());
			strcpy(identifier, entry.identifier().c_str());
			//auto emplace = remoteInvokeMethods.emplace(identifier, std::vector<RemoteInvokeMethod*>());
			auto vec = remoteInvokeMethods[identifier];
			dbg("vec size: %lu, add identifier '%s' method_guid '%s' proxyGuid '%s'", remoteInvokeMethods.at(identifier).size(), identifier, entry.method_guid().c_str(), entry.proxy_guid().c_str());

			bool found = false;
//			auto vec = (*emplace.first).second;
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
			warn("push back %s %s", entryStruct->identifier, entryStruct->methodGuid);
		}
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
	cmdResp.set_target_proxy_guid(cmdReq.source_proxy_guid());
	
	publisher->Publish(nodeGuid, cmdResp);
}

Proxy::~Proxy()
{
	dbg("Destructor");
	isWork = FALSE;
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
		cmdReq.set_target_proxy_guid(s);
		cmdReq.set_payload("REGISTRY_EXCHANGE_REQUEST please :)");
		publisher->Publish(s.c_str(), cmdReq);
	}
	lastRegistryExchange = time(NULL);
}

bool Proxy::RemoteRegistryHealthCheck()
{
	int now = time(NULL);
	for (auto& pair: remoteInvokeMethods) {
		auto vec = pair.second;
		for (auto it = vec.begin(); it != vec.end(); ++it)
		{
			auto tEntry = *it;
			if( now > tEntry->lastCheckTime + 5)
			{
				warn("delete method `%s` with guid %s", tEntry->identifier, tEntry->methodGuid);
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

void *Proxy::MainLoop(void *p)
{
	info("MainLoop started");
	pthread_setname_np("MainLoop of ESB");
	auto self = (Proxy*)p;
	while (self->isWork) {
		bool needToSleep = TRUE;
		
		self->responder->Poll();
		
		for ( auto local_it = self->subscribers.begin(); local_it!= self->subscribers.end(); ++local_it )
		{
			auto s = local_it->second;
			auto msg = s->Poll();
			if(msg==NULL) continue;
			needToSleep = FALSE;
			
			auto nodeGuid = local_it->first;
			self->SubscriberCallback(*msg->cmdReq, nodeGuid.c_str());
			
			zmq_msg_close(&msg->msg);
			delete msg->cmdReq;
			free(msg);
		}
		
		self->PingRedis();
		while(self->RemoteRegistryHealthCheck());
		self->RequestRegistryExchange();
		
		if(needToSleep) usleep(50000);
	}
	
	return 0;
}

void Proxy::ConnectToAnotherProxy(const char *proxyGuid, const char *connectionString)
{
	info("connecting to '%s' '%s'", proxyGuid, connectionString);
	
	auto requester = new Requester(connectionString, proxyGuid, this);
	if(requester->Connect()) {
		info("connected successfull");
		char connectionString[512];
		if(!requester->SendProxyHello(connectionString)){
			err("hello failed");
		} else {
			info("hello ok: '%s'", connectionString);
			
			auto subscriber = new Subscriber(connectionString, proxyGuid, this);
			if(subscriber->Connect()) {
				info("connected successfull");
				subscribers.insert(std::pair<std::string,Subscriber*> {proxyGuid, subscriber});
				proxiesGuids.emplace(proxyGuid, proxyGuid);
			} else {
				err("can not connect to publisher");
				delete subscriber;
				proxiesGuids.erase(proxyGuid);
			}
		}
	} else {
		err("can not connect to responder");
	}
	delete requester;
}

void Proxy::PingRedis()
{
	if(time(NULL)-lastRedisPing<3) return;
	info("ping redis");
	auto reply = (redisReply*)redisCommand(
										   redisCtx,
										   "ZADD ZSET:PROXIES %i %s#tcp://%s:%i",
										   time(NULL),
										   guid,
										   host,
										   responderPort
										   );
	freeReplyObject(reply);
	reply = (redisReply*)redisCommand(redisCtx, "ZREVRANGEBYSCORE ZSET:PROXIES +inf %i", time(NULL)-5);
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
		err("Redis return weird answer...");
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

void GenerateGuid(char *guidStr)
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

NODE_MODULE(proxy, InitAll)