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
#define MAX_CPID 100

// In future iterations, I would probably replace this with a dynamic array
// But for the purposes of this assignment, I will just use a large array. Hopefully
// I won't get larger than 100 children. If I get to that amount, I will exit.
// This should also help prevent my program from going wild.
struct ChildPIDs
{
    int nChild;
    pid_t cPID[MAX_CPID];
};

struct Arguments
{
    int nArgs;
    int useArgs;
    char *args[ARGSIZE];
    char *inRedirect;
    char *outRedirect;
    int inBackground;
};

void AdditionalProcess(struct Arguments *argsIn);
int ProcessLine(struct Arguments *argsIn, struct ChildPIDs* structIn);
int ProcessExit(struct Arguments *argsIn, struct ChildPIDs* structIn);
void ProcessCD(struct Arguments *argsIn);
void ProcessStatus(struct Arguments *argsIn);
pid_t ProcessOther(struct Arguments *argsIn, struct ChildPIDs* structIn);
void InitializeStruct(struct ChildPIDs* structIn);
void AddChildPID(struct ChildPIDs *structIn, pid_t cPIDIn);
int IsInBackground(struct Arguments *argsIn);

#endif /* processArgs_h */
