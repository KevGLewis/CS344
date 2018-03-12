//
//  snd_recv_help.h
//  DevEnviro
//
//  Created by Kevin Lewis on 3/8/18.
//  Copyright © 2018 Kevin Lewis. All rights reserved.
//

#ifndef snd_recv_help_h
#define snd_recv_help_h
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

struct InputFileNames
{
    char keyFileName[256];
    char textToEncryptFileName[256];
};


void error(const char *msg);
void VerifyInput(char* plaintextFile, char* keyFile);
void LoadFile(char** buffer, char* fileName);
void LoadPlaintext(char* buffer, char* filename);
int PasswordSend(char* buffer, int socketFD, char* password);
int PasswordReceive(char* buffer, int socketFD, char* password);
int SendFileData(char* buffer, int socketFD);
int ReceiveFileData(char* buffer, int establishedConnectionFD);
void InitializeStructs(struct InputFileNames* ifn);
void ParseInput(char* input, struct InputFileNames* fileNames);
void CleanupStructs(struct InputFileNames* ifn);
void CryptInput(char* returnBuffer, struct InputFileNames* fileNames, int EncryptTogg);
void SetupSignalHandlers(void);
void catchSIGCHLD(int signo);
void catchSIGTERM(int signo);
void ExpandDynArray(char** buffer, int *arraySize);


#endif /* snd_recv_help_h */
