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

#define FILTER_WIDTH 3
#define FILTER_HEIGHT 1

void print_matrix(int arr[100][100], int numRows, int numCols);
void resetOutput();

//Declare Variables to store the given matrix and final results
int inputMatrix[100][100];
int outputMatrix[100][100]; 
int filter[100][100];

//Declare structure to hold the convolution info (Like number of rows/cols)
int inputRows, inputColumns;
int filterHeight, filterWidth;

// 1. Single thread to evaluate the convolution 
void* ConvolutionPerMatrix(void *info){
	// Get the Convolution info
	// Loop through the matrix based on the info
    for(int i = 0; i < inputRows; i++){
        for(int j = 0; j < inputColumns; j++){
            outputMatrix[i][j] = inputMatrix[i][j-1] * filter[0][0] + inputMatrix[i][j] * filter[0][1] + inputMatrix[i][j+1] * filter[0][2];
        }
    }

	pthread_exit(0);
}

/* Thread function to process one row of the given matrix
 * info is an int*
 */
void* ConvolutionPerRow(void *info){
    int row = *(int *) info;
    for(int j = 0; j < inputColumns; j++){
            outputMatrix[row][j] = inputMatrix[row][j-1] * filter[0][0] + inputMatrix[row][j] * filter[0][1] + inputMatrix[row][j+1] * filter[0][2];
        }

    pthread_exit(0);
}

/* 3. Thread to process each element of the matrix.
 * info is an int[2]
 */
void* ConvolutionPerData(void *info){
    int row = ((int*) info)[0];
    int col = ((int*) info)[1];
    outputMatrix[row][col] = inputMatrix[row][col-1] * filter[0][0] + inputMatrix[row][col] * filter[0][1] + inputMatrix[row][col+1] * filter[0][2];
    pthread_exit(0);
}

// Main function
int main(int argc, char *argv[]) {
    // Define variables
    const char *filename = "20x10.txt";
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        printf("Failed to open file.\n");
        return -1;
    }

	// Step-1
    // ======
    // Read the file and load the data matrix and filter vector information
    // into a 2D and 1D array respectively.	
	// Scan in matrix size using fscanf
    fscanf(file, "%d %d", &inputRows, &inputColumns);

    // Loop to initialize the original matrix
    for(int i = 0; i < inputRows; i++){
        for(int j = 0; j < inputColumns; j++){
            fscanf(file, "%d", &inputMatrix[i][j]);
        }
    }

    // Scan in filter
    fscanf(file, "%d %d", &filterHeight, &filterWidth);

    for(int i = 0; i < filterHeight; i++){
        for(int j = 0; j < filterWidth; j++){
            fscanf(file, "%d", &filter[i][j]);
        }
    }

    // Print the original data 
    printf("Input Matrix:");
    print_matrix(inputMatrix, inputRows, inputColumns);
    printf("Input Filter:");
    print_matrix(filter, filterHeight, filterWidth);

	/* Convolution using a single thread */
    printf("**Case-1: Convolution using single thread for matrix * * * * * * *\n\n");
    pthread_t t1;

	// Start Timing
    clock_t startTime = clock();
    
    // Use pthread_create function to create pthreads
    // Use pthread_join the join the threads
    pthread_create(&t1, NULL, &ConvolutionPerMatrix, NULL);
    pthread_join(t1, NULL);

	// End timing
    printf("Time took: %li\n", clock() - startTime);
	// Print out search count with 1 thread and the time it took

    printf("Output Matrix:");
    print_matrix(outputMatrix, inputRows, inputColumns);

    // Reset the Matrix to check accuracy
    resetOutput();

	/* Convolution using one thread per row */
    printf("\n**Case-2: Convolution using single thread per row * * * * * * *\n");
    startTime = clock();

    pthread_t threads2[inputRows];
    int temp = 0; // Temporary fix: thread 0 is always being skipped
    pthread_create(&(threads2[0]), NULL, &ConvolutionPerRow, &temp);
    for(int i = 0; i < inputRows; i++) {
        int curr_row = i;
        pthread_create(&(threads2[i]), NULL, &ConvolutionPerRow, &curr_row);
    }

    for(int i = 0; i < inputRows; i++) {
        pthread_join(threads2[i], NULL);
    }

    printf("Time took: %li\n", clock() - startTime);
    printf("Output Matrix:");
    print_matrix(outputMatrix, inputRows, inputColumns);
    
    // Reset the Matrix to check accuracy
    resetOutput();
 
    /* Convolution using one thread per element */
    printf("\n**Case 3: Convolution with 1 thread per value * * * * * * *\n");

    int numbers = inputRows * inputColumns;
    pthread_t threads3[numbers];
    int currNum = 0;
    int currIndex[2];

    startTime = clock();
    for(int i = 0; i < inputRows; i++) {
        for(int j = 0; j < inputColumns; j++){
            currIndex[0] = i;
            currIndex[1] = j;
            if(pthread_create(&(threads3[currNum]), NULL, &ConvolutionPerData, currIndex)) {
                printf("Pthread create error!\n");
            }
            currNum++;
        }
    }

    for(int i = 0; i < numbers; i++) {
        pthread_join(threads3[i], NULL);
    }

    printf("Time took: %li\n", clock() - startTime);
    printf("Output Matrix:");
    print_matrix(outputMatrix, inputRows, inputColumns);

    return 0;
}

/* Helper Functions
 * ================
 */
// Function to print out the result matrix
void print_matrix(int arr[100][100], int numRows, int numCols){
    printf("\n");
    for(int i = 0; i < numRows; i++){
        for(int j = 0; j < numCols; j++){
            printf("%3d ",arr[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Reset the outputMatrix with 0's
void resetOutput() {
    for(int i = 0; i < inputRows; i++){
        for(int j = 0; j < inputColumns; j++){
            outputMatrix[i][j] = 0;
        }
    }
}
