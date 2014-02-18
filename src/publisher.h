//
//  publisher.h
//  ESB
//
//  Created by Arseniy Pavlenko on 2/3/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#ifndef __ESB__publisher__
#define __ESB__publisher__

#include <iostream>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>

#include "command.pb.h"
#include "globals.h"

class Publisher
{
public:
	Publisher(int port);
	~Publisher();
	void Publish(const char* targetGuid, ESB::Command &cmd);
	void Publish(const char* targetGuid, size_t guidSize, ESB::Command &cmd);
	
	int port;
private:
	void *zContext;
	void *zResponder;
};

#endif /* defined(__ESB__publisher__) */
