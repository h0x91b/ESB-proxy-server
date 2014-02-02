#include "proxy.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
	Proxy::Init(exports);
}

Persistent<Function> Proxy::constructor;

Proxy::Proxy(unsigned int commanderPort, unsigned int publisherPort, char *_guid)
{
	commander = NULL;
	strncpy(guid, _guid, 36);
	guid[36] = '\0';
	printf("guid: %s\n", guid);
	char connectionString[256];
	sprintf(connectionString, "tcp://*:%i", commanderPort);
    printf("Create commander with connection string: %s\n", connectionString);
	commander = new Commander(connectionString);
}

Proxy::~Proxy()
{
	printf("terminator proxy\n");
	if(commander) commander->~Commander();
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
        if(args.Length() != 1)
        {
            ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
            return scope.Close(Undefined());
        }
        if(!args[0]->IsObject())
        {
            ThrowException(Exception::TypeError(String::New("Wrong type of arguments")));
            return scope.Close(Undefined());
        }
        
        Local<Object> confObj = args[0]->ToObject();
        if(!confObj->Has(String::New("commanderPort")))
        {
            ThrowException(Exception::TypeError(String::New("Config doesnt have `commanderPort` variable")));
            return scope.Close(Undefined());
        }
		
		if(!confObj->Has(String::New("guid")))
        {
            ThrowException(Exception::TypeError(String::New("Config doesnt have `guid` variable")));
            return scope.Close(Undefined());
        }
		
		v8::String::Utf8Value str(confObj->Get(String::New("guid"))->ToString());
        
        unsigned int commanderPort = confObj->Get(String::New("commanderPort"))->Uint32Value();
        unsigned int publisherPort = confObj->Get(String::New("publisherPort"))->Uint32Value();
        printf("Proxy::New commanderPort: %i, publisherPort: %i\n", commanderPort, publisherPort);
		
		Proxy* obj = new Proxy(commanderPort, publisherPort, *str);
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

NODE_MODULE(proxy, InitAll)