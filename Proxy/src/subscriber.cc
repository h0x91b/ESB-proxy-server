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
	
	if(rc==0)
	{
		isWork = true;
		zmq_setsockopt(zResponder, ZMQ_SUBSCRIBE, "", 0);
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

SUBSCRIBER_POLL_MSG *Subscriber::Poll()
{
	zmq_msg_t msg;
	int msgInitRC = zmq_msg_init (&msg);
	assert (msgInitRC == 0);
	int len = zmq_recvmsg(zResponder, &msg, ZMQ_DONTWAIT);
	if(len == -1 && zmq_errno() == EAGAIN){
		zmq_msg_close (&msg);
		return NULL;
	} else if(len<1){
		dbg("Error %i, %s", zmq_errno(), zmq_strerror(zmq_errno()));
		zmq_msg_close (&msg);
		return NULL;
	}
	auto *cmdReq = new ESB::Command;
	dbg ("received: %i bytes", len);
	assert (len != -1);
	unsigned char *buffer = (unsigned char *)malloc(len+1);
	memcpy(buffer, zmq_msg_data(&msg), len);
	
	const int guidSize = 38*sizeof(char);
	buffer+=guidSize;
	cmdReq->ParseFromArray(buffer, len-guidSize);
	buffer-=guidSize;
	zmq_msg_close (&msg);
	
	auto ret = (SUBSCRIBER_POLL_MSG*)calloc(sizeof(SUBSCRIBER_POLL_MSG),sizeof(SUBSCRIBER_POLL_MSG));
	ret->buffer = buffer;
	ret->cmdReq = cmdReq;
	return ret;
}