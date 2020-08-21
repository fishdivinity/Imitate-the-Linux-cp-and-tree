# Linux环境下的cp程序  
## dir.c  
- `int32_t Conversion(uint64_t byte, file_size *size)`  
#### **此函数用于将读取到的文件字节数转换成`file_size`结构体来存储已经包含在`Find_all_file`和`Find_file`函数中**  
`typedef struct`  
`{`  
&emsp;`uint16_t byte; &emsp;`&emsp;&emsp;&nbsp;//存放字节  
&emsp;`uint16_t kilobyte;` &emsp;&emsp;&emsp;&emsp;//存放千字节  
&emsp;`uint16_t megabyte;` &emsp;&emsp;&emsp;&nbsp;//存放兆字节  
&emsp;`uint16_t gigabyte;` &emsp;&emsp;&emsp;&nbsp;&nbsp;&nbsp;&nbsp;//存放吉字节  
`} file_size;`  
&emsp;  

- `file_size *Conversion_add(path_str *head)`  
#### **此函数用于将读取到的`file_size`结构体加起来，成功返回加起来的`file_size`指针地址**  
- `file_size *Conversion_minus(file_size *all_size, file_size *minus_size)`  
#### **此函数用于将返回的`file_size`结构体减去总的，成功返回减去的`file_size`指针地址**  
- `path_str *Find_all_file(const int8_t *absolute_address, path_str *head)`
#### **找到目录下的所有文件，成功返回`path_str`结构体的头节点地址，返回NULL为错误，可能是地址不对，或者目录里没有文件**  
- `path_str *Find_file(const int8_t *absolute_address, const int8_t *file_suffix, path_str *head)`
#### **找到目录下的所有后缀相同的文件，成功返回`path_str`结构体的头节点地址，返回NULL为错误，可能是地址不对，或者目录里没有指定的文件**  
`typedef struct path_str`  
`{`  
&emsp;`int8_t *file_name`;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;//文件名  
&emsp;`int8_t *absolute_address;`&emsp;&emsp;&nbsp;//绝对地址  
&emsp;`file_size *file_size;`&emsp;&emsp;&emsp;&emsp;&emsp;&nbsp;&nbsp;&nbsp;//文件大小  
&emsp;`struct path_str *prev;`&emsp;&emsp;&emsp;&emsp;&nbsp;//上一个结构体指针  
&emsp;`struct path_str *next;`&emsp;&emsp;&emsp;&emsp;&nbsp;//下一个结构体指针  
`} path_str;`  
&emsp;  
  
- `path_str *Linked_list(path_str *head, path_str *new)`
#### **把`path_str`结构体链表连接起来，需要把`path_str`头节点传进去，返回的是`path_str`头节点**
- `int32_t destroy_path_str_list(path_str *head)`
#### **摧毁`path_str`结构体链表，在程序执行结束之后，必须要执行此代码，把堆空间释放掉，但是如果重复释放会出现段错误，使用时要注意**
- `int32_t tree_printing_directory(const int8_t *absolute_address, int16_t loop)`
#### **以树状图的方式打印目录内的所有文件与子目录,成功返回0，失败返回-1**
## copy.c
- `copy_str *copy_pretreatment(path_str *file_info, const int8_t *original_address, const int8_t *destination_address)`
#### **copy主函数运行前的预处理，职责是将`path_str`结构体中的`int8_t *file_name`、`int8_t *absolute_address`和`file_size *file_size`三个地址，还有copy的原地址和copy的目标地址保存到`copy_str`结构体中，因为copy为适应回调函数，只能传入一个指针地址**  
`typedef struct`  
`{`  
&emsp;`int8_t *file_name;`&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;//文件名  
&emsp;`int8_t *file_original_address;`&emsp;&nbsp;&nbsp;&nbsp;&nbsp;//文件原地址  
&emsp;`int8_t *original_address;`&emsp;&emsp;&emsp;&nbsp;&nbsp;&nbsp;&nbsp;//目录原地址  
&emsp;`int8_t *destination_address;`&emsp;&emsp;&nbsp;//目标地址  
&emsp;`file_size *file_size;`&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;//文件大小  
`} copy_str;`  
&emsp;  

- `void *copy(copy_str *copy_arg)`
#### **copy的主函数，可以直接采用回调函数来使用，复制的过程中，会在复制文件前先把目录创建出来，如果文件的权限或者文件格式发生损坏，会出现打开文件失败的情况，当然原地址或者目标地址错误也会导致copy失败**
## thread_pool.c
### 注意这里只会说一些可以调用的函数，`start_routine`和`add_pthread`函数不可以调用，所以不介绍，此线程池初始化之后默认为创建10个进程，如果任务数量少于10个，会销毁多出来的，默认最大的进程上限为100，如果需要，可以在`thread_pool.c`中修改`PTHREAD_MAX`，初始化结束之后，建议延时1秒，线程创建的速度可能慢于主函数执行下一个代码的速度，会照成段错误  
- `pthread_pool_t *pthread_pool_init(void)`
#### **初始化线程池，初始化成功会返回`pthread_pool_t`结构体地址，失败返回`NULL`**  
`typedef struct`  
`{`  
&emsp;/&nbsp;* 互斥锁 *&nbsp;/  
&emsp;`pthread_mutex_t *queue_mutex;`  
&emsp;/&nbsp;* 条件变量 *&nbsp;/  
&emsp;`pthread_cond_t *queue_cond;`  
&emsp;/&nbsp;* 要等待的线程队列链表头 *&nbsp;/  
&emsp;`pthread_work_t *queue_head;`  
&emsp;/&nbsp;* 是否销毁进程池，是1，否0 *&nbsp;/  
&emsp;`uint16_t destruction;`  
&emsp;/&nbsp;* 线程id *&nbsp;/  
&emsp;`pthread_t *pthread_id;`  
&emsp;/&nbsp;* 线程池最大的线程数 *&nbsp;/  
&emsp;`uint32_t Max_pthread_num;`
&emsp;/&nbsp;* 当前正在等待的线程数 *&nbsp;/  
&emsp;`int32_t Current_wait_pthread_num;`  
`} pthread_pool_t;`  
&emsp;  

- `pthread_work_t *pthread_add_wait_queue(pthread_work_t *head, void *(*task_function)(void *arg), void *arg)`
#### **将函数和要传入函数的信息进行处理，转变成`pthread_work_t`结构体，并链接起来，返回`pthread_work_t`的头节点地址，注意参数必须传入`pthread_work_t`的头节点，如果没有头节点，可以传入`NULL`**
`typedef struct worker`  
`{`  
&emsp;`void *(*worker)(void *arg);`  
&emsp;`void *arg;`  
&emsp;`struct worker *prev;`  
&emsp;`struct worker *next;`  
`} pthread_work_t;`  
&emsp;  

- `void current_wait_num(pthread_work_t *head, pthread_pool_t *pthread_pool)`
#### **检索加进来的任务链表总数,并调用`add_pthread`函数，自动重新调整线程池的大小**
- `int32_t destroy_pthread_pool(pthread_pool_t *pthread_pool)`
#### **唤醒所有线程，并摧毁线程池，成功返回0，失败返回-1** 
## perrnum.c
### 这个函数定义了一个`errnum`的全局变量，其他文件都可以调用，在`currency.h`中宏定义了相关的错误参数，可以直接调用perrnum把错误原因打印出来问题
- `void perrnum(const char *s)`  
#### **相当于perrno函数，可以当作printf使用
## rrand.c
- `int32_t rrand(int32_t limit)`
#### **随机数函数，默认的`SRAND_SEED`是`/dev/random`这个字符设备文件，如果要修改，可以在`currency.h`里重新宏定义，`int32_t limit`是返回值的最大上限，返回的值是在0~limit中的随机数**
## main.c
#### **`main.c`的主要内容都是一些判断类的代码，判断用户是否输入错误、选择的功能等，还有一个打印总大小和打印总使用时间的简单代码功能**  