#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/timerfd.h>
#include <inttypes.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>

#include <wiringPi.h>

#define DIR_A_PIN 5
#define DIR_B_PIN 4
#define PEDES_PIN 2 
#define BUTTON 16

#define OVERRIDE 20

#define PRIOR_A 20
#define PRIOR_B 20
#define PRIOR_P 80

sem_t mutex;

// Direction A thread
void* DirectionA(void* ptr) {
    while(TRUE) {
        sem_wait(&mutex);
        digitalWrite(DIR_A_PIN, HIGH);
        sleep(2);
        digitalWrite(DIR_A_PIN, LOW);
        sem_post(&mutex);
        usleep(1000);
    }
    pthread_exit(0);
}

// Direction B thread
void* DirectionB(void* ptr) {
    while(TRUE) {
        sem_wait(&mutex);
        digitalWrite(DIR_B_PIN, HIGH);
        sleep(2);
        digitalWrite(DIR_B_PIN, LOW);
        sem_post(&mutex);
        usleep(1000);
    }
    pthread_exit(0);
}

// Pedestrian Crossing thread
void* Pedestrian(void* ptr) {
    while(TRUE) {
        if(digitalRead(BUTTON) == HIGH){
            sem_wait(&mutex);
            digitalWrite(PEDES_PIN, HIGH);
            sleep(2);
            digitalWrite(PEDES_PIN, LOW);
            sem_post(&mutex);
            usleep(1000);
        }
    }
    pthread_exit(0);
}

// Main thread
int main(void) {
    pthread_attr_t attrA, attrB, attrPed; // 1,2,Pedestrian
    pthread_t dirA, dirB, ped;

    // Perform WiringPi setup functions, including setting up required pins
    wiringPiSetupGpio();
    pinMode(DIR_A_PIN, OUTPUT);
    pinMode(DIR_B_PIN, OUTPUT);
    pinMode(PEDES_PIN, OUTPUT);

    // Clear all LEDs to off
    pinMode(3, OUTPUT);
    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(2, LOW);

    // Set up button and override button so it loops
    pinMode(BUTTON, INPUT);
    pullUpDnControl(BUTTON, PUD_DOWN);
    pinMode(OVERRIDE, INPUT);
    pullUpDnControl(OVERRIDE, PUD_DOWN);

    // Set priorities
    struct sched_param param;
    pthread_attr_init(&attrA);
    // pthread_attr_getschedparam(&attrA, &param);
    pthread_attr_setschedpolicy(&attrA, SCHED_FIFO);
    param.sched_priority = PRIOR_A;
    pthread_attr_setschedparam(&attrA, &param);

    struct sched_param param2;
    pthread_attr_init(&attrB);
    // pthread_attr_getschedparam(&attrB, &param2);
    pthread_attr_setschedpolicy(&attrB, SCHED_FIFO);
    param2.sched_priority = PRIOR_B;
    pthread_attr_setschedparam(&attrB, &param2);

    struct sched_param paramPed;
    pthread_attr_init(&attrPed);
    pthread_attr_setschedpolicy(&attrPed, SCHED_FIFO);
    paramPed.sched_priority = PRIOR_P;
    pthread_attr_setschedparam(&attrPed, &paramPed);

    // Release semaphore
    sem_init(&mutex, 0, 1);

    // Run scheduler threads
    if( pthread_create(&dirA, &attrA, &DirectionA, NULL) != 0 ||
        pthread_create(&dirB, &attrB, &DirectionB, NULL) != 0 ||
        pthread_create(&ped, &attrPed, &Pedestrian, NULL)!= 0){
            printf("Thread creation error");
            return 0;
    }

    // Threads continue running until OVERRIDE button pressed
    while(!digitalRead(OVERRIDE)) {
        sleep(1);
    }

    pthread_cancel(dirA);
    pthread_cancel(dirB);
    pthread_cancel(ped);

    pthread_join(dirA, NULL);
    pthread_join(dirB, NULL);
    pthread_join(ped, NULL);

    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    digitalWrite(2, LOW);

    sem_destroy(&mutex);
    
    return 0;
}
