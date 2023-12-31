#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "list.h"

// Constructor, grabs lists created in main() grabs console arguments to initialize sockets.
int initNetwork(char* sLocalPort,char* sRemotePort, char* sHostName, List* keyTXlist, List* screenRXlist, pthread_mutex_t* keyTXlistMutex, pthread_mutex_t* screenRXlistMutex);

int initReceiver();

void* receiveMessage(void* unused);

int initSender();

void* sendMessage(void* unused);

int Network_shutdown(void);

#endif