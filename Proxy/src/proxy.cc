#include "proxy.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
	Proxy::Init(exports);
}

Persistent<Function> Proxy::constructor;

Proxy::Proxy(char *connectionString)
{
	printf("Create zmq context\n");
	zContext = zmq_ctx_new();
	
}

Proxy::~Proxy()
{
	printf("terminator proxy\n");
}

void Proxy::Init(Handle<Object> exports) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewSymbol("Proxy"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	// Prototype
	tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
	FunctionTemplate::New(PlusOne)->GetFunction());
	constructor = Persistent<Function>::New(tpl->GetFunction());
	exports->Set(String::NewSymbol("Proxy"), constructor);
}

Handle<Value> Proxy::New(const Arguments& args) {
	HandleScope scope;

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new Proxy(...)`
		
		printf("Proxy::New with #args %i\n", args.Length());
		
		char *connectionString = NULL;
		if(args[0]->IsUndefined()) {
			v8::String::Utf8Value param1(args[0]->ToString());
		} else {
			connectionString = DEFAULT_REQRES_CONNECTION_STRING;
		} 
		Proxy* obj = new Proxy(connectionString);
		obj->Wrap(args.This());
		return args.This();
	} else {
		// Invoked as plain function `Proxy(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		return scope.Close(constructor->NewInstance(argc, argv));
	}
}

Handle<Value> Proxy::PlusOne(const Arguments& args) {
	HandleScope scope;

	Proxy* obj = ObjectWrap::Unwrap<Proxy>(args.This());
	obj->value_ += 1;

	return scope.Close(Number::New(obj->value_));
}

NODE_MODULE(proxy, InitAll)