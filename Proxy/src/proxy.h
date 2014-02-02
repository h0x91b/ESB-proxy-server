#ifndef PROXY_H
#define PROXY_H

#include <node.h>
#include <v8.h>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>
#include "command.pb.h"

#define DEFAULT_REQRES_CONNECTION_STRING "tcp://*:5555"

class Commander 
{
public:
	Commander(char *connectionString);
	~Commander();
private:
	void *zContext;
	void *zResponder;
	static void* Thread(void*);
	bool isWork;
	pthread_t thread;
};


class Proxy : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);

private:
	explicit Proxy(int port);
	~Proxy();

	static v8::Handle<v8::Value> New(const v8::Arguments& args);
	static v8::Persistent<v8::Function> constructor;
	Commander *commander;
};

#endif