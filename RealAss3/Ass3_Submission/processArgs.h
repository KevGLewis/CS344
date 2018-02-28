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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

// Let's set up the global room name list
#define NUMNAMES 10
#define NUMROOMS 7
#define STR_SIZE 10000
#define BUFFER 2048
#define ARGSIZE 512
#define MAX_CPID 50

/*
 
 Get input from user on a loop, including parsing $$, as part of the prompt
 exit command (don't worry about cleaning up background children yet)
 cd command
 fork() + exec() handling
 Input/output redirection
 SIGINT handling
 Background commands
 Finish exit command to clean up background children
 status command
 SIGTSP handling
 */


// In future iterations, I would probably replace this with a dynamic array
// But for the purposes of this assignment, I will just use a large array. Hopefully
// I won't get larger than 50 children. If I get to that amount, I will exit.
// This should also help prevent my program from going wild.
struct ChildPIDs
{
    int nChild;
    pid_t cPID[MAX_CPID];
    
    // This will keep track of the most recently exited program
    pid_t latestFG;
    int exitMethod;
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
void ProcessStatus(struct ChildPIDs* structIn);
pid_t ProcessOther(struct Arguments *argsIn, struct ChildPIDs* structIn);
void InitializeStruct(struct ChildPIDs* structIn);
void AddChildPID(struct ChildPIDs *structIn, pid_t cPIDIn);
int IsInBackground(struct Arguments *argsIn);
void IORedirection(struct Arguments *argsIn);

// Child Checking
void CheckProcesses(struct ChildPIDs *structIn);
void ProcessExitMethod(pid_t pidIn, int childExitMethod, int isInBackground);
void SetUpChildSigHandle(int isInBackground);
void CatchSIGINT_Child(int signo);

// Kills the children (Oh MY!!)
void KillChildren(struct ChildPIDs *structIn);
void CleanChildren(struct ChildPIDs *structIn);

// Background Functions
void HandleBackgroundSwitch(void);
void ToggleBackground(void);
void InitializeBackground(void);


#endif /* processArgs_h */
