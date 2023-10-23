// Consumer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include "screen.h"
#include "string.h"

static pthread_t tScreen;    // Screen thread (consumes input from network)
static List* rxList;

//unsure if loop is necessary here? 
void write_stdout(void *unused){
    char* rxMessage = rxList->pCurrentNode->pItem;
    ssize_t bytes_read = write(STDOUT_FILENO, rxMessage, strlen(rxMessage));
    if (bytes_read == -1) {
        perror("Error reading from LIST ADT");
    } 
}
    

int Screen_init(List *sList, pthread_mutex_t *screenRXlistMutex){ // Thread initializer in order to output messages
    rxList = sList;

    if (pthread_create(&tScreen, NULL, write_stdout, NULL) != 0){
        perror("Error creating screen/output thread\n");
        return -1;
    }

    return 1; //keeping same structure as you
}

void Screen_shutdown(){
    pthread_cancel(tScreen);      // cancel thread
    pthread_join(tScreen, NULL);  // waits for thread to finish

}



