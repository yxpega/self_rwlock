/* H file of implementation of self_rwlock */
#ifndef SLF_RWLOCK_H
#define SLF_RWLOCK_H

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#define USLEEP_TIME 500

typedef struct {
        unsigned int wlock;
        unsigned int rlocks;
        pthread_mutex_t mtx_lock;
} slf_rwlock;

static inline int slf_rwlock_init(slf_rwlock *ptr)
{
        assert(ptr != NULL);
        int err = pthread_mutex_init(&ptr->mtx_lock, NULL);
        ptr->wlock = 0u;
        ptr->rlocks = 0u;
        return err;
}

static inline int slf_rwlock_try_lockr(slf_rwlock *ptr)
{
        assert(ptr != NULL);
        int err;

        err = pthread_mutex_lock(&ptr->mtx_lock);
        if (err != 0) {
                return err;
        }
        if (ptr->wlock > 0 || ptr->rlocks == UINT_MAX) {
                (void)pthread_mutex_unlock(&ptr->mtx_lock);
                return EBUSY;
        }
        ptr->rlocks++;
        err = pthread_mutex_unlock(&ptr->mtx_lock);

        return err;
}

static inline int slf_rwlock_unlockr(slf_rwlock *ptr)
{
        assert(ptr != NULL);
        int err;

        err = pthread_mutex_lock(&ptr->mtx_lock);
        if (err != 0) {
                return err;
        }
        if (ptr->rlocks == 0u) {
                (void)pthread_mutex_unlock(&ptr->mtx_lock);
                return EINVAL;
        }
        ptr->rlocks--;
        err = pthread_mutex_unlock(&ptr->mtx_lock);

        return err;
}

static inline int slf_rwlock_try_lockw(slf_rwlock *ptr)
{
        assert(ptr != NULL);
        int err;

        err = pthread_mutex_lock(&ptr->mtx_lock);
        if (err != 0) {
                return err;
        }
        if (ptr->rlocks > 0 || ptr->wlock > 0) {
                (void)pthread_mutex_unlock(&ptr->mtx_lock);
                return EBUSY;
        }
        ptr->wlock++;
        ptr->rlocks = UINT_MAX;
        err = pthread_mutex_unlock(&ptr->mtx_lock);

        return err;
}

static inline int slf_rwlock_unlockw(slf_rwlock *ptr)
{
        assert(ptr != NULL);
        int err;

        err = pthread_mutex_lock(&ptr->mtx_lock);
        if (err != 0) {
                return err;
        }
        if (ptr->wlock == 0u) {
                (void)pthread_mutex_unlock(&ptr->mtx_lock);
                return EINVAL;
        }
        ptr->wlock--;
        ptr->rlocks = 0u;
        err = pthread_mutex_unlock(&ptr->mtx_lock);

        return err;
}

static inline int slf_rwlock_lockr(slf_rwlock *ptr)
{
        int err;

        do {
                err = slf_rwlock_try_lockr(ptr);
                if (err == 0 || err != EBUSY) {
                        break;
                }
                usleep(USLEEP_TIME);
        } while (err == EBUSY);

        return err;
}

static inline int slf_rwlock_lockw(slf_rwlock *ptr)
{
        int err;

        do {
                err = slf_rwlock_try_lockw(ptr);
                if (err == 0 || err != EBUSY) {
                        break;
                }
                usleep(USLEEP_TIME);
        } while (err == EBUSY);

        return err;
}

static inline void slf_rwlock_destroy(slf_rwlock *ptr)
{
        assert(ptr != NULL);
        (void)pthread_mutex_destroy(&ptr->mtx_lock);
        ptr->wlock = 0u;
        ptr->rlocks = 0u;
}
#endif
