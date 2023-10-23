#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_
#include <pthread.h>
#include "list.h"

int Screen_init(List* slist, pthread_mutex_t* screenRXlistMutex);
void Screen_shutdown();
void* write_stdout(void *unused);

#endif