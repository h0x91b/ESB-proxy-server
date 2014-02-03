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

#include "deps/hiredis/hiredis.h"
#include "requester.h"
#include "responder.h"
#include "publisher.h"
#include "subscriber.h"

void GenerateGuid(char *guidStr);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

class Proxy : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
	char guid[38];
	Responder *responder;
	redisContext *redisCtx;
	int responderPort;
	char host[128];

private:
	Proxy();
	~Proxy();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
	
	static void* Thread(void*);
	bool isWork;
	pthread_t thread;
};



#endif