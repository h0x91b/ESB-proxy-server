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

#define LOG_LEVEL 3
#define GUID_SIZE 16

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#if LOG_LEVEL >= 4
#  define dbg(format, ...) fprintf(stdout, ANSI_COLOR_MAGENTA "DEBUG: %s:%i %s: " format ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#  define dbg(...)
#endif

#if LOG_LEVEL >= 3
#  define verb(format, ...) fprintf(stdout, ANSI_COLOR_CYAN "VERBOSE: %s:%i %s: " format ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#  define verb(...)
#endif

#if LOG_LEVEL >= 2
#  define info(format, ...) fprintf(stdout, ANSI_COLOR_GREEN "INFO: %s:%i %s: " format ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#  define info(...)
#endif

#if LOG_LEVEL >= 1
#  define warn(format, ...) fprintf(stderr, ANSI_COLOR_YELLOW "WARN: %s:%i %s: " format ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#  define warn(...)
#endif

#define err(format, ...) fprintf(stderr, ANSI_COLOR_RED "ERROR: %s:%i %s: " format ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)


//48 - Address already in use (mac)
//98 - Address already in use (linux)
#ifdef __APPLE__
#define ADDR_IN_USE_ERROR 48
#else
#define ADDR_IN_USE_ERROR 98
#endif

void GenerateGuid(char *guidStr, const size_t len);
unsigned long mix(unsigned long a, unsigned long b, unsigned long c);
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

inline double timestamp();

#endif
