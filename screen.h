#ifndef _SCREEN_H_
#define _SCREEN_H_
#include <pthread.h>
#include "list.h"

int Screen_init(List *sList, pthread_mutex_t *screenRXlistMutex, char* hostNameArg);
int Screen_shutdown(void);
void* write_stdout(void *unused);

#endif