#最终的目标名
TARGET := ../BIN/main

#指定一个编译器
CC := gcc -pthread

#找到我们的源文件夹 .c文件
#在当前路径下面找所有的.c文件
#CFILES := $(wildcard ../CODE/SRC/*.c ./*.c)
CFILES := $(wildcard ../CODE/SRC/*.c)
CFILES += $(wildcard ./*.c)

#去掉路径
CFILES := $(notdir $(CFILES))


#中间文件的名字  做模式替换
#将所有的.c模式替换为.o
#到这里为止 没有.o文件生成
OBJS := $(patsubst %.c,../OBJ/%.o,$(CFILES))

#头文件搜索路径  如果你的头文件不是放在这些路径下面的 请添加进来
#建议用这些
INCPATH := -I../CODE/INC
INCPATH += -I./
INCPATH += -I../LIB/inc

#库的搜索路径
LIBPATH := -L../LIB/lib

#库的名字
LIBFILES := 


#最终的可执行文件依赖.o文件
#将所有的.o文件进行链接
$(TARGET) : $(OBJS)	
	$(CC)  $^ -o $@ $(INCPATH) $(LIBPATH) $(LIBFILES)


#上面的OBJS只有名字没有文件  是不是要生成
../OBJ/%.o : ../CODE/SRC/%.c
	$(CC) -c $< -o $@ $(INCPATH) $(LIBPATH) $(LIBFILES)


../OBJ/%.o : ./%.c
	$(CC) -c $< -o $@ $(INCPATH) $(LIBPATH) $(LIBFILES)

print:
	@echo $(CFILES)
	@echo -------
	@echo $(OBJS)


#加标记 你make clean就来这里执行
clean:
	rm -rf $(OBJS) $(TARGET)






