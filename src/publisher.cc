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
	uint64_t highWaterMark = 10000;
	zmq_setsockopt(zResponder, ZMQ_SNDHWM, &highWaterMark, sizeof(uint64_t));
	int sndBufSize = 256*1024;
	zmq_setsockopt(zResponder, ZMQ_SNDBUF, &sndBufSize, sizeof(int));
	
	int retries = 0;
	while (true) {
		char bindstr[256] = {0};
		sprintf(bindstr, "tcp://*:%i", port);
		info("try to bind: %s", bindstr);
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
		info("bind success on %s", bindstr);
		break;
	}

}

Publisher::~Publisher()
{
	dbg("the end");
}

void Publisher::Publish(const char* targetGuid, ESB::Command &cmd)
{
	//cmd.set_target_proxy_guid(targetGuid);
	
	size_t guidSize = GUID_SIZE*sizeof(char);
	size_t size = cmd.ByteSize();
	
	zmq_msg_t msg;
	int rc = zmq_msg_init_size (&msg, size+guidSize);
	assert(rc == 0);
	
	char *bb = (char*)zmq_msg_data (&msg);
	memcpy(bb, targetGuid, guidSize);
	bb+=guidSize;
	
	if(!cmd.SerializeToArray(bb, size))
	{
		dbg("SerializeToArray fail!!!");
	}
	bb-=guidSize;
	dbg("Publish len: %zu bytes", size+guidSize);

	rc = zmq_msg_send (&msg, zResponder, ZMQ_DONTWAIT);
	assert(rc == (int)(size+sizeof(char)*guidSize));
}