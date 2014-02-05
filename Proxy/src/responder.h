//
//  responder.h
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#ifndef __ESB__responder__
#define __ESB__responder__

#include <iostream>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>

#include "command.pb.h"
#include "globals.h"

class Responder
{
public:
	Responder(int, char*, std::function<ESB::Command(ESB::Command cmd)>);
	~Responder();
	int port;
	char *guid;

private:
	void *zContext;
	void *zResponder;
	static void* Thread(void*);
	bool isWork;
	pthread_t thread;
	std::function<ESB::Command(ESB::Command cmd)> callback;
};

#endif /* defined(__ESB__responder__) */
