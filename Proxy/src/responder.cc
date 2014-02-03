//
//  responder.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "responder.h"

Responder::Responder(int _port, char *_guid)
{
	port = _port;
	guid = _guid;
	printf("Responder::Responder(int port=%i)", port);
	
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_REP);
	isWork = true;
	pthread_create(&thread, NULL, &Thread, this);

}

Responder::~Responder()
{
	printf("Responder::~Responder()\n");
	isWork = false;
	pthread_cancel(thread);
}

void *Responder::Thread(void* d)
{
	printf("Responder::Thread start\n");
	auto self = (Responder*)d;
	
	char bindstr[256] = {0};
	sprintf(bindstr, "tcp://*:%i", self->port);
	printf("Responder::Thread try to bind: %s\n", bindstr);
	int rc = zmq_bind (self->zResponder, bindstr);
	assert (rc == 0);
	printf("Responder::Thread bind success\n");
	
	while(self->isWork) {
		printf("Responder::Thread wait for REQ\n");
		zmq_msg_t msg;
		int msgInitRC = zmq_msg_init (&msg);
		assert (msgInitRC == 0);
		int len = zmq_recvmsg(self->zResponder, &msg, 0);
		assert (len != -1);
		char *buffer = (char*)zmq_msg_data(&msg);
		
		printf ("Responder::Thread received: %i bytes\n", len);
		
		ESB::Command cmdReq, cmdResp;
		cmdReq.ParseFromArray(buffer, len);
		switch (cmdReq.cmd()) {
			case ESB::Command::INFO:
				printf("Responder::Thread get request for INFO\n");
				cmdResp.set_cmd(ESB::Command::ERROR);
				cmdResp.set_payload("Not implemented");
				break;
			default:
				printf("Responder::Thread Error, received unknown cmd: %i\n", cmdReq.cmd());
				cmdResp.set_cmd(ESB::Command::ERROR);
				cmdResp.set_payload("Unknown command");
				break;
		}
		
		cmdResp.set_guid_from(self->guid);
		cmdResp.set_guid_to(cmdReq.guid_from());
		
		int size = cmdResp.ByteSize();
		void *bb = malloc(size);
		
		cmdResp.SerializeToArray(bb, size);
		printf("Send response len: %i bytes\n", size);
		
		//zmq_send(self->zResponder, buffer, strlen(buffer), 0);
        zmq_send(self->zResponder, bb, size, 0);
        free(bb);
		
		zmq_msg_close (&msg);
	}
	printf("Responder::Thread finished\n");
	return 0;
}
