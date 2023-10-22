#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

int initReceiver(char* port, struct sockaddr_in** localAddress);

int receiveMessage(int socketDescriptor, char* messageRx, struct sockaddr_in* sinLocal);

int initSender(const char* hostName,const char* port, struct sockaddr_in** remoteAddress); //get ip address for CLI when running code ./main IP_ADDR PORT NUMBER

int sendMessage(int socketDescriptor, char* messageRx, struct sockaddr_in* remoteAddress);

#endif