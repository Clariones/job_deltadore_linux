/*
 * OS.cpp
 *
 *  Created on: Dec 18, 2016
 *      Author: clari
 */
#ifdef  _WIN32
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <winsock.h>
#include <unistd.h>
#endif

#ifdef  _WIN32
__PTW32_DECLSPEC int pthread_create
(pthread_t *, const pthread_attr_t *, void *(__PTW32_CDECL *)(void *), void *){
	return 0;
}

__PTW32_DECLSPEC int sem_init (sem_t * sem, int pshared, unsigned int value) {return 0;}
__PTW32_DECLSPEC int sem_wait (sem_t * sem) {return 0;}
__PTW32_DECLSPEC int sem_post (sem_t * sem) {return 0;}

int PASCAL select(int nfds,fd_set*,fd_set*,fd_set*,const struct timeval* tv) {
	unsigned long long timeMs = 0;
	timeMs = tv->tv_sec * 1000;
	timeMs = timeMs + tv->tv_usec/1000;
	sleep((timeMs+999)/1000);
	return 0;
}
__PTW32_DECLSPEC int sem_destroy (sem_t * sem) {return 0;}
__PTW32_DECLSPEC int pthread_join (pthread_t, void **){return 0;}
#endif
