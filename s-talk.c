// Main s-talk file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

//#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "list.h"
#include "keyboard.h"
#include "network.h"
#include "screen.h"

/* print_terminal : prints characters to screen (terminal) */
void print_terminal(){

}


/* main function */
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

    // TODO!!!
    // setup the threads, sockets, etc.

    // Create two lists:
    List* keyTXlist = List_create();    
    List* screenRXlist = List_create();

    // Create shared mutexes:
    // mutex to protect concurrent access to keyTXlist
    pthread_mutex_t keyTXlistMutex = PTHREAD_MUTEX_INITIALIZER;
    // mutex to protect concurrent access to screenRXlist
    pthread_mutex_t screenRXlistMutex = PTHREAD_MUTEX_INITIALIZER;

    // Shared Condition Variables:
    //pthread_cond_t keyTXlistCondVar = PTHREAD_COND_INITIALIZER;
    //pthread_cond_t screenRXlistCondVar = PTHREAD_COND_INITIALIZER;

    // Startup the four modules
    if (Keyboard_init(keyTXlist, &keyTXlistMutex) < 0 ) { 
        return -1; 
    }
    if (Screen_init(keyTXlist, &keyTXlistMutex) < 0 ) { 
        return -1; 
    }
    initNetwork(args[1],args[3],args[2], keyTXlist, screenRXlist, &keyTXlistMutex, &screenRXlistMutex);

    // TODO...


    // Shutdown my modules
    Keyboard_shutdown();
    Screen_shutdown();
    closeNetwork();
    // TODO...

    //printf("s-talk DONE");

    //pthread_t tScreen;  // Screen thread (prints characters to terminal screen)
    //pthread_t tSend;    // Send thread (sends data to remote UNIX process over network using UDP)
    //pthread_t tRecv;    // Receive thread (awaits a UDP datagram)


    // Local and Remote address information used for sending information.
    // Essentially used to pass as arugments for receiveMessage(..) and sendMessage(..)  




    // Code used for initial testing of the network 
    /*
    const int MSG_MAX_LEN = 1024;
    char messageToSend[MSG_MAX_LEN];
    char receivedMessage[MSG_MAX_LEN];
    while (1) {

        // Get user input
        fgets(messageToSend, MSG_MAX_LEN, stdin);
        // Remove newline character from the input
        size_t len = strlen(messageToSend);
        if (len > 0 && messageToSend[len - 1] == '\n'){
            messageToSend[len - 1] = '\0';
        }
        // Send the message
        if (sendMessage(messageToSend) == -1){

            fprintf(stderr, "Error sending message\n");
            break;
        }

        printf("Message sent: %s\n", messageToSend);
        // Receive a message
        int bytesReceived = receiveMessage(receivedMessage);
        if (bytesReceived == -1){
            fprintf(stderr, "Error receiving message\n");
            break;
        }

        // Print the received message
        printf("Received message: %s\n", receivedMessage);

    }
    */
    closeNetwork();

    return 0;
}