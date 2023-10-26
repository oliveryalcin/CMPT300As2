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
static char* msg;
//static char* dynamicMessage;

/* read_stdin : reads input from stdin (keyboard/terminal)*/
void* read_stdin(void* unused){
    // TODO:
    char buffer[1024];      // Buffer to store user input
    ssize_t bytes_read;     // number of bytes read
    size_t nbytes;          // number of bytes to attempt to read
    nbytes = sizeof(buffer);

    while (1) {
        memset(buffer, 0, nbytes);
        bytes_read = read(STDIN_FILENO, buffer, nbytes);

        if (bytes_read == -1) {
            perror("Error reading from terminal");
            //return 1;
        }

        // Check if the user pressed Enter (newline character)
        if (bytes_read > 0 && buffer[bytes_read - 1] == '\n') {
            // Remove the newline character
            bytes_read--;
            buffer[bytes_read] = '\0';

            pthread_mutex_lock(sharedlistMutex);
            {
            msg = malloc(nbytes);
            memcpy(msg, buffer, nbytes);
            List_prepend(kList, msg);
            }
            pthread_mutex_unlock(sharedlistMutex);
        }
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
    printf("Keyboard Shutdown...\n");

    if(pthread_cancel(tKeyboard) != 0){     // cancel thread
        printf("Error cancelling keyboard\n");
    }      
    printf("    Keyboard thread cancelled\n");

    if (pthread_join(tKeyboard, NULL) != 0){ // waits for thread to finish
        printf("Error joining keyboard\n");
    }
    printf("    Keyboard thread joined\n");

    // cleanup memory
    /*if (msg){
        free(msg);
        msg = NULL;
    }*/    
}


