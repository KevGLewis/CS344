//
//  bashShell.c
//  Assignment-3
//
//  Created by Kevin Lewis on 2/20/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#include "processArgs.h"

char* GetUserInput()
{
    int numCharsEntered = -5; // How many chars we entered
    char* lineEntered = NULL;
    size_t bufferSize = 0; // Holds how large the allocated buffer is
    int nArgs;
    
    // Get input from the user
    while(1)
    {
        nArgs = 0; // set the number of arguments to zero
        printf(": ");
        fflush(stdout); // flush the output
        // Get a line from the user
        numCharsEntered = (int) getline(&lineEntered, &bufferSize, stdin);
        if (numCharsEntered == -1)
            clearerr(stdin);
        else
            break; // Exit the loop - we've got input
    }
    
    return lineEntered;
}

void ParseLine(struct Arguments *argsIn, char* lineEntered)
{
    int i = 0; // will keep track of where we are in our argument array
    char buffer [50];
    char *token;
    
    lineEntered[strcspn(lineEntered, "\n")] = '\0'; // Get rid of the new line escape
    
    token = strtok(lineEntered, " ");
    
    while (token != NULL)
    {
        // figure out if the argument is the process ID and fill it in
        if(strcmp(token, "$$") == 0)
        {
            sprintf (buffer, "%d", getpid());
            argsIn->args[i] = malloc(sizeof(buffer));
            strcpy(argsIn->args[i], buffer);
        }
        
        else
        {
            argsIn->args[i] = malloc(sizeof(token));
            strcpy(argsIn->args[i], token);
        }
        i++;
        token = strtok(NULL, " ");
    }
    
    argsIn->nArgs = i; // this will be equivalent to the number of arguments
}

// Initialize the arguments tructure, setting what is required to zero
void InitializeArgStruct(struct Arguments *argsIn)
{
    for(int i = 0; i < ARGSIZE; i++)
    {
        argsIn->args[i] = NULL;
    }
    
    argsIn->nArgs = 0;
    argsIn->useArgs = -1;
    argsIn->inRedirect = NULL;
    argsIn->outRedirect = NULL;
    argsIn->inBackground = 0;
    
}

// clean up the arguments array
void CleanStruct(struct Arguments *argsIn)
{
    for(int i = 0; i < argsIn->nArgs; i++)
    {
        free(argsIn->args[i]);
        argsIn->args[i] = NULL;
    }
    
    argsIn->nArgs = 0;
    argsIn->useArgs = -1;
    free(argsIn->inRedirect);
    argsIn->inRedirect = NULL;
    free(argsIn->outRedirect);
    argsIn->outRedirect = NULL;
    argsIn->inBackground = 0;
}

// Let's figure out if we need to turn off the background functionality
// if so then we will toggle it between off and on
void HandleBackgroundSwitch()
{
    if(SwitchBackground == 1)
    {
        if(BackgroundOn == 1)
        {
            BackgroundOn = 0;
            printf("Entering foreground-only mode (& is now ignored)");
        }
        else
        {
            BackgroundOn = 1;
            printf("Exiting foreground-only mode");
        }
        SwitchBackground = 0;
    }
}

void runProgram()
{
    // first, set the background on and switch variables to their correct
    // values
    BackgroundOn = 1;
    SwitchBackground = 0;
    
    struct Arguments ArgsOut;
    InitializeArgStruct(&ArgsOut);
    struct ChildPIDs cPIDs; // set up our struct to hold our Child PIDs
    InitializeStruct(&cPIDs);
    
    char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
    
    int noExit = 1;
    
    while(noExit)
    {
        CheckProcesses(&cPIDs);
        HandleBackgroundSwitch();
        lineEntered = GetUserInput(); // Get input from the user
        
        ParseLine(&ArgsOut, lineEntered); // parse the input into the argument array
        
        noExit = ProcessLine(&ArgsOut, &cPIDs);
        
        free(lineEntered);
        lineEntered = NULL;
        CleanStruct(&ArgsOut);
    }
}

void catchSIGTSTP_Parent(int signo);

int main(int argc, char* argv[])
{
    
    struct sigaction SIGTSTP_action = {{0}}, ignore_action = {{0}};
    
    SIGTSTP_action.sa_handler = catchSIGTSTP_Parent;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    
    ignore_action.sa_handler = SIG_IGN;
    //sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGINT, &ignore_action, NULL); // Ignore we don't want the shell to end
    sigaction(SIGTSTP, &SIGTSTP_action, NULL); // Ignore for the time being
    sigaction(SIGHUP, &ignore_action, NULL);
    sigaction(SIGQUIT, &ignore_action, NULL);
    
    runProgram();
}

void catchSIGTSTP_Parent(int signo)
{
    SwitchBackground = 1;
}
