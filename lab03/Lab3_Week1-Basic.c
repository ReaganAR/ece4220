<<<<<<< HEAD
<<<<<<< HEAD
/*  Important Note: 

 *      1. Compile this file using the command: gcc Lab3_W1.c -lwiringPi -lpthread -o Lab3_W1
 *      2. Run the program as a SUDO user: sudo ./Lab3_W1
 *  
 */

// http://wiringpi.com/reference/core-functions/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <time.h>			
#include <semaphore.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>



#define DIR_A_PIN 5
#define DIR_B_PIN 4
#define PEDES_PIN 2 
#define BUTTON 16

#define OVERRIDE 20

// Scheduler thread which should turn on one light, then the other, then check to see if pedestrian needs to cross
void* SchedulerTask(void* ptr) {
    while(!digitalRead(OVERRIDE)) {
        // Basic vehicle timing
        digitalWrite(DIR_A_PIN, HIGH);
        sleep(2);
        digitalWrite(DIR_A_PIN, LOW);
        digitalWrite(DIR_B_PIN, HIGH);
        sleep(2);
        digitalWrite(DIR_B_PIN, LOW);
        
        //Pedestrian control
        if(digitalRead(BUTTON)) {
            digitalWrite(PEDES_PIN, HIGH);
            sleep(2);
            digitalWrite(PEDES_PIN, LOW);
        }
    }
    pthread_exit(0);
}

// Main thread
int main(void) {
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

    // Run scheduler thread
    pthread_t thrd;
    pthread_create(&thrd, NULL, &SchedulerTask, NULL);
    pthread_join(thrd, NULL);

    return 0;
}
=======


/*  Important Note: 

 *      1. Compile this file using the command: gcc Lab3_W1.c -lwiringPi -lpthread -o Lab3_W1
 *      2. Run the program as a SUDO user: sudo ./Lab3_W1
 *  
 */

// http://wiringpi.com/reference/core-functions/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <time.h>			
#include <semaphore.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>




int main(void){


    // Step-1
    // ------
    
    // Set-up of GPIO pins using WiringPi library




    // Step-2
    // ------
	// Run a infinite while loop and turn on/off one light at a time.
    





	return 0;
}
>>>>>>> 28cea9d (Import from previous semester)
=======
/*  Important Note: 

 *      1. Compile this file using the command: gcc Lab3_W1.c -lwiringPi -lpthread -o Lab3_W1
 *      2. Run the program as a SUDO user: sudo ./Lab3_W1
 *  
 */

// http://wiringpi.com/reference/core-functions/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <time.h>			
#include <semaphore.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>



#define DIR_A_PIN 5
#define DIR_B_PIN 4
#define PEDES_PIN 2 
#define BUTTON 16

#define OVERRIDE 20

// Scheduler thread which should turn on one light, then the other, then check to see if pedestrian needs to cross
void* SchedulerTask(void* ptr) {
    while(!digitalRead(OVERRIDE)) {
        // Basic vehicle timing
        digitalWrite(DIR_A_PIN, HIGH);
        sleep(2);
        digitalWrite(DIR_A_PIN, LOW);
        digitalWrite(DIR_B_PIN, HIGH);
        sleep(2);
        digitalWrite(DIR_B_PIN, LOW);
        
        //Pedestrian control
        if(digitalRead(BUTTON)) {
            digitalWrite(PEDES_PIN, HIGH);
            sleep(2);
            digitalWrite(PEDES_PIN, LOW);
        }
    }
    pthread_exit(0);
}

// Main thread
int main(void) {
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

    // Run scheduler thread
    pthread_t thrd;
    pthread_create(&thrd, NULL, &SchedulerTask, NULL);
    pthread_join(thrd, NULL);

    return 0;
}
>>>>>>> f3cdaaa (Import from previous semester)
