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

static char* messageTx;


/* initNetwork - initialize the network*/
int initNetwork(char* sLocalPort,char* sRemotePort, char* sHostName, List* keyTXlist, List* screenRXlist,pthread_mutex_t* keyTXlistMutex, pthread_mutex_t* screenRXlistMutex){
    localPort = sLocalPort;
    remotePort = sRemotePort;
    hostName = sHostName;

    txList = keyTXlist; // List 1: -> Producer: Keyboard/Input, Consumer: Sender
    rxList = screenRXlist; // List 2 -> Producer: Receiver, Consumer Output

    senderListMutex = keyTXlistMutex;
    receiverListMutex = screenRXlistMutex;

    if (initSender() < 0){
        return -1;
    }
    if (initReceiver() < 0){
        return -1;
    }

    if (pthread_create(&tReceiverPID, NULL, receiveMessage, NULL) != 0){
        perror("Error creating receiver thread\n");
        return -1;
    }
    if (pthread_create(&tSenderPID,NULL,sendMessage,NULL) != 0){
        perror("Error creating sender thread\n");
        return -1;
    }
    return 1;
}


/* initReceiver - initialize the Receiver*/
int initReceiver(){
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;

    if(getaddrinfo(NULL, localPort, &hints, &localAddressInfoRX)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
        return -1;
    }
    sinLocal = (struct sockaddr_in*)localAddressInfoRX->ai_addr;
    localSocketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (localSocketDescriptor == -1){
        close(localSocketDescriptor);
        perror("ERROR in INIT RECEIVER Socket creation");
        return -1;
    }

    if (bind(localSocketDescriptor, (struct sockaddr*) sinLocal, sizeof(*sinLocal)) == -1){
        close(localSocketDescriptor);
        perror("ERROR in INIT RECEIVER Socket creation");
        return -1;
    }
    printf("Listening on UDP port %s:\n", localPort);
    return 1;
}


/* receiveMessage - RX thread will be running this function */
//                  it is a Producer.
void* receiveMessage(void* unused){
    while(1){
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
            // as soon as we receive ! we signal a shutdown
            signalShutdown();
            break;
        }
        
        // Critical Section add messageRx to list
        pthread_mutex_lock(receiverListMutex);
        {
            List_prepend(rxList, messageRx);
        }
        pthread_mutex_unlock(receiverListMutex);
        pthread_testcancel();
    }
    while(1){ pthread_testcancel(); }
    return NULL;
}


/* initSender - initialize the sender */
int initSender(){ 
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if(getaddrinfo(hostName, remotePort, &hints, &localAddressInfoTX)){ //initializes localAddressINFO and sets receiver to localhost
        perror("Unable to obtain local address info");
        return -1;
    }
    sinRemote = (struct sockaddr_in*)localAddressInfoTX->ai_addr;

    remoteSocketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if (remoteSocketDescriptor == -1){ 
        close(remoteSocketDescriptor);
        perror("ERROR in INIT Sender Socket creation");
        return -1;
    }
    return 1;
}


/* sendMessage - TX thread will be running this function */
//               it is a Consumer
void* sendMessage(void* unused){
    while(1){
        pthread_mutex_lock(senderListMutex);
        {
            messageTx = (char*)List_remove(txList); //Critical section
        }
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
    return NULL;
}


/* Network Shutdown */
int Network_shutdown(void){
    //printf("Network Shutdown...\n");

    // receiv thread cancel and join
    if (pthread_cancel(tReceiverPID) != 0){
        perror("Error cancelling receivr\n");
        return -1;
    }
    //printf("    Network recvthread cancelled\n");
    if (pthread_join(tReceiverPID, NULL) != 0){
        perror("Error joining receivr\n");
        return -1;
    }
    //printf("    Network recvthread joined\n");

    // sender thread cancel and join
    if (pthread_cancel(tSenderPID) != 0){
        perror("Error cancelling sender\n");
        return -1;
    }
    //printf("    Network sendthread cancelled\n");
    if (pthread_join(tSenderPID, NULL) != 0){
        perror("Error joining sender\n");
        return -1;
    }
    //printf("    Network sendthread joined\n");

    // free address info
    freeaddrinfo(localAddressInfoRX);
    freeaddrinfo(localAddressInfoTX);

    // close sockets
    if (close(localSocketDescriptor) != 0){
        perror("Error closing localSocketDescriptor\n");
        return -1;
    }
    //printf("    Network localsocket closed\n");
    if (close(remoteSocketDescriptor) != 0){
        perror("Error closing remoteSocketDescriptor\n");
        return -1;
    }
    //printf("    Network remotesocket closed\n");

    // dereference pointers
    sinLocal = NULL;
    sinRemote = NULL;
    localPort = NULL;
    remotePort = NULL;
    hostName = NULL;
    localAddressInfoRX = NULL;
    localAddressInfoTX = NULL;
    txList = NULL;
    rxList = NULL;
    senderListMutex = NULL;
    receiverListMutex = NULL;
    messageTx = NULL;

    return 1;
}
