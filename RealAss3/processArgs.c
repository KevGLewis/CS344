//
//  processArgs.c
//  Assignment-3
//
//  Created by Kevin Lewis on 2/20/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

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

#include "processArgs.h"

void AdditionalProcess(struct Arguments *argsIn)
{
    // first, run through the array and figure out if there is any redirection
    int i;
    
    // Parse out the redirection
    for(i = 0; i < argsIn->nArgs; i++)
    {
        if(strcmp(argsIn->args[i], "<") == 0 ||
           strcmp(argsIn->args[i], ">") == 0 )
        {
            argsIn->useArgs = i;
        }
        if(strcmp(argsIn->args[i], "<") == 0 && i < argsIn->nArgs)
        {
            argsIn->inRedirect = argsIn->args[i];
        }
        else if(strcmp(argsIn->args[i], ">") == 0 && i < argsIn->nArgs)
        {
            argsIn->outRedirect = argsIn->args[i];
        }
    }
    
    // Determine if it should be run in the background
    if (strcmp(argsIn->args[argsIn->nArgs], "&") == 0)
    {
        argsIn->inBackground = 1;
    }
    
    // Delete the rest of the arguments
    for (i = argsIn->useArgs; i < argsIn->nArgs; i++)
    {
        if(strcmp(argsIn->args[i], "<") == 0 ||
           strcmp(argsIn->args[i], ">") == 0 ||
           strcmp(argsIn->args[i], "&") == 0)
        {
            free(argsIn->args[i]);
        }
        
        argsIn->args[i] = NULL;
    }
    
    // the use args variable store the index of the last used argument
    // so we'll need to increment it by one to get the total number of elements
    argsIn->nArgs = argsIn->useArgs + 1;
}

int ProcessLine(struct Arguments *argsIn, struct ChildPIDs* structIn)
{
    // first Process the arguments further, parsing out the indirection and
    // background command
    AdditionalProcess(argsIn);
    
    if(strcmp(argsIn->args[0], "exit") == 0)
    {
        return ProcessExit(argsIn, structIn);
    }
    
    else if(strcmp(argsIn->args[0], "cd") == 0)
    {
        ProcessCD(argsIn);
    }
    
    else
    {
        ProcessOther(argsIn, structIn);
    
    }
    
    return 1;
}

// Process the exit command
int ProcessExit(struct Arguments *argsIn, struct ChildPIDs* structIn)
{
    if(argsIn->nArgs > 1) // Verify whether or not we have enough characters
    {
        printf("\nError: Too Many Arguments");
        return 1;
    }
    else
    {
        return 0;
    }
}

// Process the CD command
void ProcessCD(struct Arguments *argsIn)
{
    if(argsIn->nArgs == 1) // Verify whether or not we have enough characters
    {
        if(chdir(getenv("HOME")) != 0)
        {
            perror("\nError: ");
        }
    }
    else if(argsIn->nArgs > 2)
    {
        printf("\nError: Too Many Arguments\n");
    }
    else
    {
        if(chdir(argsIn->args[1]) != 0) // if we do have the proper arguments, then try to change directory
        {
            printf("\nError: ");
        }
    }
}

// This will process all of the other commands, including the execvp commands
pid_t ProcessOther(struct Arguments *argsIn, struct ChildPIDs* structIn)
{
    pid_t spawnPid = -5;
    int childExitMethod = -5;
    
    spawnPid = fork();
    // add the new child ID to our array
    if(argsIn->inBackground)
    {
        AddChildPID(structIn, spawnPid);
    }

    switch (spawnPid)
    {
        case -1:
            perror("Hull Breach!");
            exit(1);
            break;
        case 0:
            execvp(argsIn->args[0], argsIn->args);
            break;
        default:
            break;
    }
    if(!argsIn->inBackground)
    {
        waitpid(spawnPid, &childExitMethod, 0);
    }

    return spawnPid;
}

void InitializeStruct(struct ChildPIDs *structIn)
{
    structIn->nChild = 0;
}

// Add the child PID to our child PID struct
void AddChildPID(struct ChildPIDs *structIn, pid_t cPIDIn)
{
    if(structIn->nChild >= MAX_CPID)
    {
        perror("Too Many Children");
        exit(1);
    }
    else
    {
        structIn->cPID[structIn->nChild] = cPIDIn;
        structIn->nChild++;
    }
}

// Checks if a function should be run in the background
int IsInBackground(struct Arguments *argsIn)
{
    if(strcmp(argsIn->args[argsIn->nArgs], "&"))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

