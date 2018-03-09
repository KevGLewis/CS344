//
//  snd_recv_help.c
//  DevEnviro
//
//  Created by Kevin Lewis on 3/8/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#include "snd_recv_help.h"

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

void SendMessageSize(char* bufSize, char* buffRecv, int socketFD)
{
    // Send the server the data size
    int charsWritten, charsRead;
    
    charsWritten = (int) send(socketFD, bufSize, strlen(bufSize), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(bufSize)) printf("CLIENT: WARNING: Not all data written to socket!\n");
    
    // Get return message from server
    // Clear out the buffer again for reuse
    charsRead = (int) recv(socketFD, buffRecv, sizeof(buffRecv) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    printf("CLIENT: I received this from the server: \"%s\"\n", buffRecv);
    
    if(strcmp(bufSize, buffRecv) != 0)
    {
        perror("Incorrect file size received from server");
        exit(1);
    }
}

void ReceiveMessageSize(char* buffRecv, int socketFD)
{
    int charsRead = 0;
    int charsWritten = 0;
    
    // Receive our message from the server, verify that it is ok
    charsRead = (int) recv(socketFD, buffRecv, sizeof(buffRecv) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    printf("CLIENT: I received this from the server: \"%s\"\n", buffRecv);
    
    charsWritten = (int) send(socketFD, buffRecv, strlen(buffRecv), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(buffRecv)) printf("CLIENT: WARNING: Not all data written to socket!\n");
}

void CheckPassword(char* buffer, char* buffRecv, int socketFD, char* password)
{
    int charsWritten, charsRead;
    // Load the array with our password and send it with our
    strcpy(buffer, password);
    charsWritten = (int) send(socketFD, buffer, strlen(buffer), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    
    // Receive our message from the server, verify that it is ok
    charsRead = (int) recv(socketFD, buffRecv, sizeof(buffRecv) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    printf("CLIENT: I received this from the server: \"%s\"\n", buffRecv);
    
    if(strcmp(buffRecv, "OK") != 0)
    {
        perror("Server password not accepted\n");
        exit(1);
    }
}

void SendFileData(char* buffer, int socketFD)
{
    // Loop the sending action until we are sure we
    int charsWritten = 0;
    int totalSent = 0;
    while(totalSent < strlen(buffer))
    {
        charsWritten = (int) send(socketFD, &buffer[totalSent], strlen(&buffer[totalSent]), 0); // Write to the server
        if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
        totalSent += charsWritten;
    }
}

void ReceiveFileData(char* buffer, int establishedConnectionFD, int size)
{
    char buffSpurt[256];
    int sizeDataRecv = 0;
    int charsRead = 0;
    
    while(sizeDataRecv < size)
    {
        memset(buffSpurt, '\0', 256);
        charsRead = (int) recv(establishedConnectionFD, buffSpurt, 255, 0);
        if (charsRead < 0) error("ERROR reading from socket");
        sizeDataRecv += strlen(buffSpurt);
        strcat(buffer, buffSpurt);
    }
}
