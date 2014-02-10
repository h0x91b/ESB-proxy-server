//
//  responder.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "responder.h"

Responder::Responder(int _port, char *_guid, Proxy* _proxy)
{
	port = _port;
	guid = _guid;
	proxy = _proxy;
	dbg("port=%i", port);
	
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_REP);
	
	char bindstr[256] = {0};
	sprintf(bindstr, "tcp://*:%i", port);
	dbg("try to bind: %s", bindstr);
	int rc = zmq_bind (zResponder, bindstr);
	assert (rc == 0);
	dbg("bind success");
}

Responder::~Responder()
{
	dbg("Destructor");
}

bool Responder::Poll()
{	
	zmq_msg_t msg;
	int msgInitRC = zmq_msg_init (&msg);
	assert (msgInitRC == 0);
	int len = zmq_recvmsg(zResponder, &msg, ZMQ_DONTWAIT);
	if(len == -1 && zmq_errno() == EAGAIN){
		zmq_msg_close (&msg);
		return FALSE;
	} else if(len<1){
		dbg("Error %i, %s", zmq_errno(), zmq_strerror(zmq_errno()));
		zmq_msg_close (&msg);
		return FALSE;
	}
	assert (len != -1);
	char *buffer = (char*)zmq_msg_data(&msg);
	info ("received: %i bytes", len);
	
	ESB::Command cmdReq;
	cmdReq.ParseFromArray(buffer, len);
	auto cmdResp = proxy->ResponderCallback(cmdReq);
	
	size_t size = cmdResp.ByteSize();
	void *bb = malloc(size);
	
	cmdResp.SerializeToArray(bb, size);
	info("Send response len: %zu bytes", size);
	
	zmq_send(zResponder, bb, size, 0);
	free(bb);
	zmq_msg_close (&msg);

	return TRUE;
}

