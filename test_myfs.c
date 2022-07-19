#include "myfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
    mymkfs(1000000);
    int fd = myopen("/file1.txt", O_RDWR);
    mywrite(fd, "hello world", sizeof("hello world"));
    myclose(fd);
    fd = myopen("/file1.txt", O_RDONLY);
    char* buf = (char*)calloc(1024, sizeof(char));
    myread(fd, buf, 10);
    printf("the file said: %s\n", buf);
    mylseek(fd, 6, 0);
    myread(fd, buf, 5);
    printf("the file said: %s\n", buf);
    myclose(fd);
    fd = myopen("/file2.txt", O_RDWR);
    myclose(fd);
    fd = myopen("/file3.txt", O_RDWR);
    myclose(fd);
    myDIR *dir = myopendir("/");
    struct mydirent *dirent = myreaddir(dir);
    printf("directory '/' contains the following files:\n");
    while(dirent != NULL){
        printf("%s\n",dirent->inode->path);
        dirent = myreaddir(dir);
    }
    return 0;
}