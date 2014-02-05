//
//  responder.h
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#ifndef __ESB__responder__
#define __ESB__responder__

#include <iostream>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>

class Proxy;

#include "command.pb.h"
#include "globals.h"
#include "proxy.h"

class Responder
{
public:
	Responder(int, char*, Proxy*);
	~Responder();
	int port;
	char *guid;

private:
	void *zContext;
	void *zResponder;
	static void* Thread(void*);
	bool isWork;
	pthread_t thread;
	Proxy *proxy;
};

#endif /* defined(__ESB__responder__) */
