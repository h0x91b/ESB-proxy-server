//
//  responder.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "responder.h"

Responder::Responder(int _port)
{
	port = _port;
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
		char buffer[65536];
		zmq_recv(self->zResponder, buffer, 65536, 0);
		printf ("Responder::Thread received: %s\n", buffer);
		
		ESB::Command cmd;
		cmd.set_identifier("/DEV/test/method/v1");
		cmd.set_version(1);
		cmd.set_cmd(ESB::Command::INFO);
		
		int size = cmd.ByteSize();
		void *bb = calloc(size+1, size+1);
		cmd.SerializeToArray(bb, size);
		printf("Protobuf: `%s` len: %i bytes\n", bb, size);
		
		//zmq_send(self->zResponder, buffer, strlen(buffer), 0);
        zmq_send(self->zResponder, bb, size, 0);
        free(bb);
	}
	printf("Responder::Thread finished\n");
	return 0;
}
