// Consumer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include "list.h"
#include "screen.h"
#include "string.h"

static pthread_t tScreen;    // Screen thread (consumes input from network)
static List* rxList;
static pthread_mutex_t* sharedlistMutex;
static char* hostName;
char* rxMessage;

/* write_stdout - function that screen thread will be running */
void* write_stdout(void *unused){
    while(1){
        rxMessage = NULL;
        pthread_mutex_lock(sharedlistMutex);
        {
            rxMessage = List_remove(rxList); //pop item from list and update list
        }
        pthread_mutex_unlock(sharedlistMutex);

        if(rxMessage != NULL){
            write(1, hostName, strlen(hostName));
            write(1, ": ", 2);
            ssize_t bytes_read = write(1, rxMessage, strlen(rxMessage));
            if (bytes_read == -1) {
                perror("Write Failed");
            } 
            write(1, "\n", 1);
        }
        pthread_testcancel();
    }

    return NULL;
}
    

/* Screen_init */
int Screen_init(List *sList, pthread_mutex_t *screenRXlistMutex, char* hostNameArg){
    rxList = sList;
    sharedlistMutex = screenRXlistMutex;
    hostName = hostNameArg;
    if (pthread_create(&tScreen, NULL, write_stdout, NULL) != 0){
        perror("Error creating screen/output thread\n");
        return -1;
    }

    return 1;
}


/* Screen_shutdown */
int Screen_shutdown(void){
    //printf("Screen Shutdown...\n");

    if (pthread_cancel(tScreen) != 0) {       // cancel thread
        perror("Error cancelling screen\n");
        return -1;
    }
    //printf("    Screen thread cancelled\n");

    if (pthread_join(tScreen, NULL) != 0){  // waits for thread to finish
        perror("Error joining screen\n");
        return -1;
    } 
    //printf("    Screen thread joined\n");

    // dereference pointers
    rxList = NULL;
    sharedlistMutex = NULL;
    hostName = NULL;
    rxMessage = NULL;

    return 1;
}



