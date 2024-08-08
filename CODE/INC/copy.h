#ifndef _COPY_H_
#define _CPOY_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "pthreadpool.h"

typedef struct information
{
    char srcName[256];
    char destName[256];
} information;

void *cp_file(void *arg);
int cp_dir(char *srcPath, char *destPath, pthreadPool * pool);

#endif