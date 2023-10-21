#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define MSG_MAX_LEN  1024
#define PORT 3000

int initReceiver(){

    struct sockaddr_in sin; // receiver server address
    memset(&sin, 0, sizeof(sin)); 
    sin.sin_family = AF_INET; // Connection to network long
    sin.sin_port = htons(PORT); // Host to network long
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network short 

    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor < 0){
        perror("ERROR in INIT RECEIVER Socket creation");
        return -1;
    }

    if (bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin))){
        close(socketDescriptor);
        perror("ERROR in INIT RECEIVER Socket creation");
        return -1;
    }
    printf("Oliver's Receiver Network Listening on UDP port %d:\n", PORT);

    return socketDescriptor;
}

// Critical section most likely using Pthreads here. 
int receiveMessage(int socketDescriptor, char* messageRx){

    struct sockaddr_in sinRemote; // contains sender address
    unsigned int sin_len = sizeof(sinRemote);
    int bytesRx = recvfrom(socketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr *) &sinRemote, &sin_len);

    if(bytesRx < 0){
        perror("Error in message received");
    }

    // Process string, make it null terminated so its easier to work with/process
    // Also we should be able to process strings of any size, consequently having hardcoded
    // maxlength will not work for us. 
    int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx : MSG_MAX_LEN - 1;
    messageRx[terminateIdx] = 0;

    return bytesRx;
}
// Refactor so that address is taken from CLI
int initSender(){ //get ip address for CLI when running code ./main IP_ADDR PORT NUMBER

    struct sockaddr_in sin; // receiver server address
    memset(&sin, 0, sizeof(sin)); 
    sin.sin_family = AF_INET; // Connection to network long
    sin.sin_port = htons(PORT); // Host to network long
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // Host to network short 

    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor < 0){
        perror("ERROR in INIT Sender Socket creation");
        return -1;
    }

    

}