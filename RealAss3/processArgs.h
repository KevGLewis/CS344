//
//  processArgs.h
//  Assignment-3
//
//  Created by Kevin Lewis on 2/20/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#ifndef processArgs_h
#define processArgs_h

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

// Let's set up the global room name list
#define NUMNAMES 10
#define NUMROOMS 7
#define STR_SIZE 10000
#define BUFFER 2048
#define ARGSIZE 512

int ProcessLine(char **args, int nArgs);
void ProcessExit(void);

#endif /* processArgs_h */
