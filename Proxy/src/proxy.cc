#include "proxy.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
	Proxy::Init(exports);
}

Persistent<Function> Proxy::constructor;

Proxy::Proxy()
{
	GenerateGuid(guid);
	printf("Proxy::Proxy() guid: %s\n", guid);
	responder = new Responder(7770);
}

Proxy::~Proxy()
{
	printf("Proxy::~Proxy()\n");
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
		Proxy* obj = new Proxy();
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

void GenerateGuid(char *guidStr)
{
	char *pGuidStr = guidStr;
	int i;
	
	srand(static_cast<unsigned int> (time(NULL)));  /*Randomize based on time.*/
	
	/*Data1 - 8 characters.*/
	*pGuidStr++ = '{';
	for(i = 0; i < 8; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data2 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data3 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data4 - 4 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 4; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	/*Data5 - 12 characters.*/
	*pGuidStr++ = '-';
	for(i = 0; i < 12; i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
	
	*pGuidStr++ = '}';
	*pGuidStr = '\0';
}


NODE_MODULE(proxy, InitAll)