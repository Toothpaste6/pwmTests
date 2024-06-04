/*pwmTests starts a pwm of period 1s and duty cycle 50%, configures P8_8 as GPIO input pin, and enables rising edge on P8_8
It prints the timestamp to the gui 
W. Stone 3/23/2024 original code written for lab4 Umass ECE231 Spring 2024*/
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
//buffer size is defined here in an attempt to eliminate segmentation faults
//why? no clue but when I use int buffer[10] everything seems to break
#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];

void* threadFunction(void *var) {
    printf("Thread function started\n");
    // Casting input argument type to integer
    int *input = (int *) var;
    // Pause the program for 1 second
    sleep(1);
    // The following code is used to receive interrupts on the registered pin
    char InterruptPath[40];
    sprintf(InterruptPath, "/sys/class/gpio/gpio67/value");
    printf("Interrupt path: %s\n", InterruptPath);
    int epfd;
    struct epoll_event ev;
    // Step 1: Open the interrupt file
    // File pointer 
    FILE* fp = fopen(InterruptPath, "r");
    //checks for errors when opening the interrupt file
    if (!fp) {
        perror("Failed to open interrupt file");
        pthread_exit(NULL);
    }
    // Get the file descriptor
    int fd = fileno(fp);
    // Step 2: Create epoll instance to monitor I/O events on interrupt file
    epfd = epoll_create(1);
    //checks to see if there are any errors in the epoll instance (was having issues with this when doing the lab assignment)
    if (epfd == -1) {
        perror("Failed to create epoll instance");
        fclose(fp);
        pthread_exit(NULL);
    }
    // Step 3: Register events that will be monitored
    // detects whenever a new data is available for read (EPOLLIN)
    // signals the read events when the available read value has changed (EPOLLET)
    ev.events = EPOLLIN | EPOLLET;
    // Step 4: Register interrupt file with epoll interface for monitoring
    ev.data.fd = fd;
    //checks the addition of the interrupt file to epoll
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("Failed to add interrupt file to epoll");
        close(epfd);
        fclose(fp);
        pthread_exit(NULL);
    }
    int capture_interrupt;
    struct epoll_event ev_wait;
    struct timespec tm;

    for(int i = 0; i < BUFFER_SIZE; i++) {
        // Step 5: Wait for epoll interface to signal the change
        capture_interrupt = epoll_wait(epfd, &ev_wait, 1, -1);
        clock_gettime(CLOCK_MONOTONIC_RAW, &tm);
        //printf("Interrupt received\n");
        buffer[i] = tm.tv_sec;
    }
    close(epfd);
    fclose(fp); //required to close for segmentation faults?
    //printf("Thread function completed\n");
    return NULL;
}

int main() {
    // Instantiate argument required for thread creation
    pthread_t thread_id;
    printf("Before Thread\n");
    // Create thread
    if (pthread_create(&thread_id, NULL, threadFunction, (void*)(buffer)) != 0) {
        perror("Failed to create thread");
        return EXIT_FAILURE;
    }
    // Wait for thread to finish
    if (pthread_join(thread_id, NULL) != 0) {
        perror("Failed to join thread");
        return EXIT_FAILURE;
    }
    // Print buffer contents to see if the code made it this far before segmentation faults
    //printf("Buffer contents:\n");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("time is %d\n", buffer[i]);
    }
    printf("After Thread\n");
    pthread_exit(NULL);
}