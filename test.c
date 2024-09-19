#include "threads.h"

#define N 1000000

thr_t t1, t2;
sem_t s1, s2;
int i = 0;

void dec () {
	int count;

	printf("Welcome in dec\n");
	sleep(1);
	for (count = 0; count < N; count ++) {
		i --;
		// printf("Dec: %d\n", i); fflush(stdout);
	}

	printf("Dec: %d, Thread function: %d\n", i, running_thread->id); fflush(stdout);
    mythreads_sem_up(&s1);
}

void inc () {
	int count;

	printf("Welcome in inc\n");

	sleep(1);
	for (count = 0; count < N; count ++) {
		i ++;
		// printf("Inc: %d\n", i); fflush(stdout);
	}

	printf("Inc: %d, Thread function: %d\n", i, running_thread->id); fflush(stdout);
    mythreads_sem_up(&s2);
}

int main (int argc, char *argv[]) {
	mythreads_init();

    mythreads_sem_create(&s1,0);
    mythreads_sem_create(&s2,0);
	mythreads_create(&t1, inc, NULL);

	mythreads_create(&t2, dec, NULL);
    
    mythreads_join(&t1);
    mythreads_join(&t2);
    // mythreads_sem_down(&s1);
    // mythreads_sem_down(&s2);

	mythreads_destroy(&t1);
    mythreads_destroy(&t2);
    mythreads_destroy(&main_thread);

    printf("\nMain end!\n");
    
	return 0;
}