#define _XOPEN_SOURCE 700

#ifndef _THREADS_H
#define _THREADS_H_

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <stdbool.h>
#include "coroutines.h"

typedef struct {
    int value;
    int id;
    bool exists;
} sem_t;

typedef struct thread {
	int id;
    bool exist;
    bool terminated;
	co_t coroutine;
    struct thread *joined;
    int blocked_sem_id;
} thr_t;

typedef struct node {
	thr_t *thread;
	struct node *next;
	struct node *prev;
} node_t;

typedef struct {
	int size;
	node_t *head;
	node_t *tail;
} list_t;

extern struct sigaction action;
extern struct itimerval timer, resetTimer;
extern list_t *ready, *blocked;
extern thr_t main_thread;
extern thr_t *running_thread;
extern thr_t *scheduler_thread;
extern int idFlag, semIdFlag;

extern bool thread_terminate;

// Scheduler functions.
extern void alarm_handler (int sig);
extern void timer_set (struct itimerval *t, int timeout);
extern void scheduler();

// List processing.
extern void add_blocked_queue (thr_t *thread);
extern void add_ready_queue (thr_t *thread);
extern int remove_blocked_queue (char selection, thr_t *threadJoined, int semId);
extern thr_t *remove_ready_queue ();

// Thread functions.
extern int mythreads_init();
extern int mythreads_create(thr_t *thr, void (body)(void *), void *arg);
extern int mythreads_yield();
extern int mythreads_join(thr_t *thr);
extern int mythreads_destroy(thr_t *thr);
extern int mythreads_sem_create(sem_t *s, int val);
extern int mythreads_sem_down(sem_t *s);
extern int mythreads_sem_up(sem_t *s); 
extern int mythreads_sem_destroy(sem_t *s);

#endif