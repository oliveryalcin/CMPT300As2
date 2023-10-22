#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_
#include <pthread.h>
#include "list.h"

int Keyboard_init(List* klist, pthread_mutex_t* keyTXlistMutex);
void Keyboard_shutdown();
void* read_stdin(void* unused);

#endif
