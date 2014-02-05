//
//  subscriber.h
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#ifndef __ESB__subscriber__
#define __ESB__subscriber__

#include <iostream>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>

#include "command.pb.h"
#include "globals.h"
#include "proxy.h"

class Proxy;

class Subscriber
{
public:
	Subscriber(char *, char*, Proxy*);
	bool Connect();
	~Subscriber();
private:
	void *zContext;
	void *zResponder;
	char *connectString;
	static void *Thread(void* d);
	
	bool isWork;
	char *targetGuid;
	pthread_t thread;
	Proxy *proxy;
};

#endif /* defined(__ESB__subscriber__) */
