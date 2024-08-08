#include "pthreadpool.h"

/*
    功能：尾插法添加一个任务到任务队列
    参数：
        @pool: 线程池指针
    返回值：
        成功返回任务列表的第一个节点
        失败返回NULL
*/
static task * add_Task_At_Listtail(pthreadPool *pool, do_task func, void *arg)
{
    task * newTask = calloc(sizeof(task), 1);
    if (newTask == NULL)
    {
        return NULL;
    }
    newTask->arg = arg;
    newTask->func = func;
    newTask->next = NULL;
    // 说明任务队列中没有任务
    if (0 == pool->ListCurNum)
    {
        pool->taskList = newTask;
    }
    else
    {
        task * temp = pool->taskList;
        // 找到任务队列中最后一个节点
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = newTask;
    }
    ++pool->ListCurNum;
    return pool->taskList;
}
/*
    功能： 初始化线程池
    参数要求：
        @max : 最大线程数
        @min : 核心线程数
        @size : 任务队列最大任务数
    返回值：
        初始化成功返会 一个初始化完成的线程池
        初始化失败返回 NULL
*/
pthreadPool *init_PthreadPool(int max, int min, int size)
{
    pthreadPool *pool = NULL;
    do
    {
        pool = calloc(sizeof(pthreadPool), 1);
        if (pool == NULL)
        {
            printf("线程池内存空间分配失败！\n");
            break;
        }

        // 分配任务队列的节点内存以及初始化队列最大任务数和当前任务数
        pool->ListMaxNum = size;
        pool->ListCurNum = 0;
        pool->taskList = (task*)malloc(sizeof(task));
        if (pool->taskList == NULL)
        {
            printf("任务队列节点内存分配失败！\n");
            break;
        }

        // 分配节点内储存数据的空间
        pool->taskList->next = (task *)calloc(sizeof(task), 1);

        // 初始化锁 和 条件变量
        if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
            pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
            pthread_cond_init(&pool->listNotFull, NULL) != 0 ||
            pthread_cond_init(&pool->listNotEmpty, NULL) != 0)
        {
            printf("初始化锁或条件变量失败！\n");
            break;
        }

        pool->maxPthreadNum = max; // 最大线程数
        pool->corePthreadNum = min; // 核心线程数
        // 开辟工作线程数组空间
        pool->workPthread = calloc(sizeof(*(pool->workPthread)), max);

        // malloc(sizeof(*(pool->workPthread)) * max);
        // memset(pool->workPthread, 0 , sizeof(*(pool->workPthread)) * max);

        if (pool->workPthread == NULL)
        {
            break;
        }
        pool->busyPthreadNum = 0;     // 工作线程数
        pool->willExitPthreadNum = 0; // 需要释放的线程数
        pool->livePthreadNum = min;   // 存活线程数

        pool->shutDown = POOL_DISABLE;
        // 创建管理者线程
        pthread_create(&pool->managerPthread, NULL, (void *)manage, pool);

        // 创建工作线程
        for (int i = 0; i < pool->corePthreadNum; i++)
        {
            pthread_create(&pool->workPthread[i], NULL, (void *)work, pool);
        }

        return pool;

    } while (0);

    if (pool && pool->workPthread)
        free(pool->workPthread);
    if (pool && pool->taskList)
        free(pool->taskList);
    if (pool)
        free(pool);
    return NULL;
}

/*
    功能：添加任务到队列中
    参数：
        @pool 线程池指针
        @do_task 执行任务的函数
        @arg 执行任务的函数需要的参数
    返回值：
        成功返回0
        失败返回非0
*/
int add_Task_Tolist(pthreadPool *pool, do_task func, void *arg)
{
    pthread_mutex_lock(&pool->mutexPool);
    // if(pool->ListCurNum == pool->ListMaxNum){
    //     pthread_cond_wait(&pool->listNotFull, pool->mutexPool);
    // }

    // 使用while 防止虚假唤醒
    /*
        当线程被唤醒的时候， 会重复检查 while 内条件， 而 if 做不到
    */
    while (pool->ListCurNum == pool->ListMaxNum)
    {
        pthread_cond_wait(&pool->listNotFull, &pool->mutexPool);
    }

    // 往任务队列中添加任务
    add_Task_At_Listtail(pool, func, arg);

    // 唤醒一个线程来处理任务
    pthread_cond_signal(&pool->listNotEmpty);

    pthread_mutex_unlock(&pool->mutexPool);
    return 0;
}
/*
    功能：销毁线程池
    参数：
        @pool 线程池指针
    返回值：
        返回 0 销毁成功
        返回非0 销毁失败
*/
int destory_PthreadPool(pthreadPool *pool)
{
    if (pool == NULL)
    {
        printf("线程池为空！\n");
        return -1;
    }
    pool->shutDown = POOL_ENABLE;

    // 等待管理线程执行完毕
    pthread_join(pool->managerPthread, NULL);

    // 唤醒阻塞的任务线程
    for (int i = 0; i < pool->livePthreadNum; i++)
    {
        pthread_cond_signal(&pool->listNotEmpty);
    }

    // 释放开辟的一些堆内存
    free(pool->workPthread);
    free(pool->taskList);

    // 释放 锁和条件变量
    pthread_mutex_destroy(&pool->mutexBusy);
    pthread_mutex_destroy(&pool->mutexPool);
    pthread_cond_destroy(&pool->listNotEmpty);
    pthread_cond_destroy(&pool->listNotFull);

    free(pool);

    return 0;
}

// 管理线程函数
static void *manage(void *arg)
{
    pthreadPool *pool = (pthreadPool *)arg;
    while (!pool->shutDown)
    {
        // 每隔 2s 检测一次
        sleep(1);

        // 增加线程
        // 取出当前任务数量 和 当前活着的线程数 , 线程池最大线程数
        // 线程池是 共用资源 使用前先上锁
        pthread_mutex_lock(&pool->mutexPool);
        int taskNum = pool->ListCurNum;
        int livePthreadNum = pool->livePthreadNum;
        int maxPthreadNum = pool->maxPthreadNum;
        pthread_mutex_unlock(&pool->mutexPool);
        
        pthread_mutex_lock(&pool->mutexBusy);
        int busyPthreadNum = pool->busyPthreadNum;
        pthread_mutex_unlock(&pool->mutexBusy);

        printf("busyPthreadNum: %d taskNUM: %d livePrhreadNum: %d\n", busyPthreadNum, taskNum, livePthreadNum);

        // 如果当前任务数大于存活的线程数 同时 存活的线程数 小于 最大线程数 增加2个线程
        if (taskNum > livePthreadNum && livePthreadNum < maxPthreadNum)
        {
            printf("我进来创建新的线程了!\n");
            pthread_mutex_lock(&pool->mutexPool);
            // 遍历当前 工作线程 数组， 找到要添加的位置
            int cnt = 0;
            for (int i = 0; i < maxPthreadNum, cnt < 2; i++)
            {
                if (pool->workPthread[i] == 0)
                {
                    pthread_create(&pool->workPthread[i], NULL, work, pool);
                    pool->livePthreadNum++;
                    cnt++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }

        // 减少线程
        // 如果当前的工作线程数 少于 两倍存活线程数 则减少2个线程
        if (busyPthreadNum * 2 < livePthreadNum)
        {
            
            pthread_mutex_lock(&pool->mutexPool);
            pool->willExitPthreadNum = 2;
            // 先解锁 再进行唤醒
            pthread_mutex_unlock(&pool->mutexPool);
            for (int i = 0; i < 2; i++)
            {
                pthread_cond_signal(&pool->listNotEmpty);
                //printf("busyPthread : %d  livePthread : %d willExitPthread: %d \n", pool->busyPthreadNum, pool->livePthreadNum, pool->willExitPthreadNum);
            }
            
        }
    }
    return NULL;
}
/*
    介绍：每个工作线程创建都会运行此函数
    功能：从任务队列取出任务来执行；
            此外如果接受到结束信号变量则会自杀
    参数：
        @arg：一个线程池的指针
    返回值：
        成功返回NULL
*/
// 工作线程函数
static void *work(void *arg)
{
    pthreadPool *pool = (pthreadPool *)arg;
    // 如果没有收到shutdowm命令 则一直存活
    while (1)
    {
        pthread_mutex_lock(&pool->mutexPool);

        // 需要休眠的情况 ： 任务队列为空 且 未收到释放命令
        while (pool->ListCurNum == 0 && !pool->shutDown && pool->willExitPthreadNum == 0)
        {
            pthread_cond_wait(&pool->listNotEmpty, &pool->mutexPool);
        }
        // 收到需要 释放的命令
        while (pool->willExitPthreadNum != 0)
        {
            printf("i will dead\n");
            --pool->willExitPthreadNum;
            --pool->livePthreadNum;
            pthread_mutex_unlock(&pool->mutexPool);
            exit_CurPthread(pool);
        }
        if (pool->shutDown)
        {
            pthread_mutex_unlock(&pool->mutexPool);
            exit_CurPthread(pool);
        }
        // 休眠结束 分配任务
        --pool->ListCurNum;
        // 从任务列表头 取出一个任务
        do_task func = pool->taskList->func;
        void *arg = pool->taskList->arg;
        task *temp = pool->taskList;
        pool->taskList = pool->taskList->next;
        temp->next = NULL;
        free(temp);
        temp = NULL;
        // 尝试唤醒 任务添加线程
        pthread_cond_signal(&pool->listNotFull);
        pthread_mutex_unlock(&pool->mutexPool);
        // 开始工作
        pthread_mutex_lock(&pool->mutexBusy);
        ++pool->busyPthreadNum;
        pthread_mutex_unlock(&pool->mutexBusy);
        func(arg);
        // 结束工作
        pthread_mutex_lock(&pool->mutexBusy);
        --pool->busyPthreadNum;
        pthread_mutex_unlock(&pool->mutexBusy);
    }
}

/*
    功能：退出对应的线程
    参数：
        @pool 线程池
    返回值：
        成功返回0
        失败返回非0
*/
static int exit_CurPthread(pthreadPool *pool)
{
    pthread_mutex_lock(&pool->mutexPool);
    pthread_t tid = pthread_self();
    for (int i = 0; i < pool->maxPthreadNum; i++)
    {
        if (tid == pool->workPthread[i])
        {
            pool->workPthread[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&pool->mutexPool);
    pthread_exit(NULL);
    return 0;
}