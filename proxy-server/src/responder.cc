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
	
	int retries = 0;
	while (true) {
		char bindstr[256] = {0};
		sprintf(bindstr, "tcp://*:%i", port);
		dbg("try to bind: %s", bindstr);
		int rc = zmq_bind (zResponder, bindstr);
		if(rc != 0)
		{
			err("Bind on port %i failed, code: %i, reason: %s", port, zmq_errno(), zmq_strerror(zmq_errno()));
		}
		//48 - Address already in use
		if(zmq_errno() == 48 && ++retries < 10)
		{
			warn("Try to bind on next port");
			port++;
			continue;
		}
		assert (rc == 0);
		info("bind success");
		break;
	}
}

Responder::~Responder()
{
	dbg("Destructor");
	zmq_ctx_term(zContext);
}

bool Responder::Poll()
{	
	zmq_msg_t msgReq;
	int msgInitRC = zmq_msg_init (&msgReq);
	assert (msgInitRC == 0);
	int len = zmq_recvmsg(zResponder, &msgReq, ZMQ_DONTWAIT);
	if(len == -1 && zmq_errno() == EAGAIN){
		zmq_msg_close (&msgReq);
		return false;
	} else if(len<1){
		dbg("Error %i, %s", zmq_errno(), zmq_strerror(zmq_errno()));
		zmq_msg_close (&msgReq);
		return false;
	}
	assert (len != -1);
	dbg ("received: %i bytes", len);
	
	ESB::Command cmdReq;
	cmdReq.ParseFromArray(zmq_msg_data(&msgReq), len);
	auto cmdResp = proxy->ResponderCallback(cmdReq);
	
	size_t size = cmdResp.ByteSize();
	zmq_msg_t msgResp;
	int rc = zmq_msg_init_size (&msgResp, size);
	assert(rc == 0);

	cmdResp.SerializeToArray(zmq_msg_data (&msgResp), size);
	info("Send response len: %zu bytes", size);
	
	rc = zmq_msg_send (&msgResp, zResponder, 0);
	assert(rc == (int)(size));
	
	zmq_msg_close (&msgReq);
	return true;
}

