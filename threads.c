#include "threads.h"
#define _GNU_SOURCE

struct sigaction action = {{0}};
struct itimerval timer = {{0}}, resetTimer = {{0}};
list_t *ready, *blocked;
thr_t main_thread;
thr_t *running_thread = NULL, *scheduler_thread;
int idFlag, semIdFlag;

bool thread_terminate = false;

// Scheduler for threads.
// Round robin scheduling.
// Scheduler runs in a different thread.
void scheduler() {
	getcontext(&scheduler_thread->coroutine.context);

    // Stop the timer.
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    printf("Scheduler\n");

    // If a thread has terminated.
	if (thread_terminate) {
        printf("Thread terminated: %d\n", running_thread->id);

		remove_blocked_queue('j', running_thread, 0);
        
        running_thread->exist = false;
        running_thread = NULL;
	}
    else {
        running_thread = scheduler_thread;
    }

    // If no threads are in ready queue.
	if (ready->size == 0) {
        // If main program has terminated terminate the program.
        if(main_thread.terminated) {
            printf("\nProgram terminate\n");
            mycoroutines_destroy(&scheduler_thread->coroutine);
            return;
        }

        // Run the scheduler thread in a loop.
		running_thread = scheduler_thread;
		
        setcontext(&running_thread->coroutine.context);
		timer_set(&timer, 150000);
		return;
	}

    // Get the first thread from the running queue.
	running_thread = remove_ready_queue();
    printf("Thread to run: %d\n", running_thread->id);

	// Restore the saved context of the new running coroutine
	thread_terminate = true;
    scheduler_thread->coroutine.active = 0;

    // Reset the timer.
	timer_set(&timer, 150000);

    setcontext(&running_thread->coroutine.context);
}

// Set the alarm handler.
void alarm_handler (int sig) {
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    printf("\nAlarm\n");

    // Call scheduler.
	thread_terminate = false;
    add_ready_queue(running_thread);
    mycoroutines_switchto(&running_thread->coroutine, &scheduler_thread->coroutine);
}

// Set the timer.
void timer_set (struct itimerval *t, int timeout) {
	t->it_value.tv_sec = 0;
	t->it_value.tv_usec = timeout;
	t->it_interval.tv_sec = 0;
	t->it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, t, NULL);
}

// Add thread into blocked queue.
void add_blocked_queue (thr_t *thread) {
	node_t *node;

	// Make the list node.
	node = (node_t *) calloc(1, sizeof(node_t));

	node->thread = thread;
	node->next = node->prev = NULL;
	
	// If queue is empty.
	if (blocked->size == 0) {
		blocked->head = blocked->tail = node;
		blocked->size ++;

		return;
	}

    // Remove the first thread from the list.
	node->prev = blocked->tail;
	blocked->tail->next = node;
	blocked->tail = node;

	blocked->size ++;
    printf("\nBlocked queue size: %d\n", blocked->size);
}

// Add thread into ready queue.
void add_ready_queue (thr_t *thread) {
	node_t *node;
	
	// Make the list node.
	node = (node_t *) calloc(1, sizeof(node_t));

	node->thread = thread;
	node->next = node->prev = NULL;

	// If queue is empty.
	if (ready->size == 0) {
		ready->head = ready->tail = node;
		ready->size ++;

		return;
	}

    // Remove the first thread from the list.
	node->prev = ready->tail;
	ready->tail->next = node;
	ready->tail = node;

	ready->size ++;
    printf("\nReady queue size: %d\n", ready->size);
}

// Remove thread from the blocked queue.
// 2 cases below: 
// 1: Blocked by join (selection 'j').
// 2. Blocked by semaphore down (selection 's').
int remove_blocked_queue (char selection, thr_t *threadJoined, int semId) {
	node_t *node = NULL;
	
    switch (selection) {
        // Case blocked by join.
        case 'j': {
            for (node = blocked->head; node != NULL; node = node->next) {
                // Find thread and remove from blocked list.
                if (node->thread->joined == threadJoined) {
                    if (node == blocked->head)
                        blocked->head = blocked->head->next;
                    if (node == blocked->tail)
                        blocked->tail = blocked->tail->prev;
                    if (node->prev != NULL)    
                        node->prev->next = node->next;
                    if (node->next != NULL)    
                        node->next->prev = node->prev;

                    blocked->size --;
                    printf("\nBlocked queue size: %d\n", blocked->size);

                    node->thread->joined = NULL;

                    // Add the thread in ready queue.
                    add_ready_queue(node->thread);
                    break;
                }
            }

            break;
        }
        // Case blocked by semaphore down.
        case 's': {
            for (node = blocked->head; node != NULL; node = node->next) {
                // Find thread and remove from blocked list.
                if (node->thread->blocked_sem_id == semId) {
                    if (node == blocked->head)
                        blocked->head = blocked->head->next;
                    if (node == blocked->tail)
                        blocked->tail = blocked->tail->prev;
                    if (node->prev != NULL)    
                        node->prev->next = node->next;
                    if (node->next != NULL)    
                        node->next->prev = node->prev;

                    blocked->size --;
                    printf("\nBlocked queue size: %d\n", blocked->size);

                    node->thread->blocked_sem_id = -1;

                    // Add the thread in ready queue.
                    add_ready_queue(node->thread);
                    break;
                }
            }

            break;
        }
    }

    if(node!=NULL) {
        return (1);
    }

    return (0);
}

// Remove thread from the ready queue.
thr_t *remove_ready_queue () {
	node_t *node;

	if (ready->head == NULL) {
		return (NULL);
	}

	// If blocked queue will be empty.
	if (ready->size == 1) {
		node = ready->head;

		ready->head = ready->tail = NULL;
		ready->size --;

		return (node->thread);
	}

	// Remove from the queue.
	if (ready->head->next != NULL)
		ready->head->next->prev = NULL;
	node = ready->head;
	
	ready->head = ready->head->next;
	ready->size --;
    printf("\nReady queue size: %d\n", blocked->size);

	return (node->thread);
}

// Initialize environment.
int mythreads_init () {
	if (main_thread.exist == true) {
		return (-1);
	}

    printf("\nInit\n");

	idFlag = 0;
    semIdFlag = 0;

	// Make the handler for the SIGALRM.
	action.sa_handler = alarm_handler;
	sigaction (SIGALRM, &action, NULL);

	// Make the two lists for threads.
	ready = (list_t *) calloc(1, sizeof(list_t));
	blocked = (list_t *) calloc(1, sizeof(list_t));

    // Initialize thread lists.
	ready->head = ready->tail = NULL;
	ready->size = 0;

	blocked->head = blocked->tail = NULL;
	blocked->size = 0;

	scheduler_thread = (thr_t *) calloc(1, sizeof(thr_t));

	// Initialize main thread.
	mycoroutines_init (&main_thread.coroutine);
	main_thread.id = idFlag;
    main_thread.blocked_sem_id = -1;

	main_thread.exist = true;
	main_thread.terminated = false;
	
	idFlag ++;

	// Make and set the timer.
	timer_set(&timer, 150000);

    // Make the scheduler coroutine and swap.
	thread_terminate = false;

	mycoroutines_create(&scheduler_thread->coroutine, scheduler, NULL);

    scheduler_thread->id = 1000000;

    // Swap from main to scheduler.
	add_ready_queue(&main_thread);
    mycoroutines_switchto(&main_thread.coroutine, &scheduler_thread->coroutine);
	
	return (0);
}

// Create a thread.
int mythreads_create (thr_t *thr, void (body)(void *), void *arg) {
	int res;

    // Stop the timer.
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    // If the thread already exists.
	if (thr->exist == true) {
		return (-1);
	}

    // Initialize thread info.
	thr->id = idFlag;
	idFlag ++;
    thr->blocked_sem_id = -1;

    printf("\nThread created: %d\n", thr->id);

    // When any thread terminates go to scheduler.
	thr->coroutine.context.uc_link = &scheduler_thread->coroutine.context;
	res = mycoroutines_create (&thr->coroutine, body, arg);
	if (res == -1) {
		return (-1);
	}

    // Add the thread in ready queue.
	add_ready_queue(thr);
	thr->exist = true;
	thr->terminated = false;

	timer_set(&timer, 150000);

	return (0);
}

// Switch intentionally.
int mythreads_yield () {
	thread_terminate = false;
    
    mycoroutines_switchto(&running_thread->coroutine, &scheduler_thread->coroutine);

	return (0);
}

// Wait for a thread to terminate.
int mythreads_join(thr_t *thr) {
    // Stop the timer.
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    printf("\nThread %d waits %d\n", running_thread->id, thr->id);

    // If thread doesn't exist.
	if (thr->exist == false) {
		return (-1);
	}

    // Make the running thread.
	running_thread->joined = thr;

    // Add in blocked queue.
	add_blocked_queue(running_thread);

    mythreads_yield();

	return (0);
}

// Destroy a thread.
int mythreads_destroy(thr_t *thr) {
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    printf("\nDestroy %d\n", thr->id);

	node_t *node;
    bool foundReady = false;

    // Remove the thread from the ready or blocked queue.
	if (running_thread->id != thr->id){
		for (node = ready->head; node != NULL; node = node->next) {
			if (node->thread->id == thr->id) {
                foundReady = true;

				if (node == ready->head)
					ready->head = ready->head->next;
                if (node == ready->tail)
                    ready->tail = ready->tail->prev;
				if (node->prev != NULL)    
					node->prev->next = node->next;
				if (node->next != NULL)    
					node->next->prev = node->prev;

				free(node);
				ready->size --;

				break;
			}
		}

        if (!foundReady) {
            for (node = blocked->head; node != NULL; node = node->next) {
                if (node->thread->id == thr->id) {
                    if (node == blocked->head)
                        blocked->head = blocked->head->next;
                    if (node == blocked->tail)
                        blocked->tail = blocked->tail->prev;
                    if (node->prev != NULL)    
                        node->prev->next = node->next;
                    if (node->next != NULL)    
                        node->next->prev = node->prev;

                    free(node);
                    blocked->size --;
                    break;
                }
            }
        }
	}
    
    // Destroy the thread coroutine.
	mycoroutines_destroy(&thr->coroutine);

    // If thread is the running one.
    if (running_thread->id == thr->id) {
        running_thread = NULL;
    }
    else {
        running_thread->coroutine.active = 0;
    }

	thread_terminate = false;
    thr->exist = false;
    thr->terminated = true;
    
    // Swap to scheduler.
    thr->coroutine.active = 0;
	if (running_thread == NULL) {    
        setcontext(&scheduler_thread->coroutine.context);
    }

	return (0);
}

// Create semaphore.
int mythreads_sem_create(sem_t *s, int val) {
    // Stop the timer.
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    printf("\nSemaphore create\n");

    // If semaphore exists.
    if(s->exists) {
        return (-1);
    }

    // If value is not binary.
    if(val!=0 && val!=1) {
        return (-2);
    }

    // Initialize the semaphore info.
    s->exists = true;
    s->id = semIdFlag;
    semIdFlag++;

    s->value = val;
    
	timer_set(&timer, 150000);

    return (1);
}

// Down semaphore.
int mythreads_sem_down(sem_t *s) {
    // Stop the timer.
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    // if semaphore does not exist.
    if(s->exists == false) {
        return (-1);
    }

    // If the thread can not do down.
    if(s->value == 0) {
        // Block the thread.
        running_thread->blocked_sem_id = s->id;
        add_blocked_queue(running_thread);
        mythreads_yield();

        return (0);
    }

    s->value --;
    
	timer_set(&timer, 150000);

    return (1);
}

// Up semaphore.
int mythreads_sem_up(sem_t *s) {
    // Stop the timer.
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    // If semaphore does not exist.
    if(s->exists == false) {
        return (-1);
    }

    // If an up is lost.
    if(s->value == 1) {
        return (0);
    }

    // Wake up from a thread from blocked queue.
    if(remove_blocked_queue('s', NULL, s->id) == 1) {
        return (1);
    }

    s->value ++;
    
	timer_set(&timer, 150000);

    return (1);
}

// Destroy semaphore.
int mythreads_sem_destroy(sem_t *s) {
    // Stop the timer.
	setitimer(ITIMER_REAL, &resetTimer, NULL);

    // If the semaphore does not exist.
    if(s->exists == false) {
        return (-1);
    }

    s->exists = false;

	timer_set(&timer, 150000);

    return (1);
}