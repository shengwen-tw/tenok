#include <tenok.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <kernel/task.h>

static pthread_mutex_t mutex;

static void print(int fd, char *str)
{
    int len = strlen(str);
    write(fd, str, len);
}

void mutex_task_high(void)
{
    setprogname("mutex high");

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
#if 1 /* priority inheritance enabled */
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
#else /* priority inheritance disabled */
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_NONE);
#endif
    pthread_mutex_init(&mutex, &attr);

    int serial_fd = open("/dev/serial0", O_RDWR);

    print(serial_fd, "[mutex high] attempt to lock the mutex in 5 seconds.\n\r");

    sleep(5);

    while(1) {
        /* start of the critical section */
        pthread_mutex_lock(&mutex);

        print(serial_fd, "[mutex high] highest-priority thread locked the mutex successfully.\n\r");

        /* end of the critical section */
        pthread_mutex_unlock(&mutex);

        /* simulate some work */
        sleep(2);
    }
}

void mutex_task_median(void)
{
    setprogname("mutex median");

    int serial_fd = open("/dev/serial0", O_RDWR);

    /* occupy the cpu to block the lowest-prioiry thread after 3 seconds */
    print(serial_fd, "[mutex median] block the lowest-priority thread in 3 seconds.\n\r");
    sleep(3);

    /* occupy the cpu for 20 seconds */
    print(serial_fd, "[mutex median] block the lowest-priority thread for 10 seconds.\n\r");

    struct timespec start_time, curr_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while(1) {
        clock_gettime(CLOCK_MONOTONIC, &curr_time);

        if((curr_time.tv_sec - start_time.tv_sec) > 10)
            break;
    }

    /* pause the thread af the job is finished */
    while(1) {
        pause();
    }
}

void mutex_task_low(void)
{
    setprogname("mutex low");

    int serial_fd = open("/dev/serial0", O_RDWR);

    while(1) {
        /* start of the critical section */
        pthread_mutex_lock(&mutex);

        print(serial_fd, "[mutex low] lowest-priority thread locked the mutex successfully.\n\r");

        /* simulate some works */
        sleep(2);

        /* end of the critical section */
        pthread_mutex_unlock(&mutex);
    }
}

HOOK_USER_TASK(mutex_task_high, 3, 512);
HOOK_USER_TASK(mutex_task_median, 2, 512);
HOOK_USER_TASK(mutex_task_low, 1, 512);
