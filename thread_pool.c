/*
 *  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *
 *  If you want to use the thread pool, join this header file *
 * Use with dynamic libthreadpool                                               *
 * Creater:fishdivinity                                                                          *
 * Date:2020-8-13                                                                                  *
 * File encoding format:UTF-8                                                         *
 *  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include "currency.h"

#define PTHREAD_DEFAULT 10
#define PTHREAD_MAX 100

/* 测试后门 */
#define DEBUG printf("%d\n", __LINE__);
#define DEBUGUP printf("%lx线程被销毁\n", pthread_self());
#define DEBUGTHREAD printf("This pthread id is %lx\n", pthread_self());

/* 提前定义一些接口函数 */
/* 这个函数就是线程池的最基本组成部分 */
static void *start_routine(void *arg); //||
/* 初始化线程池，因为里面的一些参数是只在这个文件里有的，所以不方便外调 */
pthread_pool_t *pthread_pool_init(void); //||
/* 动态分配线程数，但是最大不能超过此文件定义的Max */
static int32_t add_pthread(int32_t wait_pthread_num, pthread_pool_t *pthread_pool); //||
/* 摧毁线程池 */
int32_t destroy_pthread_pool(pthread_pool_t *pthread_pool); //||
/* 检索加进来的任务链表总数 */
void current_wait_num(pthread_work_t *head, pthread_pool_t *pthread_pool); //||
/* 给新的任务初始化，并组成链表，外面可以调用 */
pthread_work_t *pthread_add_wait_queue(pthread_work_t *head, void *(*task_function)(void *arg), void *arg); //||

static void cleanup(void *arg)
{
    pthread_mutex_unlock((pthread_mutex_t *)arg);
}

static void *start_routine(void *arg)
{
    pthread_pool_t *temp = (pthread_pool_t *)arg;
    pthread_cleanup_push(cleanup, temp->queue_mutex);

    while (1)
    {
        pthread_mutex_lock(temp->queue_mutex);
        while (temp->Current_wait_pthread_num == 0 && temp->destruction == 0)
        {
            pthread_cond_wait(temp->queue_cond, temp->queue_mutex);
        }
        if (temp->destruction)
        {
            pthread_mutex_unlock(temp->queue_mutex);

            pthread_exit(NULL);
        }
        else
        {
            temp->Current_wait_pthread_num--;
            pthread_work_t *work = temp->queue_head;
            temp->queue_head = temp->queue_head->next;
            pthread_mutex_unlock(temp->queue_mutex);

            (*(work->worker))(work->arg);
            free(work);
            work = NULL;
        }
    }
    pthread_cleanup_pop(0);
}

pthread_pool_t *pthread_pool_init(void)
{
    pthread_pool_t *pthread_pool = calloc(1, sizeof(pthread_pool_t));
    if (NULL == pthread_pool)
    {
        //申请线程池堆空间失败
        return NULL;
    }
    pthread_pool->queue_mutex = calloc(1, sizeof(pthread_mutex_t));
    if (NULL == pthread_pool->queue_mutex)
    {
        //申请互斥锁堆空间失败
        free(pthread_pool);
        return NULL;
    }
    pthread_mutex_init(pthread_pool->queue_mutex, NULL); //互斥锁初始化
    pthread_pool->queue_cond = calloc(1, sizeof(pthread_cond_t));
    if (NULL == pthread_pool->queue_cond)
    {
        // //申请条件变量堆空间失败
        free(pthread_pool->queue_mutex);
        free(pthread_pool);
        return NULL;
    }
    pthread_cond_init(pthread_pool->queue_cond, 0);
    pthread_pool->queue_head = NULL;                                       //初始化，头节点为NULL
    pthread_pool->destruction = 0;                                         //初始化，销毁为0
    pthread_pool->pthread_id = calloc(PTHREAD_DEFAULT, sizeof(pthread_t)); //初始化id，id数为默认值
    pthread_pool->Max_pthread_num = PTHREAD_DEFAULT;
    pthread_pool->Current_wait_pthread_num = 0; //初始化的等待数为-1，为区分是初始化还是运行结束

    for (int32_t p = 0; p < pthread_pool->Max_pthread_num; p++)
    {
        pthread_create(((pthread_pool->pthread_id) + p), NULL, start_routine, (void *)pthread_pool);
    }
    return pthread_pool;
}

static int32_t add_pthread(int32_t wait_pthread_num, pthread_pool_t *pthread_pool)
{
    if (wait_pthread_num < PTHREAD_DEFAULT)
    {
        int32_t i;
        for (i = pthread_pool->Max_pthread_num; i != wait_pthread_num; i--)
        {
            pthread_cancel(pthread_pool->pthread_id[i - 1]);
            pthread_join(pthread_pool->pthread_id[i - 1], NULL);
        }
        pthread_pool->pthread_id = realloc(pthread_pool->pthread_id, sizeof(pthread_t) * i);
        pthread_pool->Max_pthread_num = i;
        return pthread_pool->Max_pthread_num;
    }
    else
    {
        if (wait_pthread_num > PTHREAD_MAX) //如果链表的大小大于线程池定义的最大值，就按照最大值计算
        {
            pthread_pool->pthread_id = realloc(pthread_pool->pthread_id, PTHREAD_MAX * sizeof(pthread_t));
            int32_t i;
            for (i = (pthread_pool->Max_pthread_num) - 1; i < PTHREAD_MAX; i++)
            {
                pthread_create(((pthread_pool->pthread_id) + i), NULL, start_routine, (void *)pthread_pool);
            }
            pthread_pool->Max_pthread_num = PTHREAD_MAX;
            return pthread_pool->Max_pthread_num;
        }
        else
        {
            pthread_pool->pthread_id = realloc(pthread_pool->pthread_id, sizeof(pthread_t) * wait_pthread_num);
            int32_t i;
            for (i = (pthread_pool->Max_pthread_num) - 1; i < wait_pthread_num; i++)
            {
                pthread_create(((pthread_pool->pthread_id) + i), NULL, start_routine, (void *)pthread_pool);
            }
            pthread_pool->Max_pthread_num = wait_pthread_num;
            return pthread_pool->Max_pthread_num;
        }
    }
}

int32_t destroy_pthread_pool(pthread_pool_t *pthread_pool)
{
    if (pthread_pool->destruction == 1)
    {
        return -1;
    }
    else
    {
        pthread_pool->destruction = 1;

        /* 唤醒所有线程 */
        pthread_cond_broadcast(pthread_pool->queue_cond);

        for (int32_t i = 0; i < pthread_pool->Max_pthread_num; i++)
        {
            /* 回收线程 */
            pthread_join(pthread_pool->pthread_id[i], NULL);
        }
        /* destroy the pthread id list */
        free(pthread_pool->pthread_id);
        pthread_pool->pthread_id = NULL;
        /* destroy the queue list */
        while (pthread_pool->queue_head != NULL)
        {
            pthread_work_t *p = pthread_pool->queue_head;
            pthread_pool->queue_head = pthread_pool->queue_head->next;
            free(p);
        }
        pthread_pool->queue_head = NULL;
        /* destroy the mutex */
        pthread_mutex_destroy(pthread_pool->queue_mutex);
        free(pthread_pool->queue_mutex);

        /* destroy the cond */
        pthread_cond_destroy(pthread_pool->queue_cond);
        free(pthread_pool->queue_cond);

        free(pthread_pool);
        return 0;
    }
}

void current_wait_num(pthread_work_t *head, pthread_pool_t *pthread_pool)
{
    if (NULL == head || NULL == pthread_pool)
        ;
    else
    {
        pthread_work_t *p = head;
        int32_t num = 1;
        while (p->next != NULL)
        {
            p = p->next;
            num++;
        }

        if (add_pthread(num, pthread_pool))
        {
            /* sleep(2); */
            pthread_pool->Current_wait_pthread_num = num;
            pthread_pool->queue_head = head;
            pthread_cond_broadcast(pthread_pool->queue_cond);
        }
    }
}

pthread_work_t *pthread_add_wait_queue(pthread_work_t *head, void *(*task_function)(void *arg), void *arg)
{
    if (task_function == NULL)
    {
        return NULL;
    }
    else
    {
        pthread_work_t *new = calloc(1, sizeof(pthread_work_t));
        new->worker = task_function;
        new->arg = arg;
        if (NULL == head)
        {
            new->prev = new;
            new->next = NULL;
            return new;
        }
        else
        {
            head->prev->next = new;
            new->prev = head->prev;
            new->next = NULL;
            head->prev = new;
            return head;
        }
    }
}