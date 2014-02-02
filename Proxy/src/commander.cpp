//
//  commander.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/2/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "commander.h"

Commander::Commander(char *connectionString)
{
	printf("Commander::Commander(%s)\n", connectionString);
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_REP);
	int rc = zmq_bind (zResponder, "tcp://*:5555");
	assert (rc == 0);
	printf("Commander::Commander bind success\n");
	isWork = true;
	pthread_create(&thread, NULL, &Thread, this);
}

Commander::~Commander()
{
	printf("Commander::~Commander()\n");
	isWork = false;
	pthread_cancel(thread);
}

void *Commander::Thread(void* d)
{
	printf("Commander::Thread start\n");
	auto self = (Commander*)d;
	while(self->isWork) {
		printf("Commander::Thread wait for REQ\n");
		char buffer [65536];
		zmq_recv(self->zResponder, buffer, 65536, 0);
		printf ("Commander::Thread received: %s\n", buffer);
		
		ESB::Command cmd;
		cmd.set_identifier("/DEV/test/method/v1");
		cmd.set_version(1);
		cmd.set_cmd(ESB::Command::INVOKE_CALL);
		
		int size = cmd.ByteSize();
		void *bb = calloc(size+1, size+1);
		cmd.SerializeToArray(bb, size);
		printf("Protobuf: `%s` len: %i bytes\n", bb, size);
		
		//zmq_send(self->zResponder, buffer, strlen(buffer), 0);
        zmq_send(self->zResponder, bb, size, 0);
        free(bb);
	}
	printf("Commander::Thread finished\n");
	return 0;
}
