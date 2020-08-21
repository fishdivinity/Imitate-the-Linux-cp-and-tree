#include "currency.h"

#define DEBUG printf("%d\n", __LINE__);

int main(int32_t argc, int8_t *argv[])
{
    time_t second = time(NULL);

    int32_t choose = 0;
    int8_t *original_address = NULL;
    int8_t *destination_address = NULL;
    int8_t *file_suffix = NULL;
    path_str *file_head = NULL;
    pthread_work_t *work_head = NULL;
    file_size *all_size = NULL;

    if (argc < 2)
    {
        printf("缺少参数！\n");
        printf("请输入要选择的功能！\n");
        printf("1 .打印目录\n2 .拷贝文件\n3 .拷贝指定文件\n");
        scanf("%d", &choose);
        system("clear");
    }
    if (argc > 1)
    {
        original_address = calloc(1, strlen(argv[1]) + 2);
        strcpy(original_address, argv[1]);
        if (original_address[strlen(original_address) - 1] != '/') //如果目录没有加/，加上
            strcat(original_address, "/");
    }
    if (argc > 2)
    {
        destination_address = calloc(1, strlen(argv[2]) + 2);
        strcpy(destination_address, argv[2]);
        if (destination_address[strlen(destination_address) - 1] != '/')
            strcat(destination_address, "/");
    }
    if (argc > 3)
    {
        file_suffix = calloc(1, strlen(argv[3]) + 1);
        strcpy(file_suffix, argv[3]);
    }

    if (argc == 2 || choose == 1)
    {
        if (original_address == NULL)
        {
            printf("请输入目标地址！\n");
            original_address = calloc(1, 255);
            scanf("%s", original_address);
            original_address = realloc(original_address, strlen(original_address) + 2);
            if (original_address[strlen(original_address) - 1] != '/')
                strcat(original_address, "/");
        }

        tree_printing_directory(original_address, 0);
        free(original_address);
        free(destination_address);
    }
    else if (argc == 3 || choose == 2)
    {
        if (original_address == NULL)
        {
            printf("请输入原地址！\n");
            original_address = calloc(1, 255);
            scanf("%s", original_address);
            original_address = realloc(original_address, strlen(original_address) + 2);
            if (original_address[strlen(original_address) - 1] != '/')
                strcat(original_address, "/");
        }
        if (destination_address == NULL)
        {
            printf("请输入目标地址！\n");
            destination_address = calloc(1, 255);
            scanf("%s", destination_address);
            destination_address = realloc(destination_address, strlen(destination_address) + 2);
            if (destination_address[strlen(destination_address) - 1] != '/')
                strcat(destination_address, "/");
        }
        file_head = Find_all_file(original_address, file_head);

        if (NULL == file_head)
        {
            printf("目录地址错误或者目录里没有文件！\n");
            free(original_address);
            free(destination_address);
            return -1;
        }

        all_size = Conversion_add(file_head);
        path_str *p = file_head;

        while (1)
        {
            work_head = pthread_add_wait_queue(work_head, (void *)copy, (void *)copy_pretreatment(p, original_address, destination_address));
            if (p->next == file_head)
                break;
            p = p->next;
        }
        pthread_pool_t *pool = pthread_pool_init();
        sleep(1);
        system("stty -echo");
        current_wait_num(work_head, pool);
        while (1)
        {
            if (pool->Current_wait_pthread_num == 0)
            {
                destroy_pthread_pool(pool);
                destroy_path_str_list(file_head);
                system("stty echo");
                break;
            }
        }
    }
    else if (argc == 4 || choose == 3)
    {
        if (original_address == NULL)
        {
            printf("请输入原地址！\n");
            original_address = calloc(1, 255);
            scanf("%s", original_address);
            original_address = realloc(original_address, strlen(original_address) + 2);
            if (original_address[strlen(original_address) - 1] != '/')
                strcat(original_address, "/");
        }
        if (destination_address == NULL)
        {
            printf("请输入目标地址！\n");
            destination_address = calloc(1, 255);
            scanf("%s", destination_address);
            destination_address = realloc(destination_address, strlen(destination_address) + 2);
            if (destination_address[strlen(destination_address) - 1] != '/')
                strcat(destination_address, "/");
        }
        if (file_suffix == NULL)
        {
            printf("请输入文件后缀名！\n");
            file_suffix = calloc(1, 30);
            scanf("%s", file_suffix);
            file_suffix = realloc(file_suffix, strlen(file_suffix) + 1);
        }
        file_head = Find_file(original_address, file_suffix, file_head);

        if (NULL == file_head)
        {
            printf("目录地址错误或者目录里没有文件！\n");
            free(original_address);
            free(destination_address);
            return -1;
        }

        all_size = Conversion_add(file_head);
        path_str *p = file_head;

        while (1)
        {
            work_head = pthread_add_wait_queue(work_head, (void *)copy, (void *)copy_pretreatment(p, original_address, destination_address));
            if (p->next == file_head)
                break;
            p = p->next;
        }
        pthread_pool_t *pool = pthread_pool_init();
        sleep(1);
        system("stty -echo");
        current_wait_num(work_head, pool);
        while (1)
        {
            if (pool->Current_wait_pthread_num == 0)
            {
                destroy_pthread_pool(pool);
                destroy_path_str_list(file_head);
                system("stty echo");
                break;
            }
        }
    }
    else
    {
        printf("参数错误！\n");
        if (original_address != NULL)
            free(original_address);
        if (destination_address != NULL)
            free(destination_address);
        if (file_suffix != NULL)
            free(file_suffix);
        return -1;
    }

    time_t now_second = time(NULL);
    uint64_t system_second = now_second - second;
    
    if ((argc != 2 && argc != 1) || (choose != 1 && choose != 0))
    {
        printf("总文件大小：");

        if (all_size->gigabyte > 0)
            printf("%uGb", all_size->gigabyte);
        if (all_size->megabyte > 0)
            printf("%uMb", all_size->megabyte);
        if (all_size->kilobyte > 0)
            printf("%uKb", all_size->kilobyte);
        if (all_size->byte > 0)
            printf("%u字节", all_size->byte);
        printf("\t");
        printf("程序运行了%ld秒！\n", system_second);
    }
    return 0;
}