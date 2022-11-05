#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#define spin_lock_irq(lock) \
	do { \
	irq_disable(); \
	spin_lock(lock); \
	} while(0)

#define spin_unlock_irq(lock) \
	do { \
	irq_enable(); \
	spin_unlock(lock); \
	} while(0)

typedef uint32_t spinlock_t;

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif