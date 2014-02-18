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

struct SUBSCRIBER_POLL_MSG
{
	ESB::Command *cmdReq;
	zmq_msg_t msg;
};

class Subscriber
{
public:
	Subscriber(const char *, const char*, Proxy*);
	void Subscribe(std::string channel);
	bool Connect();
	~Subscriber();
	SUBSCRIBER_POLL_MSG *Poll();
	
	char connectString[512];
private:
	void *zContext;
	void *zResponder;
	
	char targetGuid[GUID_SIZE+1];
	Proxy *proxy;
};

#endif /* defined(__ESB__subscriber__) */
