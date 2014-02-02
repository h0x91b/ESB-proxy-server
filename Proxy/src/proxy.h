#ifndef PROXY_H
#define PROXY_H

#include <node.h>
#include <v8.h>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>
#include "commander.h"

class Proxy : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);

private:
	Proxy(unsigned int commanderPort, unsigned int publisherPort);
	~Proxy();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
	Commander *commander;
};

#endif