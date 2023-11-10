#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tenok.h>
#include <unistd.h>

#include <kernel/task.h>

static int serial_fd;

void *my_thread(void *arg)
{
    for (int i = 0; i < 10; i++) {
        dprintf(serial_fd, "hello new thread.\n\r");
        sleep(1);
    }

    return 0;
}

void pthread_task(void)
{
    setprogname("pthread-ex");

    serial_fd = open("/dev/serial0", O_RDWR);
    if (serial_fd < 0) {
        exit(1);
    }

    dprintf(serial_fd,
            "a new thread will be created and"
            " canceled after 10 seconds.\n\r");

    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 3;
    pthread_attr_init(&attr);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setstacksize(&attr, 1024);

    pthread_t tid;
    if (pthread_create(&tid, &attr, my_thread, NULL) < 0) {
        exit(1);
    }

    pthread_join(tid, NULL);

    while (1) {
        sleep(1);
    }
}

HOOK_USER_TASK(pthread_task, 0, 1024);
