
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <math.h>
#include <semaphore.h>
#include <sys/timerfd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

struct timeval tv;

#define MAX 1000
char location[MAX];
long long timestamp;
long buffer[1];
long long time_buffer[1];
sem_t mySem;

long prevLoc=0, prevTS=0;
long nextLoc=0, nextTS = 0;
long buttonTS = 0;


void* Thread0(void*);
void* Child_Thread(void*);


int main(void){
	pthread_t thrd0;
	pthread_create(&thrd0, NULL, &Thread0, NULL);
	
	int pfd = open("/tmp/N_pipe1", O_RDONLY);
	mkfifo("/tmp/N_pipe2", 0666);
	int pfd2 = open("tmp/N_pipe2", O_RDONLY);
	
	pthread_t thrd1;
	
	clock_t currTime;
	
	while (1) {

		// Read from N_Pipe1 and store the values in buffer sizeof(char)
		//Store the reading in location[0]
		while(read(pfd, buffer, sizeof(char)) <= 0) {}
		location[0] = buffer;
		//get time convert it to milliseconds
		currTime = clock();
		timestamp = (long long) currTime * 1000 / CLOCKS_PER_SEC;

		// pthread_create(&thrd1, NULL, &Child_Thread, NULL);
		// write(pfd2, )
	}

	// Join thread -- won't reach here anyways!


	pthread_exit(NULL);
	return 0;
}






void* Thread0(void* ptr) {
	// Read from N_Pipe2 and store the values in time_buffer sizeof(long)
	int pfd2 = open("tmp/N_pipe2", O_RDONLY);
	if(pfd2 == -1) { printf("Failed to open N_pipe2"); }

	pthread_exit(NULL);
}

/**	@brief
 * 	This function is going to interpolate the the position of the button press
 * 	event based on the previous and next GPS time and location information. It
 * 	is going to use the slope intercept form to interpolate the value.
 */
void* Child_Thread(void* ptr){


	nextLoc = (long)location[0];
	nextTS 	= (long)timestamp;


	// interpolate the location of the button press event based on the info
	// received through all the threads.
	long double deltaLoc 	= (long double)(nextLoc - prevLoc);
	long double deltaTS		= (long double)(nextTS - prevTS);
	long double slope 		= (long double)(deltaLoc/deltaTS);
	long double tmp 		= ((long double)slope)*((long double)(buttonTS - prevTS));
	long double y0 			= (long double)tmp + (long double)prevLoc;

	// Now print the information on the screen. We need to be careful in printing
	// the data. As we have 5 threads running concurrently and all of the them 
	// are trying to print on the screen at the same time, we need to restrict
	// the access so that while only one thread printing the info on the screen
	// the rest of the threads will wait for it to finish its printing job. We
	// need a semaphore for that.

	printf("\nInterpolation Results\n");
	printf("\n---------------------\n");
	printf("GPS Previous TS = %lf \t GPS Previous Location = %lf\n", \
			(long double)prevTS, (long double)prevLoc);
	printf("Button TS = %lf \t Button Location = %lf\n", \
			(long double)buttonTS, (long double)y0);
	printf("GPS Next TS = %lf \t GPS Next Location = %lf\n", \
			(long double)nextTS, (long double)nextLoc);
	printf("\n\n");


	pthread_exit(NULL);
}
