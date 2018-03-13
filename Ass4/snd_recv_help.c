//
//  snd_recv_help.c
//  DevEnviro
//
//  Created by Kevin Lewis on 3/8/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#include "snd_recv_help.h"
void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

void VerifyInput(char* plaintextFile, char* keyFile)
{
    int i, test1, test2;
    char* plaintextBuffer = NULL;
    char* keyBuffer = NULL;
    
    LoadFile(&plaintextBuffer, plaintextFile);
    LoadFile(&keyBuffer, keyFile);
    
    // Verify that our keyBuffer is longer than our plaintextBuffer
    if(strlen(keyBuffer) < strlen(plaintextBuffer))
    {
        fprintf(stderr, "Error: key \'%s\' is too short\n", keyFile);
        exit(2);
    }
    
    // Verify the plain text only contains capital letters and spaces
    for(i = 0; i < strlen(plaintextBuffer); i++)
    {
        // Is the character a capital letter?
        test1 = (int) plaintextBuffer[i] >= 65 && (int) plaintextBuffer[i] <= 90;
        
        // Is the character a space?
        test2 = (int) plaintextBuffer[i] == 32;
        
        if(!test1 && !test2)
        {
            fprintf(stderr, "Error: file \'%s\' has bad characters\n", plaintextFile);
            free(plaintextBuffer);
            free(keyBuffer);
            exit(2);
        }
    }
    
    free(plaintextBuffer);
    free(keyBuffer);
    
}

// Load a file into the buffer
void LoadFile(char** buffer, char* fileName)
{
    char* terminalLocation;
    int arraySize = 1056;
    *buffer = calloc(arraySize, sizeof(char));
    FILE *fp = fopen(fileName, "r");
    if(fp == NULL)
    {
        perror("Unable to open file");
        exit(1);
    }
    
    size_t bufferSize = 0; // Holds how large the allocated buffer is
    char* lineEntered = NULL; // Points to a buffer allocated by getline() that holds our entered string
    
    // Read the file line by line and remote the new line escape
    while(getline(&lineEntered, &bufferSize, fp) != -1) //read in the line.
    {
        if(strstr(lineEntered, "\n") != NULL)
        {
            terminalLocation = strstr(lineEntered, "\n"); // Where is the terminal
            *terminalLocation = '\0'; // End the string early to wipe out the terminal
        }
        
        while(arraySize < strlen(lineEntered))
        {
            ExpandDynArray(buffer, &arraySize);
        }
        
        strcat(*buffer, lineEntered);
    }
    fclose(fp);
    free(lineEntered);
}

void ExpandDynArray(char** buffer, int *arraySize)
{
    char* newBuffer = calloc(*arraySize, sizeof(char));
    memcpy(newBuffer, *buffer, *arraySize);
    free(*buffer);
    
    *arraySize = *arraySize * 2;
    *buffer = calloc(*arraySize, sizeof(char));
    memcpy(*buffer, newBuffer, *arraySize / 2);
}


// Return 1 if it was successful, and 0 if it was not
int PasswordSend(char* buffer, int socketFD, char* password)
{
    // Clear the arrays
    memset(buffer, '\0', sizeof(&buffer)); // Clear the array
    sprintf(buffer, "%s", password); // Load the buffer with our password
    SendFileData(buffer, socketFD);
    ReceiveFileData(buffer, socketFD);
    
    if(strcmp(buffer, "OK") != 0)
    {
        perror("Handshake Failed - Client\n");
        return 0;
    }
    
    return 1;
}

// Returns 1 if the reception was successful, and 0 if it was not
int PasswordReceive(char* buffer, int socketFD, char* password)
{
    // Wait to receive the handshake password from a client
    ReceiveFileData(buffer, socketFD);
    if(strcmp(buffer, password) != 0)
    {
        perror("Handshake Failed - Server\n");
        memset(buffer, '\0', sizeof(&buffer)); // Clear the array
        sprintf(buffer, "%s", "INCORRECT PASSWORD"); // Load the buffer with our password
        SendFileData(buffer, socketFD);
        return 0;
    }
    
    memset(buffer, '\0', sizeof(&buffer)); // Clear the array
    sprintf(buffer, "%s", "OK"); // Load the buffer with our password
    SendFileData(buffer, socketFD);
    
    return 1;
}

// All messages will not end in @@, so we shall add it
int SendFileData(char* buffer, int socketFD)
{
    // Add our ending text
    strcat(buffer, "@@");
    
    // Loop the sending action until we are sure we
    int charsWritten = 0;
    int totalSent = 0;
    while(totalSent < strlen(buffer))
    {
        charsWritten = (int) send(socketFD, &buffer[totalSent], strlen(&buffer[totalSent]), 0); // Write to the server
        if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
        totalSent += charsWritten;
    }
    
    return 0;
}

// When we receive the file, we want to ensure that all messages end with @@
int ReceiveFileData(char* buffer, int establishedConnectionFD)
{
    char buffSpurt[1056];
    int charsRead = 0;
    memset(buffer, '\0', sizeof(*buffer));
    
    while(strstr(buffer, "@@") == NULL)
    {
        memset(buffSpurt, '\0', sizeof(buffSpurt));
        charsRead = (int) recv(establishedConnectionFD, buffSpurt, 255, 0);
        if (charsRead < 0) error("ERROR reading from socket");
        strcat(buffer, buffSpurt);
    }
    
    int terminalLocation = (int) (strstr(buffer, "@@") - buffer); // Where is the terminal
    buffer[terminalLocation] = '\0'; // End the string early to wipe out the terminal
    
    return 0;
}

// Initialize the input and return message structs
void InitializeStructs(struct InputFileNames* ifn)
{
    memset(ifn->keyFileName, '\0', sizeof(ifn->keyFileName));
    memset(ifn->textToEncryptFileName, '\0', sizeof(ifn->textToEncryptFileName));
}

// The input will be in the form of keyFileName " " parseTextFileName
void ParseInput(char* input, struct InputFileNames* fileNames)
{
    char *token;
    
    // The first input is the filename
    token = strtok(input, " ");
    strcpy(fileNames->textToEncryptFileName, token);
    
    // The second input is the text to encrpt file name
    token = strtok(NULL, " ");
    strcpy(fileNames->keyFileName, token);

}

// Free the dynamically allocated portions of the structs
void CleanupStructs(struct InputFileNames* ifn)
{
    memset(ifn->keyFileName, '\0', sizeof(ifn->keyFileName));
    memset(ifn->textToEncryptFileName, '\0', sizeof(ifn->textToEncryptFileName));
}

// Either encrypt or decrypt input. When encrpyt togg is 1, it will encrypt, if it is
// zero, it will decrypt
void CryptInput(char* returnBuffer, struct InputFileNames* fileNames, int encryptTogg)
{
    int i, x, y, sum;
    
    char* ptBuffer = NULL;
    
    char* keyBuffer = NULL;
    
    LoadFile(&ptBuffer, fileNames->textToEncryptFileName);
    LoadFile(&keyBuffer, fileNames->keyFileName);
    
    for(i = 0; i < strlen(ptBuffer); i++)
    {
        // Need to account for a space
        if(ptBuffer[i] == ' ')
        {
            x = 26;
        }
        else
        {
            x = (int) ptBuffer[i] - 65;
        }

        // Need to account for a space
        if(keyBuffer[i] == ' ')
        {
            y = 26;
        }
        else
        {
            y = (int) keyBuffer[i] - 65;
        }
        
        if(encryptTogg == 1)
        {
            sum = (x + y) % 27 + 65;
        }
        else
        {
            sum = (x - y + 27) % 27 + 65;
        }
        
        if(ptBuffer[i] == '\n')
        {
            returnBuffer[i] = '\n';
        }
        
        // Include the case where we have spaces
        else if(sum == 91)
        {
            returnBuffer[i] = ' ';
        }
        else
        {
            returnBuffer[i] = (char) sum;
        }
    }
    
    // Add the new line
    strcat(returnBuffer, "\n");
    
    free(ptBuffer);
    free(keyBuffer);
}

