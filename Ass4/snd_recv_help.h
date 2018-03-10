//
//  snd_recv_help.h
//  DevEnviro
//
//  Created by Kevin Lewis on 3/8/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#ifndef snd_recv_help_h
#define snd_recv_help_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

struct InputFileNames
{
    char* keyFileName;
    char* textToEncryptFileName;
};

struct ReturnMessage
{
    int messageSize;
    char* message;
};

void error(const char *msg);
void VerifyInput(char* plaintextFile, char* keyFile);
void LoadFile(char* buffer, char* fileName);
void LoadPlaintext(char* buffer, char* filename);
int PasswordSend(char* buffer, int socketFD, char* password);
int PasswordReceive(char* buffer, int socketFD, char* password);
int SendFileData(char* buffer, int socketFD);
int ReceiveFileData(char* buffer, int establishedConnectionFD);
void InitializeStructs(struct InputFileNames* ifn, struct ReturnMessage* rn);
void ParseInput(char* input, struct InputFileNames* fileNames);
void CleanupStructs(struct InputFileNames* ifn, struct ReturnMessage* rn);
void CryptInput(char* returnBuffer, struct InputFileNames* fileNames, int EncryptTogg);


#endif /* snd_recv_help_h */
