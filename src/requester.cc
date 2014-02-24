//
//  requester.cpp
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#include "requester.h"

Requester::Requester(const char *_connectString, const char *_targetGuid, Proxy *_proxy)
{
	dbg("%s, %s", _connectString, _targetGuid);
	strncpy(connectString, _connectString, 512);
	strcpy(targetGuid, _targetGuid);
	proxy = _proxy;
	zContext = zmq_ctx_new();
	zResponder = zmq_socket (zContext, ZMQ_REQ);
	
	const int timeout = 250;
	zmq_setsockopt(zResponder, ZMQ_RCVTIMEO, &timeout, sizeof(int));
	
	const int lingerTimeout = 250;
	zmq_setsockopt(zResponder, ZMQ_LINGER, &lingerTimeout, sizeof(int));
	
	uint64_t highWaterMark = 1;
	zmq_setsockopt(zResponder, ZMQ_SNDHWM, &highWaterMark, sizeof(uint64_t));
}

Requester::~Requester()
{
	dbg("the end");
	zmq_close(zResponder);
	zmq_term(zContext);
}

bool Requester::Connect()
{
	dbg("connect to %s", connectString);
	auto rc = zmq_connect(zResponder, connectString);
	
	if(rc==0)
	{
		return true;
	}
	
	dbg("zmq failed to connect, errcode: %i, desc: %s", zmq_errno(), zmq_strerror(zmq_errno()));
	
	return false;
}

bool Requester::SendProxyHello(char *respConnectionString)
{
	ESB::Command cmdReq;
	cmdReq.set_cmd(ESB::Command::PROXY_HELLO);
	
	//cmdReq.set_target_proxy_guid(targetGuid);
	cmdReq.set_guid_to(targetGuid);
	cmdReq.set_guid_from(proxy->guid);
	cmdReq.set_source_proxy_guid(proxy->guid);
	cmdReq.set_payload(proxy->guid);
	
	size_t size = cmdReq.ByteSize();
	
	zmq_msg_t msgReq;
	int rc = zmq_msg_init_size (&msgReq, size);
	assert(rc == 0);
		
	if(!cmdReq.SerializeToArray(zmq_msg_data(&msgReq), size))
	{
		err("SerializeToArray fail!!!");
	}
	
	dbg("Publish len: %zu bytes", size);
	
	rc = zmq_msg_send (&msgReq, zResponder, 0);
	assert(rc == (int)(size));
	
	zmq_msg_t msgResp;
	int msgInitRC = zmq_msg_init (&msgResp);
	assert (msgInitRC == 0);
	int len = zmq_msg_recv(&msgResp, zResponder, 0);
	if(len<1){
		err("Error %i, %s", zmq_errno(), zmq_strerror(zmq_errno()));
		zmq_msg_close (&msgResp);
		return false;
	}
	
	ESB::Command cmdResp;
	dbg ("received: %i bytes", len);
	auto buffer = (unsigned char *)zmq_msg_data(&msgResp);
	
	cmdResp.ParseFromArray(buffer, len);
	
	strncpy(respConnectionString, cmdResp.payload().c_str(), 512);

	zmq_msg_close (&msgResp);
	
	return true;
}