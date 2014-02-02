//
//  commander.h
//  ESB
//
//  Created by Arseniy Pavlenko on 2/2/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#ifndef __ESB__commander__
#define __ESB__commander__

#include <node.h>
#include <v8.h>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>
#include "command.pb.h"

class Commander
{
public:
	Commander(char *connectionString);
	~Commander();
private:
	void *zContext;
	void *zResponder;
	static void* Thread(void*);
	bool isWork;
	pthread_t thread;
};

#endif /* defined(__ESB__commander__) */
