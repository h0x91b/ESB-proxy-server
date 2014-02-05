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
	isWork = true;
	pthread_create(&thread, NULL, &Thread, this);

}

Responder::~Responder()
{
	dbg("Destructor");
	isWork = false;
	pthread_cancel(thread);
}

void *Responder::Thread(void* d)
{
	dbg("start");
	auto self = (Responder*)d;
	
	char bindstr[256] = {0};
	sprintf(bindstr, "tcp://*:%i", self->port);
	dbg("try to bind: %s", bindstr);
	int rc = zmq_bind (self->zResponder, bindstr);
	assert (rc == 0);
	dbg("bind success");
	
	while(self->isWork) {
		dbg("wait for REQ");
		zmq_msg_t msg;
		int msgInitRC = zmq_msg_init (&msg);
		assert (msgInitRC == 0);
		size_t len = zmq_recvmsg(self->zResponder, &msg, 0);
		assert (len != (size_t)-1);
		char *buffer = (char*)zmq_msg_data(&msg);
		dbg ("received: %zu bytes", len);
		
		ESB::Command cmdReq;
		cmdReq.ParseFromArray(buffer, len);
		auto cmdResp = self->proxy->ResponderCallback(cmdReq);
		
		size_t size = cmdResp.ByteSize();
		void *bb = malloc(size);
		
		cmdResp.SerializeToArray(bb, size);
		dbg("Send response len: %zu bytes", size);
		
        zmq_send(self->zResponder, bb, size, 0);
        free(bb);
		zmq_msg_close (&msg);
	}
	dbg("finished");
	return 0;
}
