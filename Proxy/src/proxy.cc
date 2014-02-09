#include "globals.h"
#include "proxy.h"
#include <ctime>

using namespace v8;

void InitAll(Handle<Object> exports) {
	Proxy::Init(exports);
}

Persistent<Function> Proxy::constructor;

Proxy::Proxy()
{
	GenerateGuid(guid);
	dbg("guid: %s", guid);
	responderPort = 7770;
	publisherPort = 7771;
	strcpy(host, "localhost");
	responder = new Responder(responderPort, guid, this);
	publisher = new Publisher(publisherPort);
	
	redisCtx = redisConnect("127.0.0.1", 6379);
	if (redisCtx != NULL && redisCtx->err) {
		dbg("Redis connection error: %s", redisCtx->errstr);
		redisCtx = NULL;
	}
	isWork = TRUE;
	pthread_create(&thread, NULL, &MainLoop, this);
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
		dbg("connected successfull");
		subscribers.insert(std::pair<std::string,Subscriber*> {tmp[0], subscriber});
		nodesGuids.push_back(tmp[0]);
		
		cmdResp.set_cmd(ESB::Command::RESPONSE);
		cmdResp.set_payload(response);
	} else {
		dbg("can not connect");
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
		cmdResp.set_version(cmdReq.version());
		cmdResp.set_payload(cmdReq.payload());
		
		invokeResponses.insert(std::pair<std::string,std::string*> {cmdReq.guid_from(), new std::string(cmdReq.source_proxy_guid())});
		cmdResp.set_guid_from(cmdReq.guid_from());
		
		publisher->Publish(entry->nodeGuid, cmdResp);
		
		return;
	}
	auto remoteEntry = remoteInvokeMethods[cmdReq.identifier()];
	if(!remoteEntry) {
		dbg("identifier not found");
		invokeErrors++;
		cmdResp.set_cmd(ESB::Command::ERROR);
		sprintf(buf, "Identifier '%s' not found in registry", cmdReq.identifier().c_str());
		cmdResp.set_payload(buf);
	}
	
	cmdResp.set_guid_to(cmdReq.guid_from());
	cmdResp.set_source_proxy_guid(guid);
	cmdResp.set_guid_from(guid);
	
	publisher->Publish(cmdReq.source_proxy_guid().c_str(), cmdResp);
}

void Proxy::RegisterInvoke(ESB::Command &cmdReq)
{
	dbg(
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

//	auto vec = localInvokeMethods[entry->identifier];
//	if(vec==NULL){
//		vec = new std::vector<LocalInvokeMethod*>;
//		localInvokeMethods.insert(std::pair<std::string,std::vector<LocalInvokeMethod*>*> {entry->identifier, vec});
//	}
//	vec->push_back(entry);
//	localInvokeMethods.at(entry->identifier).push_back(entry);
	localInvokeMethods.emplace(entry->identifier, std::vector<LocalInvokeMethod*>());
	localInvokeMethods.at(entry->identifier).push_back(entry);
	
	
	ESB::Command cmdResp;
	
	cmdResp.set_cmd(ESB::Command::REGISTER_INVOKE_OK);
	
	cmdResp.set_guid_to(cmdReq.source_proxy_guid());
	cmdResp.set_source_proxy_guid(guid);
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
		default:
			dbg("Error, received unknown cmd: %i", cmdReq.cmd());
			cmdResp.set_cmd(ESB::Command::ERROR);
			cmdResp.set_payload("Unknown command");
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
	auto targetNode = invokeResponses[cmdReq.guid_to()];
	if(targetNode == NULL)
	{
		cmdResp.set_cmd(ESB::Command::ERROR);
		cmdResp.set_payload("Response callback not found");
		cmdResp.set_guid_from(guid);
		cmdResp.set_guid_to(cmdReq.guid_from());
		publisher->Publish(sourceNodeGuid, cmdResp);
		return;
	}
	
	cmdResp.set_cmd(ESB::Command::RESPONSE);
	cmdResp.set_payload(cmdReq.payload());
	
	cmdResp.set_guid_from(cmdReq.guid_from());
	cmdResp.set_guid_to(cmdReq.guid_to());
	
	publisher->Publish(targetNode->c_str(), cmdResp);
	delete targetNode;
	invokeResponses.erase(cmdReq.guid_to());

}

void Proxy::SubscriberCallback(ESB::Command &cmdReq, const char *nodeGuid)
{
	dbg("subscriber callback from node: %s", nodeGuid);
	ESB::Command cmdResp;
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
		default:
			dbg("Error, received unknown cmd: %i", cmdReq.cmd());
			cmdResp.set_cmd(ESB::Command::ERROR);
			cmdResp.set_payload("Unknown command");
			break;
	}
	
	cmdResp.set_guid_from(guid);
	cmdResp.set_guid_to(cmdReq.guid_from());
	
	publisher->Publish(nodeGuid, cmdResp);
}

Proxy::~Proxy()
{
	dbg("Destructor");
	isWork = FALSE;
}

void *Proxy::MainLoop(void *p)
{
	dbg("MainLoop started");
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
		
		if(needToSleep) usleep(5000);
	}
	
	return 0;
}

void *Proxy::Thread(void* d)
{
	dbg("start");
	auto self = (Proxy*)d;
	while (self->isWork)
	{
		dbg("ping redis");
		
		for(size_t n=0; n < self->nodesGuids.size(); n++)
		{
			ESB::Command ping;
			char _guid[39] = {0};
			GenerateGuid(_guid);
			ping.set_identifier("/ping");
			ping.set_guid_from(_guid);
			ping.set_guid_to(self->nodesGuids[n].c_str());
			ping.set_source_proxy_guid(self->guid);
			ping.set_version(1);
			ping.set_start_time(timestamp());
			ping.set_timeout_ms(3000);
			ping.set_cmd(ESB::Command::PING);
			ping.set_payload("ping");
			//self->publisher->Publish(self->nodesGuids[n].c_str(), ping);
		}
		
		auto reply = (redisReply*)redisCommand(
											   self->redisCtx,
											   "ZADD ZSET:PROXIES %i %s#tcp://%s:%i",
											   time(NULL),
											   self->guid,
											   self->host,
											   self->responderPort
		);
		freeReplyObject(reply);
		reply = (redisReply*)redisCommand(self->redisCtx, "ZREVRANGEBYSCORE ZSET:PROXIES +inf %i", time(NULL)-5);
		if(reply->type == REDIS_REPLY_ARRAY) {
			dbg("get %zu proxies from redis", reply->elements);
			for(size_t n=0;n<reply->elements;n++) {
				auto proxy = (redisReply*)reply->element[n];
				auto vector = split(proxy->str, '#');
				auto guid = vector[0].c_str();
				auto connectionString = vector[1].c_str();
				dbg("guid: %s\nconnect string: %s", guid, connectionString);
				if(strcmp(guid, self->guid) == 0) {
					dbg("found me in proxies, skip");
					continue;
				}
				
				//check if connected here...
			}
		} else {
			dbg("Redis return weird answer...");
		}
		freeReplyObject(reply);
		sleep(5);
	}
	return 0;
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
		Proxy* obj = new Proxy();
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