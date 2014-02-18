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
	
	info("new subscriber for target: %s", targetGuid);
}

bool Subscriber::Connect()
{
	dbg("connect to %s", connectString);
	const int timeout = 250;
	zmq_setsockopt(zResponder, ZMQ_RCVTIMEO, &timeout, sizeof(int));
	
	const int lingerTimeout = 250;
	zmq_setsockopt(zResponder, ZMQ_LINGER, &lingerTimeout, sizeof(int));
	
	int rcvBufSize = 512*1024;
	zmq_setsockopt(zResponder, ZMQ_RCVBUF, &rcvBufSize, sizeof(int));
	
	auto rc = zmq_connect(zResponder, connectString);
	
	if(rc==0)
	{
		info("subscribe on channel %s", proxy->guid);
		rc = zmq_setsockopt(zResponder, ZMQ_SUBSCRIBE, proxy->guid, GUID_SIZE);
		assert(rc == 0);
		
		for ( auto local_ch = proxy->subscribeChannels.begin(); local_ch!= proxy->subscribeChannels.end(); ++local_ch )
		{
			auto channel = local_ch->second;
			info("subscribe also on channel %s", channel.c_str());
			rc = zmq_setsockopt(zResponder, ZMQ_SUBSCRIBE, channel.c_str(), channel.length());
			assert(rc == 0);
		}
		
		return true;
	}
	
	dbg("zmq failed to connect, errcode: %i, desc: %s", zmq_errno(), zmq_strerror(zmq_errno()));
	
	return false;
}

void Subscriber::Subscribe(std::string channel)
{
	verb("subscriber of %s subscribe on channel %s", targetGuid, channel.c_str());
	char *chan = new char[channel.length()+1];
	sprintf(chan,"%s\t", channel.c_str());
	zmq_setsockopt(zResponder, ZMQ_SUBSCRIBE, chan, channel.length()+1);
	delete chan;
}

Subscriber::~Subscriber()
{
	info("The end for subscriber for target: %s, %s", targetGuid, connectString);
	zmq_close(zResponder);
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
		err("Error %i, %s", zmq_errno(), zmq_strerror(zmq_errno()));
		zmq_msg_close (&msg);
		return NULL;
	}
	auto *cmdReq = new ESB::Command;
	dbg ("received: %i bytes", len);
	assert (len != -1);
	auto buffer = (unsigned char *)zmq_msg_data(&msg);
	
	int index = 0;
	bool found = false;
	while (!found || index > len)
	{
		if(*buffer == '\t') {
			found = true;
		}
		index++;
		buffer++;
	}
	
	dbg("found \\t at %d", index-1);
	
		
	if(!cmdReq->ParseFromArray(buffer, len - index ))
	{
		info("error parse buf, orig size: %i, index: %i, proto size: %i", len, index, len-index);
		info("byte size: %i", cmdReq->ByteSize());
		buffer = (unsigned char *)zmq_msg_data(&msg);
		err("parse failed!");
		zmq_msg_close (&msg);
		delete cmdReq;
		return NULL;
	}
	cmdReq->set_target_proxy_guid(proxy->guid);
	
	auto ret = (SUBSCRIBER_POLL_MSG*)malloc(sizeof(SUBSCRIBER_POLL_MSG));
	ret->cmdReq = cmdReq;
	ret->msg = msg;
	return ret;
}