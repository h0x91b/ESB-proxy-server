//
//  globals.h
//  ESB
//
//  Created by Arseniy Pavlenko on 2/4/14.
//  Copyright (c) 2014 Arseniy Pavlenko. All rights reserved.
//

#ifndef ESB_globals_h
#define ESB_globals_h

#include <node.h>
#include <v8.h>
#include <zmq.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sstream>
#include <vector>

#ifdef _DEBUG
#  define dbg(format, ...) fprintf(stderr, "%s:%i %s()\n" format "\n\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#  define dbg(...)
#endif

void GenerateGuid(char *guidStr);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);


#endif
