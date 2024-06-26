/**
 * @file
 */
#ifndef __SCHED_H__
#define __SCHED_H__

#include <sys/types.h>
#include <time.h>

/**
 * @brief  Return the maximum priority of the thread can be set
 * @param  policy: The scheduling policy to provide. Currently,
 *         the system only supports SCHED_RR.
 * @retval int: The maximum priority of the thread can be set.
 */
int sched_get_priority_max(int policy);

/**
 * @brief  Return the minimum priority of the thread can be set
 * @param  policy: The scheduling policy to provide. Currently,
 *         the system only supports SCHED_RR.
 * @retval int: The minimum priority of the thread can be set.
 */
int sched_get_priority_min(int policy);

/**
 * @brief  Write the round-robin time quantum of the scheduler into
 *         the timespec structure pointed to by tp
 * @param  pid: Not used (The whole system shares the same scheduling
 *         policy).
 * @param  tp:  The timespec structure to provide.
 * @retval int: The minimum priority of the thread can be set.
 */
int sched_rr_get_interval(pid_t pid, struct timespec *tp);

/**
 * @brief  To cause the calling thread to relinquish the CPU
 * @retval int: 0 on success and nonzero error number on error.
 */
int sched_yield(void);

#endif
