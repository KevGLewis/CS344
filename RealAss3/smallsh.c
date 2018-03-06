//
//  bashShell.c
//  Assignment-3
//
//  Created by Kevin Lewis on 2/20/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#include "processArgs.h"

// Not quite sure how to handle the background process, so I'm going to use global
// variables and switch them as needed

int BackgroundOnToggle;
int SwitchBackgroundToggle;

// Let's figure out if we need to turn off the background functionality
// if so then we will toggle it between off and on
void HandleBackgroundSwitch()
{
    if(SwitchBackgroundToggle == 1)
    {
        if(BackgroundOnToggle == 1)
        {
            BackgroundOnToggle = 0;
            printf("Entering foreground-only mode (& is now ignored)\n");
            fflush(stdout);
        }
        else
        {
            BackgroundOnToggle = 1;
            printf("Exiting foreground-only mode\n");
            fflush(stdout);
        }
        SwitchBackgroundToggle = 0;
    }
}

void ToggleBackground()
{
    SwitchBackgroundToggle = 1;
}

void InitializeBackground()
{
    // first, set the background on and switch variables to their correct
    // values
    BackgroundOnToggle = 1;
    SwitchBackgroundToggle = 0;
}

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
    if(argsIn->nArgs != 0)
    {
        if (strcmp(argsIn->args[argsIn->nArgs - 1], "&") == 0)
        {
            argsIn->inBackground = 1;
        }
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
    if(BackgroundOnToggle == 0)
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

char* GetUserInput()
{
    int numCharsEntered = -5; // How many chars we entered
    char* lineEntered = NULL;
    size_t bufferSize = 0; // Holds how large the allocated buffer is
    
    // Get input from the user
    while(1)
    {
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

// Used to expand the PID in the str
void ExpandPID(char** lineEntered)
{
    char *substr1, *substr2;
    int index, remaining;
    char *dest = strstr(*lineEntered, "$$");
    
    pid_t newPID = getpid();
    char pidStr[100];
    memset(&pidStr[0], 0, sizeof(pidStr));
    sprintf(pidStr, "%d", newPID); // pidStr now holds our PID
    
    while( dest != NULL )
    {
        index = (int)(dest - *lineEntered); // this is the index of $$
        remaining = (int)strlen(*lineEntered) - index - 1; // we need to get rid of $$ so we have to subtract an additional
        substr1 = (char*)calloc(index, sizeof(char));
        substr2 = (char*)calloc(remaining, sizeof(char));
        
        if(substr1 != NULL)
        {
            strncpy(substr1, *lineEntered, index); // this will hold everything before the $$
        }
        strncpy(substr2, *lineEntered + index + 2, remaining);
        
        // now, replace the entered line with the expanded line
        free(*lineEntered);
        
        *lineEntered = (char*)calloc(strlen(substr1) + strlen(pidStr) + strlen(substr2), sizeof(char));
        sprintf(*lineEntered, "%s%s%s", substr1, pidStr, substr2);
        
        //free the bits
        free(substr1);
        free(substr2);
        
        dest = strstr(*lineEntered, "$$");
    }
}

// Parses the line entered into a bunch of arguments
void ParseLine(struct Arguments *argsIn, char* lineEntered)
{
    int i = 0; // will keep track of where we are in our argument array
    char *token;
    
    lineEntered[strcspn(lineEntered, "\n")] = '\0'; // Get rid of the new line escape
    
    token = strtok(lineEntered, " ");
    
    while (token != NULL)
    {

        argsIn->args[i] = malloc(sizeof(token));
        strcpy(argsIn->args[i], token);
        i++;
        token = strtok(NULL, " ");
    }
    
    argsIn->nArgs = i; // this will be equivalent to the number of arguments
}

void InitializeChildStruct(struct ChildPIDs *childIn)
{
    int i;
    childIn->exitMethod = -5;
    childIn->latestFG = -5;
    childIn->nChild = 0;
    for(i = 0; i < MAX_CPID; i++)
    {
        childIn->cPID[i] = -5;
    }
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

void runProgram()
{
    
    char firstChar;
    InitializeBackground();
    
    struct Arguments ArgsOut;
    InitializeArgStruct(&ArgsOut);
    struct ChildPIDs cPIDs; // set up our struct to hold our Child PIDs
    InitializeChildStruct(&cPIDs);
    
    char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
    
    int noExit = 1;
    
    while(noExit)
    {
        CheckProcesses(&cPIDs);
        HandleBackgroundSwitch();
        lineEntered = GetUserInput(); // Get input from the user
        firstChar = lineEntered[0];
        
        if(firstChar != '#' && firstChar != '\0'
           && firstChar != '\n') // don't do any additional processing if it is a comment line
        {
            ExpandPID(&lineEntered);
            ParseLine(&ArgsOut, lineEntered); // parse the input into the argument array
            noExit = ProcessLine(&ArgsOut, &cPIDs);
        }
        
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
    
    runProgram();
}

void catchSIGTSTP_Parent(int signo)
{
    ToggleBackground();
}
