// receiveMessage is a Producer
// sendMessage() is a Consumer

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "network.h"
#include "list.h"
#include <pthread.h>

#define MSG_MAX_LEN 1024

static struct sockaddr_in* sinLocal;
static int localSocketDescriptor;

static struct sockaddr_in* sinRemote;
static int remoteSocketDescriptor;

static char* localPort;
static char* remotePort;
static char* hostName;

static List* txList;
static List* rxList;

static pthread_t tReceiverPID;
static pthread_t tSenderPID;

static pthread_mutex_t* senderListMutex;
static pthread_mutex_t* receiverListMutex;


void initNetwork(char* sLocalPort,char* sRemotePort, char* sHostName, List* keyTXlist, List* screenRXlist,pthread_mutex_t* keyTXlistMutex, pthread_mutex_t* screenRXlistMutex){
    localPort = sLocalPort;
    remotePort = sRemotePort;
    hostName = sHostName;

    txList = keyTXlist; // List 1: -> Producer: Keyboard/Input, Consumer: Sender
    rxList = screenRXlist; // List 2 -> Producer: Receiver, Consumer Output

    initSender();
    initReceiver();

    pthread_create(
        &tReceiverPID, // PID
        NULL, // Attributes
        receiveMessage, // Function
        NULL
    );
    pthread_create(
        &tSenderPID, // PID
        NULL, // Attributes
        sendMessage, // Function
        NULL
    );
    senderListMutex = keyTXlistMutex;
    receiverListMutex = screenRXlistMutex;
}

void initReceiver(){

    struct addrinfo* localAddressInfo;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;

    if(getaddrinfo(NULL, localPort, &hints,&localAddressInfo)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
    }
    sinLocal = (struct sockaddr_in*)localAddressInfo->ai_addr;
    localSocketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (localSocketDescriptor == -1){
        close(localSocketDescriptor);
        perror("ERROR in INIT RECEIVER Socket creation");
    }

    if (bind(localSocketDescriptor, (struct sockaddr*) sinLocal, sizeof(*sinLocal)) == -1){
        close(localSocketDescriptor);
        perror("ERROR in INIT RECEIVER Socket creation");
    }
    printf("Oliver's Receiver Network Listening on UDP port %s:\n", localPort);
    
}

// Producer 
void* receiveMessage(void* unused){

    while(1){
        char messageRx[MSG_MAX_LEN];
        //struct sockaddr_in sinRemote; // contains sender address
        unsigned int sin_len = sizeof(sinLocal);
        int bytesRx = recvfrom(localSocketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr *) &sinLocal, &sin_len);

        if(bytesRx < 0){
            perror("Error in message received");
        }
        // Process string, make it null terminated so its easier to work with/process
        // Also we should be able to process strings of any size, consequently having hardcoded
        // maxlength will not work for us. 
        int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx : MSG_MAX_LEN - 1;
        messageRx[terminateIdx] = 0;

        // Critical Section add messageRx to list
        pthread_mutex_lock(receiverListMutex);
        List_prepend(rxList, messageRx);
        pthread_mutex_unlock(receiverListMutex);

    }
    
}
void initSender(){ 

    struct addrinfo* localAddressInfo;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if(getaddrinfo(hostName, remotePort, &hints, &localAddressInfo)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
    }
    sinRemote = (struct sockaddr_in*)localAddressInfo->ai_addr;

    remoteSocketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (remoteSocketDescriptor == -1){ 
        close(remoteSocketDescriptor);
        perror("ERROR in INIT Sender Socket creation");
    }
    
}

// Consumer
void* sendMessage(void* unused){

    while(1){
     
        //Critical section
        pthread_mutex_lock(senderListMutex);
        char* messageTx = (char*)List_remove(txList);
        pthread_mutex_unlock(senderListMutex);


        if(sendto(remoteSocketDescriptor, messageTx, MSG_MAX_LEN, 0, (struct sockaddr*)sinRemote, sizeof(*sinRemote)) == -1){
            perror("Unable to send the message");
        }
    }
    
}

void closeNetwork(){

    close(localSocketDescriptor);
    close(remoteSocketDescriptor);
    pthread_cancel(tReceiverPID);
    pthread_cancel(tSenderPID);
    pthread_join(tReceiverPID, NULL);
    pthread_join(tSenderPID, NULL);
}
