#ifndef PROXY_H
#define PROXY_H

#include <node.h>
#include <v8.h>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>

#include "requester.h"
#include "responder.h"
#include "publisher.h"
#include "subscriber.h"

void GenerateGuid(char *guidStr);

class Proxy : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
	char guid[38];
	Responder *responder;

private:
	Proxy();
	~Proxy();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
};



#endif