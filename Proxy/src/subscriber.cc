//
//  subscriber.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "subscriber.h"

Subscriber::Subscriber(char *_connectString, const char *_targetGuid, Proxy* _proxy)
{
	connectString = _connectString;
	strcpy(targetGuid, _targetGuid);
	proxy = _proxy;
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_SUB);
}

bool Subscriber::Connect()
{
	dbg("connect to %s", connectString);
	auto rc = zmq_connect(zResponder, connectString);
	zmq_setsockopt(zResponder, ZMQ_SUBSCRIBE, "", 0);
	
	if(rc==0)
	{
		isWork = true;
		pthread_create(&thread, NULL, &Thread, this);
		return TRUE;
	}
	
	int errCode = zmq_errno();
	dbg("zmq failed to connect, errcode: %i, desc: %s",errCode, zmq_strerror(errCode));
	
	return FALSE;
}

Subscriber::~Subscriber()
{
	dbg("the end");
	isWork = FALSE;
}

void *Subscriber::Thread(void* d)
{
	dbg("start");
	auto self = (Subscriber*)d;
	while (self->isWork)
	{
		dbg("wait for PUB");
		zmq_msg_t msg;
		int msgInitRC = zmq_msg_init (&msg);
		assert (msgInitRC == 0);
		size_t len = zmq_recvmsg(self->zResponder, &msg, 0);
		assert (len != (size_t)-1);
		//char *buffer = (char*)zmq_msg_data(&msg);
		dbg("copy buffer");
		unsigned char *buffer = (unsigned char *)malloc(len);
		memcpy(buffer, zmq_msg_data(&msg), len);
		dbg ("received: %zu bytes", len);
		
		ESB::Command cmdReq;
		cmdReq.ParseFromArray(buffer+38*sizeof(char), len-38*sizeof(char));
		//(*self->callback)(cmdReq);
		self->proxy->SubscriberCallback(cmdReq, self->targetGuid, buffer);

		zmq_msg_close (&msg);
	}
	return 0;
}