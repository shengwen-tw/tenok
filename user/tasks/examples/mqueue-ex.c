#include <tenok.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <mqueue.h>

#include <kernel/task.h>

#include "uart.h"

#define MSG_SIZE_MAX 25

void message_queue_task1(void)
{
    setprogname("queue1");

    char *str = "mqueue: greeting\n\r";

    struct mq_attr attr = {
        .mq_maxmsg = 100,
        .mq_msgsize = MSG_SIZE_MAX,
    };
    mqd_t mqdes_print = mq_open("/my_message", O_CREAT | O_WRONLY, &attr);

    while(1) {
        mq_send(mqdes_print, (char *)str, strlen(str), 0);
        sleep(1);
    }
}

void message_queue_task2(void)
{
    setprogname("queue2");

    int serial_fd = open("/dev/serial0", 0);

    struct mq_attr attr = {
        .mq_maxmsg = 100,
        .mq_msgsize = MSG_SIZE_MAX,
    };
    mqd_t mqdes_print = mq_open("/my_message", O_CREAT | O_RDONLY, &attr);

    char str[MSG_SIZE_MAX] = {0};

    while(1) {
        /* read message queue */
        mq_receive(mqdes_print, str, MSG_SIZE_MAX, 0);

        /* write serial */
        write(serial_fd, str, strlen(str));
    }
}

HOOK_USER_TASK(message_queue_task1, 0, 512);
HOOK_USER_TASK(message_queue_task2, 0, 512);
