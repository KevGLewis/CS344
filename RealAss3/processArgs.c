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
    
    // check if the background has been turned off
    if(!BackgroundOn)
    {
        argsIn->inBackground = 0;
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
    
    else if(strcmp(argsIn->args[0], "status") == 0)
    {
        ProcessStatus(structIn);
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
        KillChildren(structIn);
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

// Process the status command
void ProcessStatus(struct ChildPIDs* structIn)
{
    // verify that we have already processed a function
    if(structIn->latestFG == -5)
    {
        printf("No process to be checked");
    }
    
    else
    {
        ProcessExitMethod(structIn->latestFG, structIn->exitMethod, 0); // 0 means in the foreground
    }
}

// This will process all of the other commands, including the execvp commands
pid_t ProcessOther(struct Arguments *argsIn, struct ChildPIDs* structIn)
{
    pid_t spawnPid = -5;
    int childExitMethod = -5;
    
    spawnPid = fork();

    // This switch statement handles our forking
    switch (spawnPid)
    {
        case -1:
            perror("Hull Breach!");
            exit(1);
            break;
        case 0:
            SetUpChildSigHandle(argsIn->inBackground);
            IORedirection(argsIn);
            if(execvp(argsIn->args[0], argsIn->args) == -1)
            {
                printf("Could not execute command");
                exit(1);
            }
            break;
        default:
            break;
    }

    // add the new child ID to our array
    if(argsIn->inBackground)
    {
        AddChildPID(structIn, spawnPid);
    }
    
    // if it is in the background, then we won't need to wait until
    // before we next get input from the user
    else
    {
        // We need to save the foreground process information
        // in case we call status.
        structIn->latestFG = spawnPid;
        waitpid(spawnPid, &childExitMethod, 0);
        structIn->exitMethod = childExitMethod;
        
    }

    return spawnPid;
}

// Takes care of redirecting the input and output
void IORedirection(struct Arguments *argsIn)
{
    int devNull = open("/dev/null", O_WRONLY);
    int result, sourceFD, targetFD;
    // Take care of input redirection
    // redirect input to dev/null if nothing provided
    if(argsIn->inRedirect == NULL)
    {
        if (devNull == -1) { perror("dev/null error"); exit(1); }
        result = dup2(devNull, 0);
        if (result == -1) { perror("source dup2()"); exit(2); }
    }
    
    // redirect input to the user specified file
    else
    {
        sourceFD = open(argsIn->inRedirect, O_RDONLY);
        if (sourceFD == -1) { perror("source open()"); exit(1); }
        
        result = dup2(sourceFD, 0);
        if (result == -1) { perror("source dup2()"); exit(2); }
    }
    
    // Take care of output redirection
    // redirect input to dev/null if nothing provided
    if(argsIn->outRedirect == NULL)
    {
        if (devNull == -1) { perror("dev/null error"); exit(1); }
        result = dup2(devNull, 1);
        if (result == -1) { perror("target dup2()"); exit(2); }
    }
    
    // redirect output to the user specified file
    else
    {
        targetFD = open(argsIn->outRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) { perror("target open()"); exit(1); }
        
        result = dup2(targetFD, 1);
        if (result == -1) { perror("target dup2()"); exit(2); }
    }
}

// Used to kill all of the child processes within our
// ChildPID struct
void KillChildren(struct ChildPIDs *structIn)
{
    int i;
    for(i = 0; i < structIn->nChild; i++)
    {
        kill(structIn->cPID[i], SIGTERM);
    }
}

// initializes the ChildPID struct
void InitializeStruct(struct ChildPIDs *structIn)
{
    structIn->nChild = 0;
    structIn->inBackground = 0;
    structIn->latestFG = -5;
    structIn->exitMethod = 0;
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
void ProcessExitMethod(pid_t pidIn, int childExitMethod, int isInBackground)
{
    // Fill this in with code to process the exit code.
    if(isInBackground)
    {
        printf("\nbackground pid %d is done: ", pidIn);
    }
    else
    {
        printf("\n");
    }
    
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
            ProcessExitMethod(structIn->cPID[i], childExitMethod, 1); // 1 signifies it is in the background
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

