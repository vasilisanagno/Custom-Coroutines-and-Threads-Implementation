#ifndef _COROUTINES_H_
#define _COROUTINES_H_

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>

typedef struct {
    ucontext_t context;
    void (*body)(void*);
    void *arg;
    int active;
} co_t;

extern co_t *running;

extern int mycoroutines_init (co_t *main); 
extern int mycoroutines_create(co_t *co, void (*body)(void*), void *arg); 
extern int mycoroutines_switchto(co_t *old, co_t *new); 
extern int mycoroutines_destroy (co_t *co); 

#endif