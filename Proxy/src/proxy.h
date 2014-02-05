#ifndef PROXY_H
#define PROXY_H

#include <node.h>
#include <v8.h>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sstream>
#include <unordered_map>

class Subscriber;
class Publisher;
class Responder;

#include "globals.h"
#include "deps/hiredis/hiredis.h"
#include "requester.h"
#include "responder.h"
#include "publisher.h"
#include "subscriber.h"

//typedef void subCbPtr(ESB::Command);


class Proxy : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
	void SubscriberCallback(ESB::Command cmdReq);
	ESB::Command ResponderCallback(ESB::Command cmdReq);
	
	char guid[38];
	Responder *responder;
	Publisher *publisher;
	redisContext *redisCtx;
	int responderPort;
	int publisherPort;
	char host[128];

private:
	Proxy();
	~Proxy();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
	void NodeHello(ESB::Command &cmdReq, ESB::Command &cmdResp);
	
	static void* Thread(void*);
	bool isWork;
	pthread_t thread;
	std::vector<std::string> nodesGuids;
	std::unordered_map<std::string,Subscriber> subscribers;
	std::vector<std::reference_wrapper<void(ESB::Command cmd)>> subscriberLambdas;
};



#endif