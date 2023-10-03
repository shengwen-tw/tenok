/**
 * @file
 */
#ifndef __KERNEL_TIME_H__
#define __KERNEL_TIME_H__

#include <stdbool.h>

#include <kernel/list.h>

#include <tenok/time.h>

struct timer {
    int  id;
    int  flags;
    bool enabled;
    struct sigevent sev;
    struct itimerspec setting;
    struct itimerspec ret_time; /* for returning to the getter */
    struct timespec counter;    /* internal down counter */
    struct thread_info *thread; /* backlink to the thread */
    struct list g_list;         /* linked to the global list of timers in the system */
    struct list list;           /* linked to the timer list of the thread */
};

void timer_up_count(struct timespec *time);
void timer_down_count(struct timespec *time);
void time_add(struct timespec *time, time_t sec, long nsec);
void get_sys_time(struct timespec *tp);
void set_sys_time(struct timespec *tp);
void sys_time_update_handler(void);

#endif
