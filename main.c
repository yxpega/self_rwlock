/* Test self_rwlock */
#include <stdio.h>
#include "rwlock.h"

#define THREAD_NUMBER 8

slf_rwlock g_rwl;
int g_num;

void* writer(void *args)
{
        int num = (int)(long long)args;
        slf_rwlock_lockw(&g_rwl);
        printf("write %d into g_num...\n", num);
        g_num = num;
        printf("finish writing, g_num: %d\n", g_num);
        slf_rwlock_unlockw(&g_rwl);
        return NULL;
}

void* reader(void *args)
{
        slf_rwlock_lockr(&g_rwl);
        printf("read g_num: %d\n", g_num);
        slf_rwlock_unlockr(&g_rwl);
        return NULL;
}

int main()
{
        pthread_t t[THREAD_NUMBER];
        slf_rwlock_init(&g_rwl);
        g_num = 0;

        for (int i = 0; i < THREAD_NUMBER / 2; i++) {
                pthread_create(&t[i], NULL, writer, (void *)(long long)(i + 1));
        }

        for (int i = THREAD_NUMBER / 2; i < THREAD_NUMBER; i++) {
                pthread_create(&t[i], NULL, reader, NULL);
        }

        for (int i = 0; i < THREAD_NUMBER; i++) {
                pthread_join(t[i], NULL);
        }

        slf_rwlock_destroy(&g_rwl);

        return 0;
}
