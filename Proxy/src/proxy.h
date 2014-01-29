#ifndef PROXY_H
#define PROXY_H

#include <node.h>
#include <v8.h>
#include <zmq.h>

#define DEFAULT_REQRES_CONNECTION_STRING "tcp://*:5555"

class Proxy : public node::ObjectWrap {
	public:
		static void Init(v8::Handle<v8::Object> exports);

	private:
		explicit Proxy(char *connectionString);
		~Proxy();

		static v8::Handle<v8::Value> New(const v8::Arguments& args);
		static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
		static v8::Persistent<v8::Function> constructor;
		double value_;
		void *zContext;
};

#endif