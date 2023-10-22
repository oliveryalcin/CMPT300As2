#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "network.h"

#define MSG_MAX_LEN 1024

int initReceiver(char* port, struct sockaddr_in** localAddress){

    // getaddrinfo() will sort out all of this, no need to reset memory. This will make received address info garbage
    //struct sockaddr_in sin; // receiver server address
    //memset(&sin, 0, sizeof(sin)); 
    //sin.sin_family = AF_INET; // Connection to network long
    //sin.sin_port = htons(PORT); // Host to network short 
    //sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network long 
    struct addrinfo* localAddressInfo;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    struct sockaddr_in* sin;
    if(getaddrinfo(NULL, port, &hints,&localAddressInfo)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
        return -1;
    }
    sin = localAddressInfo->ai_addr;
    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1){
        perror("ERROR in INIT RECEIVER Socket creation");
        return -1;
    }

    if (bind(socketDescriptor, (struct sockaddr*) sin, sizeof(*sin)) == -1){
        close(socketDescriptor);
        perror("ERROR in INIT RECEIVER Socket creation");
        return -1;
    }
    printf("Oliver's Receiver Network Listening on UDP port %s:\n", port);
    (*localAddress) = sin;
    return socketDescriptor;
}

// Critical section most likely using Pthreads here. 
int receiveMessage(int socketDescriptor, char* messageRx, struct sockaddr_in* sinLocal){

    //struct sockaddr_in sinRemote; // contains sender address
    unsigned int sin_len = sizeof(sinLocal);
    int bytesRx = recvfrom(socketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr *) &sinLocal, &sin_len);

    if(bytesRx < 0){
        perror("Error in message received");
        return -1;
    }

    // Process string, make it null terminated so its easier to work with/process
    // Also we should be able to process strings of any size, consequently having hardcoded
    // maxlength will not work for us. 
    int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx : MSG_MAX_LEN - 1;
    messageRx[terminateIdx] = 0;

    return bytesRx;
}
// Refactor so that address is taken from CLI
int initSender(const char* hostName,const char* port, struct sockaddr_in** remoteAddress){ //get ip address for CLI when running code ./main IP_ADDR PORT NUMBER

    //struct sockaddr_in sin; // receiver server address
    //memset(&sin, 0, sizeof(sin)); 
    //sin.sin_family = AF_INET; // Connection to network long
    //sin.sin_port = htons(PORT); // Host to network long
    //sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network short 
    struct addrinfo* localAddressInfo;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    struct sockaddr_in* sin;

    if(getaddrinfo(hostName, port, &hints, &localAddressInfo)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
        return -1;
    }
    sin = localAddressInfo->ai_addr;

    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1){ 
        perror("ERROR in INIT Sender Socket creation");
        return -1;
    }
    (*remoteAddress) = sin;
    return socketDescriptor;
}
// Critical section
int sendMessage(int socketDescriptor, char* messageRx, struct sockaddr_in* remoteAddress){

    if(sendto(socketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr*)remoteAddress, sizeof(*remoteAddress)) != -1){
        return 0; //success 
    }
    
    return -1;
}
