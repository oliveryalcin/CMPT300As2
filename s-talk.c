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

/* Free char* msg Function*/
static void free_msg(void* pItem)
{
    free(pItem);
    return;
}

/* Dummy free*/
static void dummy_free(void* pItem)
{
    return;
}


// synchronization for shut down
static pthread_mutex_t shutdownMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t shutdownCondVar = PTHREAD_COND_INITIALIZER;

void signalShutdown(){
    pthread_mutex_lock(&shutdownMutex);
    {
        pthread_cond_signal(&shutdownCondVar);
    }
    pthread_mutex_unlock(&shutdownMutex);
    return;
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
    //char* rem_name = args[2];       // remote machine name
    int rem_port = atoi(args[3]);   // remote port number

    // check input arguments:
    if ( (my_port == 0) ){  // || (my_port < 1024) || (my_port > 49151)
        printf("Error: s-talk invalid request: invalid local port number (%s)\n", args[1]);
        //printf("    Port numbers must be integers in the range 1024 to 49151.\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }
    if ( (rem_port == 0) ){ // || (rem_port < 1024) || (rem_port > 49151)
        printf("Error: s-talk invalid request: invalid remote port number (%s)\n", args[3]);
        //printf("    Port numbers must be integers in the range 1024 to 49151.\n");
        printf("    command format should be: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }

    //printf("successful arguments supplied!\n");
    //printf("s-talk %d %s %d\n", my_port, rem_name, rem_port);

    // setup the threads, sockets, synchronization, lists, etc.

    // Create two lists:
    List* keyTXlist = List_create();    
    List* screenRXlist = List_create();

    // Create shared mutexes:
    pthread_mutex_t keyTXlistMutex = PTHREAD_MUTEX_INITIALIZER;     // mutex to protect concurrent access to keyTXlist
    pthread_mutex_t screenRXlistMutex = PTHREAD_MUTEX_INITIALIZER;  // mutex to protect concurrent access to screenRXlist

    // Shared Condition Variables:
    //pthread_cond_t keyTXlistCondVar = PTHREAD_COND_INITIALIZER;
    //pthread_cond_t screenRXlistCondVar = PTHREAD_COND_INITIALIZER;

    // Startup the four different threads (note initNetwork starts two modules/threads)
    if (Keyboard_init(keyTXlist, &keyTXlistMutex) < 0) { return -1; }
    if (Screen_init(screenRXlist, &screenRXlistMutex, args[2]) < 0) { return -1; }
    if (initNetwork(args[1],args[3],args[2], keyTXlist, screenRXlist, &keyTXlistMutex, &screenRXlistMutex) < 0) { return -1; }

    //printf("Main Thread waiting here...\n");
    // Main Thread will wait here until shutdown is signalled by RX or TX thread
    pthread_mutex_lock(&shutdownMutex);
    {
        pthread_cond_wait(&shutdownCondVar, &shutdownMutex);
    }
    pthread_mutex_unlock(&shutdownMutex);

    //printf("SHUTTING DOWN!\n");
    // Shutdown my modules
    if (Keyboard_shutdown() < 0) { return -1; }
    if (Screen_shutdown() < 0) { return -1; }
    if (Network_shutdown() < 0) { return -1; }

    //printf("Freeing lists...\n");
    List_free(keyTXlist, free_msg);
    List_free(screenRXlist, dummy_free);

    // dereference pointers
    //rem_name = NULL;
    keyTXlist = NULL;
    screenRXlist = NULL;

    printf("s-talk exited\n");
    return 0;
}