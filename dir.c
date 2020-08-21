/* File encoding format:UTF-8  */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "currency.h"

/* typedef struct 
{
    uint16_t byte;
    uint16_t kilobyte;
    uint16_t megabyte;
    uint16_t gigabyte;
}file_size; //文件大小结构体，防止单纯记录字节出现文件过大的现象

typedef struct path_str
{
    int8_t *file_name;           //文件名
    int8_t *absolute_address;    //绝对地址
    file_size *file_size; //文件大小
    struct path_str *prev;       //上一个指针
    struct path_str *next;       //下一个指针
} path_str;                      //文件结构体 */

/* 字节换算，文件大小用的函数 */
int32_t Conversion(uint64_t byte, file_size *size);
/* 将所有文件的文件大小加起来 */
file_size *Conversion_add(path_str *head);
/* 减去完成的值，返回all_size的地址 */
file_size *Conversion_minus(file_size *all_size, file_size *minus_size);
/* 找到目录下的所有文件，返回NULL为错误，可能是地址不对，或者目录里没有文件 */
path_str *Find_all_file(const int8_t *absolute_address, path_str *head);
/* 找到目录下的所有后缀相同的文件，返回NULL为错误，可能是地址不对，或者目录里没有指定的文件 */
path_str *Find_file(const int8_t *absolute_address, const int8_t *file_suffix, path_str *head);
/* 把链表连接起来 */
path_str *Linked_list(path_str *head, path_str *new);
/* 摧毁链表 */
int32_t destroy_path_str_list(path_str *head);
/* 以树状图的方式打印目录内的所有文件与子目录 */
int32_t tree_printing_directory(const int8_t *absolute_address, int16_t loop);

int32_t Conversion(uint64_t byte, file_size *size)
{
    /* 初始化，先置零 */
    size->byte = 0;
    size->gigabyte = 0;
    size->kilobyte = 0;
    size->megabyte = 0;

    int32_t byte_size;
    byte_size = 1024; //字节换算的固定单位
    size->byte = byte % byte_size;
    size->kilobyte = byte / byte_size;
    if (size->kilobyte >= byte_size)
    {
        size->megabyte = size->kilobyte / byte_size;
        size->kilobyte %= byte_size;
        if (size->megabyte >= byte_size)
        {
            size->gigabyte = size->megabyte / byte_size;
            size->megabyte %= byte_size;
        }
    }
    return 0;
}

file_size *Conversion_add(path_str *head)
{
    path_str *p = head;
    int32_t byte_size;
    byte_size = 1024;                                   //字节换算的固定单位
    file_size *all_size = calloc(1, sizeof(file_size)); //calloc在申请了堆空间之后，还会对堆空间内的内容置零
    int16_t loop = 1;
    while (1)
    {
        all_size->byte += p->file_size->byte;
        all_size->kilobyte += p->file_size->kilobyte;
        all_size->megabyte += p->file_size->megabyte;
        all_size->gigabyte += p->file_size->gigabyte;

        if (loop % 20 == 0) //偶尔进行整理防止太大溢出
        {
            if (all_size->byte > byte_size)
            {
                all_size->kilobyte += (all_size->byte / byte_size);
                all_size->byte %= byte_size;
            }
            if (all_size->kilobyte > byte_size)
            {
                all_size->megabyte += (all_size->kilobyte / byte_size);
                all_size->kilobyte %= byte_size;
            }
            if (all_size->megabyte > byte_size)
            {
                all_size->gigabyte += (all_size->megabyte / byte_size);
                all_size->megabyte %= byte_size;
            }
        }

        p = p->next;
        if (p == head)
            break;
        loop++;
    }

    if (all_size->byte > byte_size)
    {
        all_size->kilobyte += (all_size->byte / byte_size);
        all_size->byte %= byte_size;
    }
    if (all_size->kilobyte > byte_size)
    {
        all_size->megabyte += (all_size->kilobyte / byte_size);
        all_size->kilobyte %= byte_size;
    }
    if (all_size->megabyte > byte_size)
    {
        all_size->gigabyte += (all_size->megabyte / byte_size);
        all_size->megabyte %= byte_size;
    }

    return all_size;
}

file_size *Conversion_minus(file_size *all_size, file_size *minus_size)
{
    
    int64_t minus_byte, all_byte;
    int32_t byte_size;
    byte_size = 1024;

    if (minus_size->gigabyte > 0)
    {
        minus_byte = minus_byte + ((minus_size->gigabyte) * byte_size * byte_size * byte_size);
    }
    if (minus_size->megabyte > 0)
    {
        minus_byte = minus_byte + ((minus_size->megabyte) * byte_size * byte_size);
    }
    if (minus_size->kilobyte > 0)
    {
        minus_byte = minus_byte + ((minus_size->kilobyte) * byte_size);
    }
    if (minus_size->byte > 0)
    {
        minus_byte = minus_byte + minus_size->byte;
    }

    if (all_size->gigabyte > 0)
    {
        all_byte = all_byte + ((all_size->gigabyte) * byte_size * byte_size * byte_size);
    }
    if (all_size->megabyte > 0)
    {
        all_byte = all_byte + ((all_size->megabyte) * byte_size * byte_size);
    }
    if (all_size->kilobyte > 0)
    {
        all_byte = all_byte + ((all_size->kilobyte) * byte_size);
    }
    if (all_size->byte > 0)
    {
        all_byte = all_byte + all_size->byte;
    }

    all_byte -= minus_byte;

    if (all_byte < 0)
    {
        printf("文件大小大于总文件大小！\n");
        return all_size;
    }
    else
    {
        bzero(all_size, sizeof(file_size));

        all_size->byte = all_byte % byte_size;
        all_size->kilobyte = all_byte / byte_size;
        if (all_size->kilobyte >= byte_size)
        {
            all_size->megabyte = all_size->kilobyte / byte_size;
            all_size->kilobyte = all_size->kilobyte % byte_size;
        }
        if (all_size->megabyte >= byte_size)
        {
            all_size->gigabyte = all_size->megabyte / byte_size;
            all_size->megabyte = all_size->megabyte % byte_size;
        }

        return all_size;
    }
}

path_str *Find_all_file(const int8_t *absolute_address, path_str *head) //找到所有文件，并存入到指针中
{
    if (NULL == absolute_address)
    {
        errnum = ERR_PARAMETER;
        return NULL;
    }
    else
    {

        int8_t path[255];
        strcpy(path, absolute_address);
        if (path[strlen(path) - 1] != '/') //如果目录没有加/，加上
            strcat(path, "/");
        DIR *dp = NULL;
        struct dirent *dp_read = NULL;
        struct stat statbuf;

        if (NULL == (dp = opendir(path)))
        {
            errnum = ERR_ADDRESS_FALSE;
            return NULL;
        }
        else
        {
            while ((dp_read = readdir(dp)) != NULL)
            {
                if (dp_read->d_type == DT_DIR)
                {

                    if ((strcmp(dp_read->d_name, ".") == 0) || (strcmp(dp_read->d_name, "..") == 0))
                        continue;
                    else
                    {
                        int8_t *temp_p = calloc(1, strlen(path) + strlen(dp_read->d_name) + 1);
                        sprintf(temp_p, "%s%s", path, dp_read->d_name);
                        head = Find_all_file(temp_p, head);
                        free(temp_p);
                        temp_p = NULL;
                    }
                }
                else
                {
                    bzero(&statbuf, sizeof(struct stat));
                    path_str *new = calloc(1, sizeof(path_str));
                    new->file_name = calloc(1, strlen(dp_read->d_name) + 1);
                    strcpy(new->file_name, dp_read->d_name);
                    new->absolute_address = calloc(1, strlen(path) + 1);
                    strcpy(new->absolute_address, path);
                    int8_t *temp_path = calloc(1, strlen(path) + strlen(dp_read->d_name) + 1);
                    sprintf(temp_path, "%s%s", path, dp_read->d_name);
                    stat(temp_path, &statbuf);
                    new->file_size = malloc(sizeof(file_size));
                    Conversion(statbuf.st_size, new->file_size);
                    free(temp_path);
                    temp_path = NULL;
                    new->next = NULL;
                    new->prev = NULL;
                    head = Linked_list(head, new);
                }
            }
            closedir(dp);
            return head;
        }
    }
}

path_str *Find_file(const int8_t *absolute_address, const int8_t *file_suffix, path_str *head)
{
    if (NULL == absolute_address || NULL == file_suffix)
    {
        errnum = ERR_PARAMETER;
        return NULL;
    }
    else
    {
        int8_t path[255];
        int8_t temp_name[strlen(file_suffix) + 1];
        strcpy(path, absolute_address);
        if (path[strlen(path) - 1] != '/') //如果目录没有加/，加上
            strcat(path, "/");
        DIR *dp = NULL;
        struct dirent *dp_read = NULL;
        struct stat statbuf;

        if (NULL == (dp = opendir(path)))
        {
            errnum = ERR_ADDRESS_FALSE;
            return NULL;
        }
        else
        {
            while ((dp_read = readdir(dp)) != NULL)
            {
                if (dp_read->d_type == DT_DIR)
                {

                    if ((strcmp(dp_read->d_name, ".") == 0) || (strcmp(dp_read->d_name, "..") == 0))
                        continue;
                    else
                    {
                        int8_t *temp_p = calloc(1, strlen(path) + strlen(dp_read->d_name) + 1);
                        sprintf(temp_p, "%s%s", path, dp_read->d_name);
                        head = Find_file(temp_p, file_suffix, head);
                        free(temp_p);
                        temp_p = NULL;
                    }
                }
                else
                {
                    if (strstr(dp_read->d_name, file_suffix) != NULL)
                    {
                        bzero(&temp_name, strlen(file_suffix) + 1);
                        for (int16_t i = 0; i < strlen(file_suffix); i++)
                        {
                            temp_name[i] = dp_read->d_name[strlen(dp_read->d_name) - strlen(file_suffix) + i];
                        }
                        if (strcmp(temp_name, file_suffix) == 0)
                        {
                            path_str *new = calloc(1, sizeof(path_str));
                            new->file_name = calloc(1, strlen(dp_read->d_name) + 1);
                            strcpy(new->file_name, dp_read->d_name);
                            new->absolute_address = calloc(1, strlen(path) + 1);
                            strcpy(new->absolute_address, path);
                            int8_t *temp_path = calloc(1, strlen(path) + strlen(dp_read->d_name) + 1);
                            sprintf(temp_path, "%s%s", path, dp_read->d_name);
                            stat(temp_path, &statbuf);
                            new->file_size = malloc(sizeof(file_size));
                            Conversion(statbuf.st_size, new->file_size);
                            free(temp_path);
                            temp_path = NULL;
                            new->next = NULL;
                            new->prev = NULL;
                            head = Linked_list(head, new);
                        }
                    }
                }
            }
            closedir(dp);
            return head;
        }
    }
}

path_str *Linked_list(path_str *head, path_str *new) //把地址给链接起来
{
    if (NULL == new)
    {
        errnum = ERR_PARAMETER;
        return NULL;
    }
    else
    {
        if (NULL == head)
        {
            new->next = new;
            new->prev = new;
            return new;
        }
        else
        {
            head->prev->next = new;
            new->prev = head->prev;
            new->next = head;
            head->prev = new;
            return head;
        }
    }
}

int32_t destroy_path_str_list(path_str *head)
{
    if (NULL == head)
    {
        errnum = ERR_PARAMETER;
        return -1;
    }
    else
    {
        path_str *p = head->next;
        path_str *q = NULL;
        while (p->next != head)
        {
            q = p->next;
            free(p->file_name);
            p->file_name = NULL;
            free(p->absolute_address);
            p->absolute_address = NULL;
            free(p->file_size);
            p->file_size = NULL;
            free(p);
            p = q;
        }
        free(head->file_name);
        head->file_name = NULL;
        free(head->absolute_address);
        head->absolute_address = NULL;
        free(head->file_size);
        head->file_size = NULL;
        head->next = NULL;
        head->prev = NULL;
        free(head);
        return 0;
    }
}

int32_t tree_printing_directory(const int8_t *absolute_address, int16_t loop)
{
    if (NULL == absolute_address)
    {
        errnum = ERR_PARAMETER;
        return -1;
    }
    else
    {
        /* if (loop == 0) */
        loop++;

        int8_t path[255];
        strcpy(path, absolute_address);
        if (path[strlen(path) - 1] != '/') //如果目录没有加/，加上
            strcat(path, "/");
        int8_t temppath[strlen(path) + 1];
        bzero(temppath, strlen(path) + 1);
        strcpy(temppath, path);
        int8_t home_dir_name[50];
        int8_t *temp_dir_name = strtok(temppath, "/");

        while (1)
        {
            temp_dir_name = strtok(NULL, "/");
            if (temp_dir_name != NULL)
            {

                bzero(home_dir_name, 50);
                strcpy(home_dir_name, temp_dir_name);
            }
            if (temp_dir_name == NULL)
                break;
        }

        DIR *dp = NULL;
        struct dirent *dp_read = NULL;
        struct stat statbuf;
        if (NULL == (dp = opendir(path)))
        {
            errnum = ERR_ADDRESS_FALSE;
            return -1;
        }
        else
        {
            if (loop == 1)
            {
                printf("\033[1m");
                printf("\033[33m\n %s\033[0m\n", home_dir_name);
            }

            while ((dp_read = readdir(dp)) != NULL)
            {
                if (dp_read->d_type == DT_DIR)
                    ;
                else
                {
                    int32_t temp_loop = loop - 1;
                    /* printf("%d\n", loop); */
                    int32_t temp_reduce = 3;

                    while (1)
                    {
                        if (loop > temp_reduce)
                            temp_loop--;
                        else
                            break;
                        temp_reduce += 2;
                    }

                    while (temp_loop)
                    {
                        printf("   ");
                        if (loop != 1)
                        {
                            printf("\033[33m|\033[0m");
                        }
                        temp_loop--;
                    }
                    printf("\033[33m__\033[1m");
                    printf("\033[34m%s\033[0m\n", dp_read->d_name);
                }
            }

            closedir(dp);
            dp = opendir(path);

            while ((dp_read = readdir(dp)) != NULL)
            {
                if (dp_read->d_type == DT_DIR)
                {

                    if ((strcmp(dp_read->d_name, ".") == 0) || (strcmp(dp_read->d_name, "..") == 0))
                        continue;
                    else
                    {
                        int8_t *temp_p = calloc(1, strlen(path) + strlen(dp_read->d_name) + 1);
                        sprintf(temp_p, "%s%s", path, dp_read->d_name);
                        int32_t temp_loop = loop;

                        while (temp_loop)
                        {
                            printf("  ");

                            temp_loop--;
                        }

                        /* printf("%d",loop); */
                        printf("\033[33m |__ \033[1m");
                        printf("\033[32m%s\033[0m\n", dp_read->d_name);
                        tree_printing_directory(temp_p, loop + 1);
                    }
                }
            }

            closedir(dp);

            return 0;
        }
    }
}

/* int main(int32_t argc, int8_t *argv[])
{
    if (argc < 2)
    {
        printf("缺少参数！\n");
        return -1;
    }
    path_str *head = NULL;
    if (argc == 2)
    {
        head = Find_all_file(argv[1], head);
    }
    if (argc == 3)
    {
        head = Find_file(argv[1], argv[2], head);
    }
    if (head == NULL)
    {
        perrnum("输入的目录有误或其中没有文件！\n");
        return -1;
    }
    path_str *p = head;

    while (1) //head是有保存数据的
    {
        printf("文件名：%s\t绝对路径：%s\t文件大小：", p->file_name, p->absolute_address);
        if (p->file_size->gigabyte > 0)
        {
            printf("%uGB ", p->file_size->gigabyte);
        }
        if (p->file_size->megabyte > 0)
        {
            printf("%uMB ", p->file_size->megabyte);
        }
        if (p->file_size->kilobyte > 0)
        {
            printf("%uKB ", p->file_size->kilobyte);
        }
        if (p->file_size->byte > 0)
        {
            printf("%u字节", p->file_size->byte);
        }
        printf("\n");
        p = p->next;
        if (p == head)
            break;
    }
    destroy_path_str_list(head);
    head = NULL;
} */