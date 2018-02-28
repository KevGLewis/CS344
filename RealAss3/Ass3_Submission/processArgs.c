//
//  processArgs.c
//  Assignment-3
//
//  Created by Kevin Lewis on 2/20/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//



#include "processArgs.h"

// Process the exit command
int ProcessExit(struct Arguments *argsIn, struct ChildPIDs* structIn)
{
    if(argsIn->nArgs > 1) // Verify whether or not we have enough characters
    {
        printf("\nError: Too Many Arguments");
        fflush(stdout);
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
            fflush(stdout);
        }
    }
    else if(argsIn->nArgs > 2)
    {
        printf("\nError: Too Many Arguments\n");
        fflush(stdout);
    }
    else
    {
        if(chdir(argsIn->args[1]) != 0) // if we do have the proper arguments, then try to change directory
        {
            printf("\nError: ");
            fflush(stdout);
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
        fflush(stdout);
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
    int result;
    
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
            result = execvp(argsIn->args[0], argsIn->args);
            if( result == -1)
            {
                printf("%s, no such file or directory\n", argsIn->args[0]);
                fflush(stdout);
                exit(1);
            }
            break;
        default:
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
                spawnPid = waitpid(spawnPid, &childExitMethod, 0);
                structIn->exitMethod = childExitMethod;
            }
            break;
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
    if(argsIn->inRedirect != NULL)
    {
        sourceFD = open(argsIn->inRedirect, O_RDONLY);
        if (sourceFD == -1)
        {
            printf("Cannot open %s for input\n", argsIn->inRedirect);
            fflush(stdout);
            exit(1);
        }
        
        result = dup2(sourceFD, 0);
        if (result == -1) { perror("source dup2()"); exit(2); }
    }
    else if(argsIn->inBackground)
    {
        if (devNull == -1) { perror("dev/null error"); exit(1); }
        result = dup2(devNull, 0);
        if (result == -1) { perror("source dup2()"); exit(2); }
    }
    
    // Take care of output redirection
    // redirect input to dev/null if nothing provided
    if(argsIn->outRedirect != NULL)
    {
        targetFD = open(argsIn->outRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) { perror("target open()"); exit(1); }
        
        result = dup2(targetFD, 1);
        if (result == -1) { perror("target dup2()"); exit(2); }
    }
    if(argsIn->inBackground)
    {
        if (devNull == -1) { perror("dev/null error"); exit(1); }
        result = dup2(devNull, 1);
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

// Add the child PID to our child PID struct
void AddChildPID(struct ChildPIDs *structIn, pid_t cPIDIn)
{
    // First, we'll print out the background pid
    printf("background pid is %d\n", cPIDIn);
    fflush(stdout);
    
    if(structIn->nChild >= MAX_CPID)
    {
        perror("Too Many Children");
        fflush(stdout);
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
        printf("background pid %d is done: ", pidIn);
        fflush(stdout);
    }
    
    if (WIFEXITED(childExitMethod) != 0)
    {
        int exitStatus = WEXITSTATUS(childExitMethod);
        printf("exit value of %d\n", exitStatus);
        fflush(stdout);
        
    }
    else if (WIFSIGNALED(childExitMethod) != 0)
    {
        int termSignal = WTERMSIG(childExitMethod);
        printf("terminated by signal %d\n", termSignal);
        fflush(stdout);
    }
    else
    {
        printf("Hull Breach");
        fflush(stdout);
    }
    
    
}

void CleanChildren(struct ChildPIDs *structIn)
{
    int i, j;
    for(i = 0; i < structIn->nChild; i++)
    {
        if(structIn->cPID[i] == -5)
        {
            for(j = i; j < structIn->nChild - 1; j++)
            {
                structIn->cPID[j] = structIn->cPID[j + 1];
            }
            // we are shifting the PIDS down one index, so we need
            // make sure we capture the index we just checked
            i--;
            structIn->cPID[structIn->nChild - 1] = -5;
            structIn->nChild--;
        }
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
            fflush(stdout);
            exit(1);
        }
        else if(childPID_actual != 0)
        {
            ProcessExitMethod(structIn->cPID[i], childExitMethod, 1); // 1 signifies it is in the background
            structIn->cPID[i] = -5;
        }
    }
    
    CleanChildren(structIn);
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
        SIGINT_action.sa_handler = CatchSIGINT_Child;
        sigfillset(&SIGINT_action.sa_mask);
        SIGINT_action.sa_flags = 0;
        sigaction(SIGINT, &SIGINT_action, NULL);
    }
    
}

void CatchSIGINT_Child(int signo)
{
    char* message = "terminated by signal 2\n";
    write(STDOUT_FILENO, message, 24);
}

