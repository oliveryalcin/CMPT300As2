// Main s-talk file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>
//#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "list.h"
#include "network.c"

int main(int argCount, char** args){
    // Check number of input arguments for validity
    if(argCount < 4){
        printf("Error: s-talk invalid request: too few arguments supplied\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }
    else if(argCount > 4){
        printf("Error: s-talk invalid request: too many arguments supplied\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }

    // input arguments:
    int my_port = atoi(args[1]);    // local port number
    char* rem_name = args[2];       // remote machine name
    int rem_port = atoi(args[3]);   // remote port number

    // check input arguments:
    if ( (my_port == 0) || (my_port < 1024) || (my_port > 49151) ){
        printf("Error: s-talk invalid request: invalid local port number (%s)\n", args[1]);
        printf("    Port numbers must be integers in the range 1024 to 49151.\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }
    if ( (rem_port == 0) || (rem_port < 1024) || (rem_port > 49151) ){
        printf("Error: s-talk invalid request: invalid remote port number (%s)\n", args[3]);
        printf("    Port numbers must be integers in the range 1024 to 49151.\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }

    printf("successful arguments supplied!\n");
    printf("s-talk %d %s %d\n", my_port, rem_name, rem_port);

    struct sockaddr_in* local;
    struct sockaddr_in* remote;

    int localSocket = initReceiver(args[1], &local);
    int remoteSocket = initSender(args[2], args[3], &remote);

    char messageToSend[MSG_MAX_LEN];
    char receivedMessage[MSG_MAX_LEN];



    // Code used for initial testing of the network 

    /*
    while (1) {

        // Get user input
        fgets(messageToSend, MSG_MAX_LEN, stdin);
        // Remove newline character from the input
        size_t len = strlen(messageToSend);
        if (len > 0 && messageToSend[len - 1] == '\n'){
            messageToSend[len - 1] = '\0';
        }
        // Send the message
        if (sendMessage(remoteSocket, messageToSend, remote) == -1){

            fprintf(stderr, "Error sending message\n");
            break;
        }

        printf("Message sent: %s\n", messageToSend);
        // Receive a message
        int bytesReceived = receiveMessage(localSocket, receivedMessage, local);
        if (bytesReceived == -1){
            fprintf(stderr, "Error receiving message\n");
            break;
        }

        // Print the received message
        printf("Received message: %s\n", receivedMessage);

    }
    */
    close(localSocket);
    close(remoteSocket);



    return 0;

}