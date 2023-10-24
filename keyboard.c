// Producer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

#include <fcntl.h>
#include <unistd.h>

#include "keyboard.h"
#include "list.h"

#define DYNAMIC_LEN 128
#define MSG_MAX_LEN 1024

//static pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t tKeyboard;    // Keyboard thread (monitors input from keyboard)
static List* kList;
static pthread_mutex_t* sharedlistMutex;
//static char* dynamicMessage;

/* read_stdin : reads input from stdin (keyboard/terminal)*/
void* read_stdin(void* unused){
    // TODO:
    char buffer[1024];      // Buffer to store user input


    while (1) {
        fgets(buffer, sizeof(buffer), stdin);

        pthread_mutex_lock(sharedlistMutex);
        {
                // critical section/ access shared list
                // create (dynamic) item "msg"
            List_prepend(kList, buffer);
        }
            pthread_mutex_unlock(sharedlistMutex);
         
    }
    return NULL;
}

/* Keyboard_init: initialize thread */
int Keyboard_init(List* klist, pthread_mutex_t* keyTXlistMutex){
    kList = klist;
    sharedlistMutex = keyTXlistMutex;
    //dynamicMessage = malloc(DYNAMIC_LEN);
    if (pthread_create(&tKeyboard, NULL, read_stdin, NULL) != 0){
        perror("Error creating keyboard thread\n");
        return -1;
    }
    return 1;
}



/* Keyboard_shutdown: shutdown thread */
void Keyboard_shutdown(){
    //pthread_cancel(tKeyboard);      // cancel thread
    pthread_join(tKeyboard, NULL);  // waits for thread to finish

    // cleanup memory
    //pthread_mutex_lock(&myMutex);
    {
        //free(dynamicMessage);
    }
    //pthread_mutex_unlock(&myMytex);
    
}


