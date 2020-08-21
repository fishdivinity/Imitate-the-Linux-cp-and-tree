/* File encoding format:UTF-8  */
#include "currency.h"
#include <errno.h>

/* 
typedef struct
{
    int8_t *file_name;             //文件名
    int8_t *file_original_address; //文件原地址
    int8_t *original_address;      //目录原地址
    int8_t *destination_address;   //目标地址
    file_size *file_size;          //文件大小
} copy_str; */

void *copy(copy_str *copy_arg)
{
    int32_t size = 1024; //单次读取的大小
    int8_t buf[size];
    int8_t temp_address[255]; //临时的拼接地址用的数组
    bzero(temp_address, 255);
    int8_t temp_destination[255]; //第二个临时的保存的拼接地址用的数组
    bzero(temp_destination, 255);
    sprintf(temp_address, "%s%s", copy_arg->file_original_address, copy_arg->file_name);
    int32_t ofd = open(temp_address, O_RDWR); //打开原目录文件

    if (ofd == -1)
    {
        chmod(temp_address, 0777);
        ofd = open(temp_address, O_RDWR);
        if (-1 == ofd)
        {
            perror("文件打开错误！\n");
            return NULL;
        }
    }
    bzero(temp_address, 255);

    if (strlen(copy_arg->file_original_address) > strlen(copy_arg->original_address))
    {
        sprintf(temp_address, "%s%s", copy_arg->destination_address, &(copy_arg->file_original_address[strlen(copy_arg->original_address)]));
        sprintf(temp_destination, "%s%s", temp_address, copy_arg->file_name);
    }
    else
    {
        strcpy(temp_address, copy_arg->destination_address);
        sprintf(temp_destination, "%s%s", copy_arg->destination_address, copy_arg->file_name);
    }

    int32_t dfd = open(temp_destination, O_CREAT | O_TRUNC | O_RDWR, 0777);

    if (-1 == dfd)
    {

        while (1)
        {
            dfd = open(temp_destination, O_CREAT | O_TRUNC | O_RDWR, 0777);
            if (-1 == dfd)
            {
                if (mkdir(temp_address, 0777) == -1)
                {
                    if (errno == EEXIST)
                        ;
                    else
                    {
                        int8_t temp[strlen(temp_address) + 1];
                        bzero(temp, strlen(temp_address) + 1);
                        int8_t temp1[20];
                        bzero(temp1, 20);
                        int8_t *temp2 = strtok(temp_address, "/");
                        sprintf(temp, "/%s/", temp2);
                        while (1)
                        {
                            if (strlen(temp1) != 0)
                            {
                                strcat(temp, temp1);
                                strcat(temp, "/");
                                mkdir(temp,0777);
                                bzero(temp1, 20);
                            }
                            if ((temp2 = strtok(NULL, "/") )== NULL)
                                break;
                            strcpy(temp1, temp2);
                        }
                        mkdir(temp,0777);
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
    while (1)
    {
        bzero(buf, size);
        int32_t temp = read(ofd, buf, size);
        if (temp == 0)
            break;
        write(dfd, buf, temp);
    }
    close(ofd);
    close(dfd);
    file_size *file_size = copy_arg->file_size;
    free(copy_arg->original_address);
    free(copy_arg->destination_address);
    free(copy_arg);
    return (void *)file_size;
}

copy_str *copy_pretreatment(path_str *file_info, const int8_t *original_address, const int8_t *destination_address) //copy前的处理，注意，里面直接获取file_info的一些内容的地址，在free的时候小心点
{
    copy_str *copy_arg = calloc(1, sizeof(copy_str));
    copy_arg->file_size = file_info->file_size;
    copy_arg->file_original_address = file_info->absolute_address;
    copy_arg->file_name = file_info->file_name;
    copy_arg->destination_address = calloc(1, strlen(destination_address) + 1);
    strcpy(copy_arg->destination_address, destination_address);
    copy_arg->original_address = calloc(1, strlen(original_address) + 1);
    strcpy(copy_arg->original_address, original_address);
    return copy_arg;
}