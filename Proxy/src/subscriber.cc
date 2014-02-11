//
//  subscriber.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "subscriber.h"

Subscriber::Subscriber(const char *_connectString, const char *_targetGuid, Proxy* _proxy)
{
	strncpy(connectString, _connectString, 512);
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
		rc = zmq_setsockopt(zResponder, ZMQ_SUBSCRIBE, proxy->guid, 38);
		assert(rc == 0);
		return TRUE;
	}
	
	dbg("zmq failed to connect, errcode: %i, desc: %s", zmq_errno(), zmq_strerror(zmq_errno()));
	
	return FALSE;
}

Subscriber::~Subscriber()
{
	dbg("the end");
	zmq_ctx_term(zContext);
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
	auto buffer = (unsigned char *)zmq_msg_data(&msg);
	
	const int guidSize = 38*sizeof(char);
	buffer+=guidSize;
	cmdReq->ParseFromArray(buffer, len-guidSize);
	buffer-=guidSize;
	
	if(strcmp(cmdReq->target_proxy_guid().c_str(), proxy->guid) != 0)
	{
		err("subscriber receive wrong target_proxy_guid call");
	}
	
	auto ret = (SUBSCRIBER_POLL_MSG*)malloc(sizeof(SUBSCRIBER_POLL_MSG));
	ret->cmdReq = cmdReq;
	ret->msg = msg;
	return ret;
}