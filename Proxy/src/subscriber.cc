//
//  subscriber.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "subscriber.h"

Subscriber::Subscriber(char *_connectString)
{
	connectString = _connectString;
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_SUB);
}

bool Subscriber::Connect()
{
	dbg("connect to %s", connectString);
	auto rc = zmq_connect(zResponder, connectString);
	
	if(rc==0) return TRUE;
	
	int errCode = zmq_errno();
	dbg("zmq failed to connect, errcode: %i, desc: %s",errCode, zmq_strerror(errCode));
	
	return FALSE;
}

Subscriber::~Subscriber()
{
	dbg("the end");
}