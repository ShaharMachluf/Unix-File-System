#include "mystdio.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    mymkfs(1000000);
    myFILE* fd = myfopen("/file1.txt", "r+");
    myfwrite("hello world", sizeof("hello world"), 1, fd);
    myfclose(fd);
    fd = myfopen("/file1.txt", "r");
    char* buf = (char*)calloc(1024, sizeof(char));
    myfread(buf, 10, 1, fd);
    printf("the file said: %s\n", buf);
    myfseek(fd, 6, 0);
    myfread(buf, 5, 1, fd);
    printf("the file said: %s\n", buf);
    myfclose(fd);
    return 0;
}