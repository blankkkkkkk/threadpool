#include "copy.h"

void *cp_file(void *arg)
{
    information *info = (information *)arg;
    char buf[1024] = {0};
    char newfile[1024] = {0};
    sprintf(newfile, "%s/%s", info->destName, basename(info->srcName));
    int fd_src = open(info->srcName, O_RDONLY);
    if (fd_src == -1)
    {
        perror("open srcname failed");
        return NULL;
    }
    int r = 1;
    int fd_dest = open(newfile, O_CREAT | O_RDWR | O_TRUNC, 0777);
    if (fd_dest == -1)
    {
        perror("open destname failed");
        return NULL;
    }
    while (1)
    {
        r = read(fd_src, buf, 1024);
        if (r == 0)
            break;
        write(fd_dest, buf, r);
    }
    close(fd_src);  // 关闭源文件描述符
    close(fd_dest); // 关闭目标文件描述符
    free(info);     // 释放分配的内存
    return NULL;
}

// 处理目录函数
int cp_dir(char *srcPath, char *destPath, pthreadPool * pool)
{
    // 把 源文件目录 拼接到 目标目录
    char newDestPath[256] = {0};
    sprintf(newDestPath, "%s/%s", destPath, basename(srcPath));
    DIR *srcFile;
    struct dirent *srcEntry;
    srcFile = opendir(srcPath);
    if (srcFile == NULL)
    {
        perror("opendir failed");
        return -1;
    }
    mkdir(newDestPath, 0777);

    while (srcEntry = readdir(srcFile))
    {
        if (strcmp(srcEntry->d_name, ".") && strcmp(srcEntry->d_name, ".."))
        {
            char subSrcPath[512];
            sprintf(subSrcPath, "%s/%s", srcPath, srcEntry->d_name);
            if (srcEntry->d_type == DT_REG)
            {
                // 普通文件
                // 创建一个新线程去处理普通文件
                information *info = calloc(sizeof(information), 1);
                strcpy(info->srcName, subSrcPath);
                strcpy(info->destName, newDestPath);
                add_Task_Tolist(pool, cp_file, (void *)info);
            }
            else if (srcEntry->d_type == DT_DIR)
            {
                cp_dir(subSrcPath, newDestPath, pool);
            }
        }
    }
    closedir(srcFile);
    return 0;
}