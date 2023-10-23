#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_
#include <pthread.h>
#include "list.h"

int Screen_init(List* klist, pthread_mutex_t* keyTXlistMutex);
void Screen_shutdown();
void* write_stdout(void* unused);

#endif