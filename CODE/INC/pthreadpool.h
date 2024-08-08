#ifndef _PTHREADPOOL_H_
#define _PTHREADPOOL_H_

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define POOL_DISABLE 0;
#define POOL_ENABLE 1;

// 定义一个任务函数的指针
typedef void *(*do_task)(void *arg);

// 定义一个任务队列的结构体
typedef struct task
{
    do_task func;          // 保存任务所需要执行的函数
    void *arg;             // 任务执行需要的参数
    struct task *next; // 下一个任务
} task;

// 定义一个线程池的结构体
typedef struct pthreadPool
{
    task *taskList;              // 定义一个任务队列的节点
    int ListMaxNum;              // 任务队列最大任务数量
    int ListCurNum;              // 当前任务队列中任务个数
    pthread_t managerPthread;    // 管理者线程 id;
    pthread_t *workPthread;      // 工作线程 id 数组 ( 因数量未知需要动态分配， 即使用指针，使用完记得free )
    int maxPthreadNum;           // 保存的最大的线程数
    int corePthreadNum;          // 核心线程数(最小线程数)
    int busyPthreadNum;          // 正在工作的线程数
    int livePthreadNum;          // 当前存活的线程数
    int willExitPthreadNum;      // 需要释放的线程数
    pthread_mutex_t mutexPool;   // 整个线程池的线程锁
    pthread_mutex_t mutexBusy;   // 正在工作线程的线程锁
    int shutDown;                // 线程池是否关闭
    pthread_cond_t listNotFull;  // 任务队列未满
    pthread_cond_t listNotEmpty; // 任务队列非空

} pthreadPool;

// 添加任务节点到 任务队列
static task *add_Task_At_Listtail(pthreadPool *pool, do_task func, void *arg);
// 初始化 线程池
pthreadPool *init_PthreadPool(int max, int min, int size);
// 分配任务
int add_Task_Tolist(pthreadPool *pool, do_task func, void *arg);
// 销毁线程池
int destory_PthreadPool(pthreadPool *pool);
// 管理线程函数
static void *manage(void *arg);
// 工作线程函数
static void *work(void *arg);
// 线程退出函数
static int exit_CurPthread(pthreadPool *pool);

#endif