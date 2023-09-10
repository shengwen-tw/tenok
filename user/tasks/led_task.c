#include "tenok/unistd.h"
#include "tenok/semaphore.h"
#include "tenok/sys/resource.h"
#include "task.h"
#include "bsp_drv.h"
#include "semaphore.h"

sem_t sem_led;

void led_task1(void)
{
    set_program_name("led1");
    setpriority(0, getpid(), 3);

    sem_init(&sem_led, 0, 0);

    int state = 1;
    while(1) {
        sem_wait(&sem_led);

        led_write(state);

        state = (state + 1) % 2;
    }
}

void led_task2(void)
{
    set_program_name("led2");
    setpriority(0, getpid(), 3);

    while(1) {
        sem_post(&sem_led);
        sleep(1000);
    }
}

HOOK_USER_TASK(led_task1);
HOOK_USER_TASK(led_task2);
