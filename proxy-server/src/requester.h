//
//  requester.h
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#ifndef __ESB__requester__
#define __ESB__requester__

#include <iostream>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>

#include "command.pb.h"
#include "globals.h"
#include "proxy.h"

class Proxy;

class Requester
{
public:
	Requester(const char *, const char*, Proxy*);
	~Requester();
	bool Connect();
	bool SendProxyHello(char *respConnectionString);
private:
	void *zContext;
	void *zResponder;
	char connectString[512];
	
	char targetGuid[39];
	Proxy *proxy;
};

#endif /* defined(__ESB__requester__) */
