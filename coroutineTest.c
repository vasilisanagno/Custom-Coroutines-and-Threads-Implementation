#include "coroutines.h"

#define N 1000000

co_t t1, t2, temp;
int i = 0;

void dec () {
	int count;
	//int i = 0;

	printf("Welcome in dec\n");
	for (count = 0; count < N; count ++) {
		i --;
		printf("Dec: %d\n", i); fflush(stdout);
        //temp = t2;
        mycoroutines_switchto(&t2, &t1);
        //swapcontext(&t2.context, &t1.context);
	}

    mycoroutines_switchto(&t2, &main_coroutine);
	printf("Dec: %d\n", i); fflush(stdout);
}

void inc () {
	int count;
	//int i = 0;

	printf("Welcome in inc\n");
	for (count = 0; count < N; count ++) {
		i ++;
		printf("Inc: %d\n", i); fflush(stdout);
        ///temp = t1;
        mycoroutines_switchto(&t1, &t2);
        //swapcontext(&t1.context, &t2.context);
	}

    mycoroutines_switchto(&t1, &t2);
	printf("Inc: %d\n", i); fflush(stdout);
}

int main (int argc, char *argv[]) {
	mycoroutines_init(&main_coroutine);

	mycoroutines_create(&t1, inc, NULL);

	mycoroutines_create(&t2, dec, NULL);

    //swapcontext(&main_coroutine.context, &t2.context);
    mycoroutines_switchto(&main_coroutine, &t1);
    printf("\nMain end!\n");
	mycoroutines_destroy(&main_coroutine);

	return 0;
}