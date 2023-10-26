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
#include "s-talk.h"
#include <pthread.h>

#define MSG_MAX_LEN 1024

static struct sockaddr_in* sinLocal;
static int localSocketDescriptor;

static struct sockaddr_in* sinRemote;
static int remoteSocketDescriptor;

static char* localPort;
static char* remotePort;
static char* hostName;

static struct addrinfo* localAddressInfoRX;
static struct addrinfo* localAddressInfoTX;

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

    senderListMutex = keyTXlistMutex;
    receiverListMutex = screenRXlistMutex;

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
}


void initReceiver(){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;

    if(getaddrinfo(NULL, localPort, &hints, &localAddressInfoRX)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
    }
    sinLocal = (struct sockaddr_in*)localAddressInfoRX->ai_addr;
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
        //printf("receiveMessage(...)");
        char messageRx[MSG_MAX_LEN];
        unsigned int sin_len = sizeof(sinLocal);
        int bytesRx = recvfrom(localSocketDescriptor, messageRx, MSG_MAX_LEN, 0, (struct sockaddr *) &sinLocal, &sin_len);

        if(bytesRx < 0){
            perror("Error in message received");
        }
        // Process string, make it null terminated so its easier to work with/process
        // Also we should be able to process strings of any size, consequently having hardcoded
        // maxlength will not work for us. 
        int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx : MSG_MAX_LEN - 1;
        messageRx[terminateIdx] = '\0';

        if(messageRx[0] == '!' && messageRx[1] == '\0'){
            // as soon as we receive ! we quit
            signalShutdown();
            break;
        }
        
        // Critical Section add messageRx to list
        pthread_mutex_lock(receiverListMutex);
        List_prepend(rxList, messageRx);
        pthread_mutex_unlock(receiverListMutex);
        pthread_testcancel();
    }
    while(1){ pthread_testcancel(); }
    //return NULL;
}


void initSender(){ 
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if(getaddrinfo(hostName, remotePort, &hints, &localAddressInfoTX)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
    }
    sinRemote = (struct sockaddr_in*)localAddressInfoTX->ai_addr;

    remoteSocketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (remoteSocketDescriptor == -1){ 
        close(remoteSocketDescriptor);
        perror("ERROR in INIT Sender Socket creation");
    }
    
}

// Consumer
void* sendMessage(void* unused){

    while(1){
        //printf("sendMessage(...)");
        //Critical section
        pthread_mutex_lock(senderListMutex);
        char* messageTx = (char*)List_remove(txList);
        pthread_mutex_unlock(senderListMutex);
        pthread_testcancel();

        if(messageTx != NULL) {
            if(sendto(remoteSocketDescriptor, messageTx, MSG_MAX_LEN, 0, (struct sockaddr*)sinRemote, sizeof(*sinRemote)) == -1){
                perror("Unable to send the message");
            }
            if(messageTx[0] == '!' && messageTx[1] == '\0'){
                // This way we still SEND the ! so that the other person will quit too!
                free(messageTx);
                signalShutdown();
                break;
            }
            free(messageTx);
        }
    }
    while(1){ pthread_testcancel(); }
    //return NULL;
}

void Network_shutdown(){
    printf("Network Shutdown...\n");

    // receiv thread cancel and join
    if (pthread_cancel(tReceiverPID) != 0){ printf("Error cancelling receivr\n");}
    printf("    Network recvthread cancelled\n");
    if (pthread_join(tReceiverPID, NULL) != 0){ printf("Error joining receivr\n");}
    printf("    Network recvthread joined\n");

    // sender thread cancel and join
    if (pthread_cancel(tSenderPID) != 0){ printf("Error cancelling sender\n");}
    printf("    Network sendthread cancelled\n");
    if (pthread_join(tSenderPID, NULL) != 0){ printf("Error joining sender\n");}
    printf("    Network sendthread joined\n");

    // free address info
    freeaddrinfo(localAddressInfoRX);
    freeaddrinfo(localAddressInfoTX);

    // close sockets
    close(localSocketDescriptor);
    printf("    Network localsocket closed\n");
    close(remoteSocketDescriptor);
    printf("    Network remotesocket closed\n");
}
