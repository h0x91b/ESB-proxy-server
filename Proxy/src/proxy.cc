#include "proxy.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
	Proxy::Init(exports);
}

Persistent<Function> Proxy::constructor;

Proxy::Proxy(int port)
{
	commander = NULL;
	printf("Create zmq context `tcp://*:%i`\n", port);
	char connectionString[256];
	sprintf(connectionString, "tcp://*:%i", port);
	commander = new Commander(connectionString);
}

Proxy::~Proxy()
{
	printf("terminator proxy\n");
	commander->~Commander();
	commander = NULL;
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
		printf("Proxy::New with #args %i\n", args.Length());
		
		Proxy* obj = new Proxy(args[0]->Uint32Value());
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

Commander::Commander(char *connectionString) 
{
	printf("Commander::Commander(%s)\n", connectionString);
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_REP);
	int rc = zmq_bind (zResponder, "tcp://*:5555");
	assert (rc == 0);
	printf("Commander::Commander bind success\n");
	isWork = true;
	pthread_create(&thread, NULL, &Thread, this);
}

Commander::~Commander() 
{
	printf("Commander::~Commander()\n");
	isWork = false;
	pthread_cancel(thread);
}

void *Commander::Thread(void* d) 
{
	printf("Commander::Thread start\n");
	auto self = (Commander*)d;
	while(self->isWork) {
		printf("Commander::Thread wait for REQ\n");
		char buffer [65536];
		zmq_recv(self->zResponder, buffer, 65536, 0);
		printf ("Commander::Thread received: %s\n", buffer);
		
		ESB::Command cmd;
		cmd.set_identifier("/DEV/test/method/v1");
		cmd.set_version(1);
		cmd.set_cmd(ESB::Command::INVOKE_CALL);
		
		int size = cmd.ByteSize(); 
		void *bb = calloc(size+1, size+1);
		cmd.SerializeToArray(bb, size);
		printf("Protobuf: `%s` len: %i bytes\n", bb, size);
		
		//zmq_send(self->zResponder, buffer, strlen(buffer), 0);
        zmq_send(self->zResponder, bb, size, 0);
        free(bb);
	}
	printf("Commander::Thread finished\n");
	return 0;
}

NODE_MODULE(proxy, InitAll)