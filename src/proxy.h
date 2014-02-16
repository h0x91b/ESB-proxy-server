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

struct LocalInvokeMethod
{
	char *identifier;
	char nodeGuid[39];
	char methodGuid[39];
	int lastCheckTime;
};

struct RemoteInvokeMethod
{
	char *identifier;
	char proxyGuid[39];
	char methodGuid[39];
	int lastCheckTime;
};

class Proxy : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
	void SubscriberCallback(ESB::Command &cmdReq, const char *guid);
	ESB::Command ResponderCallback(ESB::Command &cmdReq);
	void Invoke(ESB::Command &cmdReq);
	void RegisterInvoke(ESB::Command &cmdReq);
	void InvokeResponse(ESB::Command &cmdReq, const char *sourceNodeGuid);
	void PingRedis();
	void ConnectToAnotherProxy(const char *proxyGuid, const char *connectionString);
	void RequestRegistryExchange();
	void RegistryExchangeResponder(ESB::Command &cmdReq);
	void RemoteRegistryUpdate(ESB::Command &cmdReq);
	bool RemoteRegistryHealthCheck();
	bool LocalRegistryHealthCheck();
	void PingRequest(ESB::Command &cmdReq);
	
	char guid[39];
	Responder *responder;
	Publisher *publisher;
	redisContext *redisCtx;
	int responderPort;
	int publisherPort;
	char host[128];
	char redisHost[256];
	unsigned long redisPort;
	
	int invokeCalls;
	int invokeErrors;
	int registerInvoke;
	int lastRedisPing;
	int lastRegistryExchange;

private:
	Proxy(const v8::Arguments& args);
	~Proxy();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
	void NodeHello(ESB::Command &cmdReq, ESB::Command &cmdResp);
	void ProxyHello(ESB::Command &cmdReq, ESB::Command &cmdResp);
	
	static void *MainLoop(void *p);
	bool isWork;
	pthread_t thread;
	std::map<std::string, std::string> nodesGuids;
	std::map<std::string, std::string> proxiesGuids;
	std::unordered_map<std::string, Subscriber*> subscribers;
	std::unordered_map<std::string, std::vector<LocalInvokeMethod*>> localInvokeMethods; //identifier, struct
	
	std::unordered_map<std::string, std::vector<RemoteInvokeMethod*>> remoteInvokeMethods; //identifier, struct
	
	std::unordered_map<std::string, std::string> invokeResponses; //identifier, struct
};



#endif