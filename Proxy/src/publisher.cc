//
//  publisher.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "publisher.h"

Publisher::Publisher(int _port)
{
	dbg("init");
	port = _port;
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_PUB);
	
	char bindstr[256] = {0};
	sprintf(bindstr, "tcp://*:%i", port);
	dbg("try to bind: %s", bindstr);
	int rc = zmq_bind (zResponder, bindstr);
	assert (rc == 0);
	dbg("bind success");

}

Publisher::~Publisher()
{
	dbg("the end");
}

void Publisher::Publish(const char* targetGuid, ESB::Command &cmd)
{
	size_t guidSize = 38;
	size_t size = cmd.ByteSize();
	char *bb = (char*)malloc(size+sizeof(char)*guidSize);
	memcpy(bb, targetGuid, 38);
	bb+=guidSize;
	
	cmd.SerializeToArray(bb, size);
	bb-=guidSize;
	dbg("Publish len: %zu bytes", size+sizeof(char)*guidSize);

	zmq_send(zResponder, bb, size+sizeof(char)*guidSize, ZMQ_DONTWAIT);
	free(bb);
}