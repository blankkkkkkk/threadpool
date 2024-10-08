# 简易线程池

一个轻量级的线程池实现，用于简化多线程编程中的线程管理，可以根据任务队列中任务数量自动增加或减少线程数，合理化了系统资源利用率，提高了程序的性能，提供了一个高效，可控的线程执行环境，使得多线程应用能够更加稳定和高效地运行。

##	 特性

- 支持动态线程数的调整
- 任务队列管理
- 线程安全 
- 易于集成和使用

## 安装

确保您的系统环境支持 C/C++ 编程以及线程操作

####  导入

在您的项目头文件目录下导入 pthreadpool.h

资源目录导入pthreadpool.c 即可通过函数使用

## 使用说明

以下是如何使用本线程池的基本步骤：

### 初始化线程池

在使用线程池之前，你需要初始化它。

```c
/*
	参数说明：
	pthreadPool * init_PthreadPool(int max, int min, int size);
		@max : 线程池支持同时运行的最大线程数
		@min : 线程池中常驻的线程数(核心线程数)
		@size : 任务队列的大小
*/ 
pthreadPool * pool = init_PthreadPool(10, 5, 10); // 创建一个最大线程数为10， 核心线程数为5， 任务队列最大数为10的线程池
```

### 添加任务到线程池

添加任务到线程池以执行。

``` c
/*
	参数说明：
	int add_Task_Tolist(pthreadPool *pool, do_task func, void *arg);
		@pool : 指向线程池的指针
		@do_task ：执行任务的函数
		@arg ： 执行任务函数的参数
*/
add_Task_Tolist(pool, func, arg);
```

### 销毁线程池

当不再需要线程池时，应该销毁它释放资源。

```c
/*
    参数说明：
        @pool 线程池指针
*/
int destory_PthreadPool(pool);
```

## 贡献指南

欢迎贡献代码！请遵循以下步骤：

1. Fork 本仓库
2. 创建您的特性分支 (`git checkout -b my-new-feature`)
3. 提交您的改动 (`git commit -am 'Add some feature'`)
4. 将您的改动推送到分支 (`git push origin my-new-feature`)
5. 创建新的 Pull Request

## 测试

运行以下命令执行测试：

```bash
make
../BIN/main
```

## 作者

- blank    ---   李濠锋

## 许可证

本项目使用 [MIT License](https://chatglm.cn/main/LICENSE)。

## 版本历史

- v1.0.0 - 初始版本

## 联系方式

如果有任何问题或者建议，请通过邮箱 lhf.blank@gmail.com 或者 2538060879@qq.com 联系。
