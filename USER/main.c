#include "pthreadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "copy.h"

int main()
{
	// 1. 初始化线程池
	pthreadPool *pool = init_PthreadPool(100, 40, 100);
	// 2. 创建任务 -- 任务函数任务数据
	char *srcPath = "../srcDir";
	char *destPath = "../destDir";
	cp_dir(srcPath, destPath, pool);
	// 3. 等待任务结束
	while (1)
	{
		pthread_mutex_lock(&pool->mutexBusy);
		int busyPthreadNum = pool->busyPthreadNum;
		pthread_mutex_unlock(&pool->mutexBusy);
		if (busyPthreadNum == 0)
		{
			destory_PthreadPool(pool);
			break;
		}
	}
	// 4. 销毁线程池

	return 0;
}
