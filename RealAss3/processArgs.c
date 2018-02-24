//
//  processArgs.c
//  Assignment-3
//
//  Created by Kevin Lewis on 2/20/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//



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
            if (argsIn->useArgs == -1)
            {
                argsIn->useArgs = i;
            }
        }
        if(strcmp(argsIn->args[i], "<") == 0 && i < argsIn->nArgs - 1)
        {
            argsIn->inRedirect = argsIn->args[i + 1];
        }
        else if(strcmp(argsIn->args[i], ">") == 0 && i < argsIn->nArgs - 1)
        {
            argsIn->outRedirect = argsIn->args[i + 1];
        }
    }
    
    // Determine if it should be run in the background
    if (strcmp(argsIn->args[argsIn->nArgs - 1], "&") == 0)
    {
        argsIn->inBackground = 1;
    }
    
    // if there was no
    
    // Delete the rest of the arguments only if I/O redirection occurred
    if(argsIn->useArgs != -1)
    {
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
    
    // Also just need to check the situation where the process is run in the background
    // with no input redirection
    else if(strcmp(argsIn->args[argsIn->nArgs - 1], "&") == 0)
    {
        free(argsIn->args[argsIn->nArgs - 1]);
        argsIn->args[argsIn->nArgs - 1] = NULL;
        argsIn->nArgs--;
    }

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
    
    // Now change the child signal handlers
    SetUpChildSigHandle(argsIn->inBackground);

    // This switch statement handles our forking
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
    // if it is in the background, then we won't need to wait until
    // before we next get input from the user
    if(!argsIn->inBackground)
    {
        waitpid(spawnPid, &childExitMethod, 0);
    }

    return spawnPid;
}

// initializes the ChildPID struct
void InitializeStruct(struct ChildPIDs *structIn)
{
    structIn->nChild = 0;
    structIn->inBackground = 0;
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

// Processes the exit method of a particular child
void ProcessExitMethod(pid_t pidIn, int childExitMethod)
{
    // Fill this in with code to process the exit code.
    printf("\nbackground pid %d is done: ", pidIn);
    
    if (WIFEXITED(childExitMethod) != 0)
    {
        int exitStatus = WEXITSTATUS(childExitMethod);
        printf("exit status of %d\n", exitStatus);
        
    }
    else if (WIFSIGNALED(childExitMethod) != 0)
    {
        int termSignal = WTERMSIG(childExitMethod);
        printf("terminated by signal %d\n", termSignal);
    }
    else
    {
        printf("Hull Breach");
    }
    
    
}

// Will run through our child processes and check if any are waiting
void CheckProcesses(struct ChildPIDs *structIn)
{
    int i;
    int childExitMethod;
    pid_t childPID_actual;
    
    for(i = 0; i < structIn->nChild; i++)
    {
        childExitMethod = -5;
        childPID_actual = waitpid(structIn->cPID[i], &childExitMethod, WNOHANG);
        if(childPID_actual == -1)
        {
            printf("\nHull Breach, Panic. Child did not exit correctly\n");
            exit(1);
        }
        else if(childPID_actual != 0)
        {
            ProcessExitMethod(structIn->cPID[i], childExitMethod);
        }
    }
}

// Need to have a different signal handler for the child vs the parent
// we want our shell to continue after running after a child is exited
void SetUpChildSigHandle(int isInBackground)
{
    // Our children should ignore the SIGTSTP Signal
    struct sigaction ignore_action = {{0}};
    ignore_action.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &ignore_action, NULL);
    
    // if in the foreground, we need the process to terminate normally
    
    if(!isInBackground)
    {
        struct sigaction SIGINT_action = {{0}};
        SIGINT_action.sa_handler = catchSIGINT_Child;
        sigfillset(&SIGINT_action.sa_mask);
        SIGINT_action.sa_flags = 0;
        sigaction(SIGINT, &SIGINT_action, NULL);
    }

}

void catchSIGINT_Child(int signo)
{
    char* message = "Caught SIGINT, sleeping for 5 seconds\n";
    write(STDOUT_FILENO, message, 38);
}

