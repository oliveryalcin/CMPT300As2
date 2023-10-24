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

//unsure if loop is necessary here since there is no Keyboard I/O waits occuring? 
void* write_stdout(void *unused){

    while(1){

        char* rxMessage = NULL;
        pthread_mutex_lock(sharedlistMutex);
        {
            rxMessage = List_remove(rxList); //pop item from list and update list
        }
        pthread_mutex_unlock(sharedlistMutex);

        if(rxMessage != NULL){
            ssize_t bytes_read = write(1, rxMessage, strlen(rxMessage));
            if (bytes_read == -1) {
                perror("Error reading from LIST ADT, either write failed and/or List is empty");
            } 
        }
    }

    return NULL;
}
    

int Screen_init(List *sList, pthread_mutex_t *screenRXlistMutex){ // Thread initializer in order to output messages
    rxList = sList;
    sharedlistMutex = screenRXlistMutex;

    if (pthread_create(&tScreen, NULL, write_stdout, NULL) != 0){
        perror("Error creating screen/output thread\n");
        return -1;
    }

    return 1; //keeping same structure as you
}

void Screen_shutdown(){
    //pthread_cancel(tScreen);      // cancel thread
    pthread_join(tScreen, NULL);  // waits for thread to finish

}



