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

void error(const char *msg);
void SendMessageSize(char* bufSize, char* buffRecv, int socketFD);
void CheckPassword(char* buffer, char* buffRecv, int socketFD, char* password);
void SendFileData(char* buffer, int socketFD);
void ReceiveFileData(char* buffer, int establishedConnectionFD, int size);
void ReceiveMessageSize(char* buffRecv, int socketFD);

#endif /* snd_recv_help_h */
