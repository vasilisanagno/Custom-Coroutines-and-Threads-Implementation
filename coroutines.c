#include "coroutines.h"

#define STACK_SIZE 16384

co_t *running;

// Initialize coroutine environment.
int mycoroutines_init(co_t *main) {
    getcontext(&main->context);
    if ((main->context.uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
        main->context.uc_stack.ss_size = STACK_SIZE;
    }
    else {
        perror("Not enough storage for stack");
        abort();
    }

    main->active = 1;

    running = main;

    return 0;
}

// Create coroutine.
int mycoroutines_create(co_t *co, void (*body)(void*), void *arg) {
    getcontext(&co->context);

    if ((co->context.uc_stack.ss_sp = (char *) malloc(STACK_SIZE)) != NULL) {
        co->context.uc_stack.ss_size = STACK_SIZE;

        errno = 0;
        makecontext(&co->context, (void (*)(void))body, 1, arg);

        if(errno != 0){
            perror("Error reported by makecontext()");
            return -1;
        }

        co->body = body;
        co->arg = arg;
        co->active = 0;
    }
    else {
        perror("not enough storage for stack");
        abort();
    }
    return 0;
}

// Switch coroutines.
int mycoroutines_switchto(co_t *old, co_t *new) {
    if (new->active) {
        return -1;
    }

    // Change the running coroutine.
    old->active = 0;
    new->active = 1;

    swapcontext(&old->context, &new->context);

    return 0;
}

// Destroy coroutine.
int mycoroutines_destroy(co_t *co) {
    free(co->context.uc_stack.ss_sp);
    return 0;
}