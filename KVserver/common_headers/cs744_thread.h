
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// So we can hopefully ensure that locks and condition variables
// are initialised before use.
#define LOCK_COND_INIT_MAGIC 0x4a3acd94

struct lock {
	int init;
	pthread_mutex_t mutex;
};
struct read_write_lock {
	int init;
	pthread_rwlock_t rwlock;
};
struct condition {
	int init;
	pthread_cond_t cond;
};

// Sush up -Wall if any of these are unused.
static void lock_init(struct lock *lock) __attribute__((unused));
static void lock_acquire(struct lock *lock) __attribute__((unused));
static void lock_release(struct lock *lock) __attribute__((unused));
static void cond_init(struct condition *cond) __attribute__((unused));
static void cond_wait(struct condition *cond, struct lock *lock) __attribute__((unused));
static void cond_signal(struct condition *cond, struct lock *lock) __attribute__((unused));
static void cond_broadcast(struct condition *cond, struct lock *lock) __attribute__((unused));
static void read_write_lock_init(struct read_write_lock * ReadWritelock) __attribute__((unused));
static void read_lock_acquire(struct read_write_lock * ReadWritelock)__attribute__((unused));
static void write_lock_acquire(struct read_write_lock * ReadWritelock)__attribute__((unused));
static void read_write_lock_release(struct read_write_lock * ReadWritelock)__attribute__((unused));


static void
__assert(int satisfied, const char *message)
{	if (!satisfied) {
		fprintf(stderr, "FATAL ERROR: %s\n", message);
		exit(1);
	}
}

static void
lock_init(struct lock *lock)
{
	lock->init = LOCK_COND_INIT_MAGIC;
	pthread_mutex_init(&lock->mutex, NULL);
}
static void read_write_lock_init(struct read_write_lock * ReadWritelock){
	ReadWritelock->init = LOCK_COND_INIT_MAGIC;
	pthread_rwlock_init(&ReadWritelock->rwlock,NULL);
}
static void read_lock_acquire(struct read_write_lock * ReadWritelock){
	__assert(ReadWritelock->init == LOCK_COND_INIT_MAGIC,
		" read_lock_acquire used before lock was initialised!");
	pthread_rwlock_rdlock(&ReadWritelock->rwlock);
}
static void write_lock_acquire(struct read_write_lock * ReadWritelock){
	__assert(ReadWritelock->init == LOCK_COND_INIT_MAGIC,
		" read_lock_acquire used before lock was initialised!");
	pthread_rwlock_wrlock(&ReadWritelock->rwlock);
}
static void read_write_lock_release(struct read_write_lock * ReadWritelock){
	__assert(ReadWritelock->init == LOCK_COND_INIT_MAGIC,
		" read_lock_acquire used before lock was initialised!");
	pthread_rwlock_unlock(&ReadWritelock->rwlock);
}
static void
lock_acquire(struct lock *lock)
{
	__assert(lock->init == LOCK_COND_INIT_MAGIC,
		"lock_acquire used before lock was initialised!");
	pthread_mutex_lock(&lock->mutex);
}

static void
lock_release(struct lock *lock)
{
	__assert(lock->init == LOCK_COND_INIT_MAGIC,
		"lock_release used before lock was initialised!");
	__assert(pthread_mutex_trylock(&lock->mutex) != 0,
		"lock_release on unlocked lock!");
	pthread_mutex_unlock(&lock->mutex);
}

static void
cond_init(struct condition *cond)
{
	cond->init = LOCK_COND_INIT_MAGIC;
	pthread_cond_init(&cond->cond, NULL);
}

static void
cond_wait(struct condition *cond, struct lock *lock)
{
	__assert(pthread_mutex_trylock(&lock->mutex) != 0,
		"cond_wait not called with lock held!");
	__assert(cond->init == LOCK_COND_INIT_MAGIC,
		"cond_wait used before cond was initialised!");
	__assert(lock->init == LOCK_COND_INIT_MAGIC,
		"cond_wait used before lock was initialised!");
	pthread_cond_wait(&cond->cond, &lock->mutex);
}

static void
cond_signal(struct condition *cond, struct lock *lock)
{
	// See comment in cond_wait().
	__assert(pthread_mutex_trylock(&lock->mutex) != 0,
		"cond_signal not called with lock held!");
	__assert(cond->init == LOCK_COND_INIT_MAGIC,
		"cond_signal used before cond was initialised!");
	__assert(lock->init == LOCK_COND_INIT_MAGIC,
		"cond_signal used before lock was initialised!");
	pthread_cond_signal(&cond->cond);
}

static void
cond_broadcast(struct condition *cond, struct lock *lock)
{
	// See comment in cond_wait().
	__assert(pthread_mutex_trylock(&lock->mutex) != 0,
		"cond_broadcast not called with lock held!");
	__assert(cond->init == LOCK_COND_INIT_MAGIC,
		"cond_broadcast used before cond was initialised!");
	__assert(lock->init == LOCK_COND_INIT_MAGIC,
		"cond_broadcast used before lock was initialised!");
	pthread_cond_broadcast(&cond->cond);
}



