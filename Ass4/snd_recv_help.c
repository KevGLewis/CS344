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
    char plaintextBuffer[1056];
    char keyBuffer[1056];
    
    LoadFile(plaintextBuffer, plaintextFile);
    LoadFile(keyBuffer, keyFile);
    
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
            exit(2);
        }
    }
    
}

// Load a file into the buffer
void LoadFile(char* buffer, char* fileName)
{
    char* terminalLocation;
    char newBuffer[1056];
    
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
        sprintf(newBuffer, "%s", lineEntered);
        if(strstr(newBuffer, "\n") != NULL)
        {
            terminalLocation = strstr(newBuffer, "\n"); // Where is the terminal
            *terminalLocation = '\0'; // End the string early to wipe out the terminal
        }
        strcat(buffer, newBuffer);
    }
    fclose(fp);
    free(lineEntered);
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
    printf("The received password is %s\n", buffer);
    if(strcmp(buffer, password) != 0)
    {
        perror("Handshake Failed - Server\n");
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
    char buffSpurt[256];
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
void InitializeStructs(struct InputFileNames* ifn, struct ReturnMessage* rn)
{
    ifn->keyFileName = NULL;
    ifn->textToEncryptFileName = NULL;
    
    rn->message = NULL;
    rn->messageSize = 0;
}

// The input will be in the form of keyFileName " " parseTextFileName
void ParseInput(char* input, struct InputFileNames* fileNames)
{
    char *token;
    
    // The first input is the filename
    token = strtok(input, " ");
    fileNames->textToEncryptFileName = calloc(strlen(token) + 1, sizeof(char));
    strcpy(fileNames->textToEncryptFileName, token);
    
    // The second input is the text to encrpt file name
    token = strtok(NULL, " ");
    fileNames->keyFileName = calloc(strlen(token) + 1, sizeof(char));
    strcpy(fileNames->keyFileName, token);

}

// Set up the signal handler for the parent

// Free the dynamically allocated portions of the structs
void CleanupStructs(struct InputFileNames* ifn, struct ReturnMessage* rn)
{
    free(ifn->keyFileName);
    free(ifn->textToEncryptFileName);
    
    free(rn->message);
}

// Either encrypt or decrypt input. When encrpyt togg is 1, it will encrypt, if it is
// zero, it will decrypt
void CryptInput(char* returnBuffer, struct InputFileNames* fileNames, int encryptTogg)
{
    int i, x, y, sum;
    // Clear our return and set up the buffer for the other files
    memset(returnBuffer, '\0', sizeof(*returnBuffer));
    
    char ptBuffer[1056];
    memset(ptBuffer, '\0', sizeof(*ptBuffer));
    
    char keyBuffer[1056];
    memset(keyBuffer, '\0', sizeof(*keyBuffer));
    
    LoadFile(ptBuffer, fileNames->textToEncryptFileName);
    LoadFile(keyBuffer, fileNames->keyFileName);
    
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
            sum = (x - y + 26) % 27 + 65;
        }
        
        // Include the case where we have spaces
        if(sum == 91)
        {
            returnBuffer[i] = ' ';
        }
        else
        {
            returnBuffer[i] = (char) sum;
        }
    }
}

