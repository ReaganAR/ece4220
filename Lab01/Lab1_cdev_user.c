/* Lab1_cdev_user.c
 * ECE4220/7220
 * Author: Luis Alberto Rivera
 * Modified by: Ramy Farag
 * Edited for Reading from Kernel Space
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define CHAR_DEV "/dev/Lab1" // "/dev/YourDevName"
#define MSG_SIZE 40

int main(void)
{
    int cdev_id, bytes_read;
    char buffer[MSG_SIZE];

    // Open the Character Device for reading
    if((cdev_id = open(CHAR_DEV, O_RDONLY)) == -1) {
        printf("Cannot open device %s\n", CHAR_DEV);
        exit(1);
    }

    while(1)
    {
        // Reading from the character device
        bytes_read = read(cdev_id, buffer, sizeof(buffer)); 
        if (bytes_read < 0) {
            printf("Read failed, leaving...\n");
            break;
        }

        // Null-terminate the buffer to safely print it as a string
        buffer[bytes_read] = '\0';

        printf("Read from device: %s\n", buffer);

    }

    close(cdev_id); // Close the device.
    return 0;
}
