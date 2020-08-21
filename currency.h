/* File encoding format:UTF-8  */

#ifndef _CURRENCY_H_
#define _CURRENCY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

/* 0是Linux，1是windows */
#define SYSTEM 0

/* ************错误代码************ */
#define ERR_SUCCESS 0       /* SUCCESS */
#define ERR_PARAMETER 1     /* 传入的参数不对 */
#define ERR_NULL_POINTER 2  /* 传入空指针 */
#define ERR_ADDRESS_FALSE 3 /* 地址信息错误 */
#define ERR_SRAND_FLASE 4   /* 可能是种子位置错误，请重新指向 */

#define SRAND_SEED "/dev/random"

typedef struct
{
    uint16_t byte;
    uint16_t kilobyte;
    uint16_t megabyte;
    uint16_t gigabyte;
} file_size; //文件大小结构体，防止单纯记录字节出现文件过大的现象

typedef struct path_str
{
    int8_t *file_name;        //文件名
    int8_t *absolute_address; //绝对地址
    file_size *file_size;     //文件大小
    struct path_str *prev;    //上一个指针
    struct path_str *next;    //下一个指针
} path_str;                   //文件结构体

typedef struct worker
{
    void *(*worker)(void *arg);
    void *arg;
    struct worker *prev;
    struct worker *next;
} pthread_work_t; //任务链表的结构体

typedef struct
{
    /* 互斥锁 */
    pthread_mutex_t *queue_mutex;
    /* 条件变量 */
    pthread_cond_t *queue_cond;
    /* 要等待的线程队列链表头 */
    pthread_work_t *queue_head;
    /* 是否销毁进程池，是1，否0 */
    uint16_t destruction;
    /* 线程id */
    pthread_t *pthread_id;
    /* 线程池最大的线程数 */
    uint32_t Max_pthread_num;
    /* 当前正在等待的线程数 */
    int32_t Current_wait_pthread_num;
} pthread_pool_t; //线程池结构体

typedef struct
{
    int8_t *file_name;             //文件名
    int8_t *file_original_address; //文件原地址
    int8_t *original_address;      //目录原地址
    int8_t *destination_address;   //目标地址
    file_size *file_size;          //文件大小
} copy_str;

extern uint16_t errnum;             //错误码，全局定义，函数错误都会在这定义
extern void perrnum(const char *s); //打印出现的错误情况

extern int32_t Conversion(uint64_t byte, file_size *size);                                             //字节换算，文件大小用的函数
extern file_size *Conversion_add(path_str *head);                                                      //将所有文件的文件大小加起来
extern file_size *Conversion_minus(file_size *all_size, file_size *minus_size);                        //全部文件大小减去返回的文件大小值
extern path_str *Find_all_file(const int8_t *absolute_address, path_str *head);                        //找到目录下的所有文件，并返回path_str *的头节点，如果失败或者目录内无文件则返回NULL
extern path_str *Find_file(const int8_t *absolute_address, const int8_t *file_suffix, path_str *head); //找到目录下的所有后缀相同文件，并返回path_str *的头节点，如果失败或者目录内无相应的文件则返回NULL
extern int32_t tree_printing_directory(const int8_t *absolute_address, int16_t loop);                  //以树状图的格式打印目录下的所有文件
extern int32_t destroy_path_str_list(path_str *head);                                                  //摧毁此链表

extern pthread_pool_t *pthread_pool_init(void);                                                                    //初始化线程池
extern void current_wait_num(pthread_work_t *head, pthread_pool_t *pthread_pool);                                  //计算任务链表的大小，并调用add_pthread来动态调整线程数量，并统一唤醒所有线程
extern pthread_work_t *pthread_add_wait_queue(pthread_work_t *head, void *(*task_function)(void *arg), void *arg); //动态调整线程数量，但是不能大于规定的最大值
extern int32_t destroy_pthread_pool(pthread_pool_t *pthread_pool);                                                 //摧毁线程池

extern void *copy(copy_str *copy_arg);                                                                                      //copy主函数
extern copy_str *copy_pretreatment(path_str *file_info, const int8_t *original_address, const int8_t *destination_address); //copy的传入数据处理

extern int32_t rrand(int32_t limit); //随机值
#endif
