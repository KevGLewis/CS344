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
    int currChar = -5; // Tracks where we are when we print out every char
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
        numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
        if (numCharsEntered == -1)
            clearerr(stdin);
        else
            break; // Exit the loop - we've got input
    }
    
    return lineEntered;
}

int ParseLine(char **arg, char* lineEntered)
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
            arg[i] = malloc(sizeof(buffer));
            strcpy(arg[i], buffer);
        }
        
        else
        {
            arg[i] = malloc(sizeof(token));
            strcpy(arg[i], token);
        }
        i++;
        token = strtok(NULL, " ");
    }
    
    return i; // this will be equivalent to the number of arguments
}

void CleanArray(char **args, int nArgs)
{
    for(int i = 0; i < nArgs; i++)
    {
        free(args[i]);
        args[i] = NULL;
    }
}

int main(int argc, char* argv[])
{
    char *arg[ARGSIZE];
    int noExit = 1;
    
    for(int i = 0; i < ARGSIZE; i++)
    {
        arg[i] = NULL;
    }
    
    char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string + \n + \0
    int nArgs;
    
    while(noExit)
    {
        lineEntered = GetUserInput(); // Get input from the user
        
        nArgs = ParseLine(arg, lineEntered); // parse the input into the argument array
        
        noExit = ProcessLine(arg, nArgs);
        
        free(lineEntered);
        lineEntered = NULL;
        CleanArray(arg, nArgs);
    }
}
