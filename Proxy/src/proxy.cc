#include "globals.h"
#include "proxy.h"

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
	strcpy(host, "localhost");
	auto callback = [this] (ESB::Command cmdReq) -> ESB::Command {
		ESB::Command cmdResp;
		
		dbg("callback happen on guid %s", guid);
		char response[256];
		
		switch (cmdReq.cmd()) {
			case ESB::Command::NODE_HELLO:
				dbg("get request for NODE_HELLO");
				sprintf(response, "%s#%i", host, 7771);
								
				cmdResp.set_cmd(ESB::Command::RESPONSE);
				cmdResp.set_payload(response);
				break;
			default:
				dbg("Error, received unknown cmd: %i", cmdReq.cmd());
				cmdResp.set_cmd(ESB::Command::ERROR);
				cmdResp.set_payload("Unknown command");
				break;
		}
		
		cmdResp.set_guid_from(guid);
		cmdResp.set_guid_to(cmdReq.guid_from());
		
		return cmdResp;
	};
	responder = new Responder(responderPort, guid, callback);
	
	redisCtx = redisConnect("127.0.0.1", 6379);
	if (redisCtx != NULL && redisCtx->err) {
		dbg("Redis connection error: %s", redisCtx->errstr);
		redisCtx = NULL;
	}
	isWork = TRUE;
	pthread_create(&thread, NULL, &Thread, this);
}

Proxy::~Proxy()
{
	dbg("Destructor");
	isWork = FALSE;
}

void *Proxy::Thread(void* d)
{
	dbg("start");
	auto self = (Proxy*)d;
	while (self->isWork)
	{
		dbg("ping redis");
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


NODE_MODULE(proxy, InitAll)