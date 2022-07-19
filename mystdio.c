#include "mystdio.h"
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

myFILE *myfopen(const char *pathname, const char *mode){
    myFILE* file = (myFILE*)malloc(sizeof(myFILE));
    if(strcmp(mode, "r") == 0){
        file->fd = myopen(pathname, O_RDONLY);
    }else if(strcmp(mode, "r+") == 0){
        file->fd = myopen(pathname, O_RDWR);
    }else if(strcmp(mode, "w") == 0){
        file->fd = myopen(pathname, O_WRONLY);
    }else if(strcmp(mode, "a") == 0){
        file->fd = myopen(pathname, O_APPEND);
    }
    memset(file->mode,0,3);
    strcpy(file->mode, mode);
    return file;
}

int myfclose(myFILE *stream){
    memset(stream->mode,0,3);
    strcpy(stream->mode, "c");
    myclose(stream->fd);
}

size_t myfread(void *ptr, size_t size, size_t nmemb, myFILE *stream){
    if(strcmp(stream->mode, "r")!=0 && strcmp(stream->mode, "r+") != 0){
        printf("not on read mode\n");
        return -1;
    }
    size_t bytes_read = myread(stream->fd, ptr, size*nmemb);
    return bytes_read;
}

size_t myfwrite(const void *ptr, size_t size, size_t nmemb, myFILE *stream){
    if(strcmp(stream->mode, "r")==0 && strcmp(stream->mode, "c") == 0){
        printf("not on write mode\n");
        return -1;
    }
    size_t bytes_written = mywrite(stream->fd, ptr, size*nmemb);
    return bytes_written;
}

int myfseek(myFILE *stream, long offset, int whence){
    if(strcmp(stream->mode, "c") == 0){
        printf("file is closed\n");
        return -1;
    }
    int off_set = mylseek(stream->fd, offset, whence);
    return off_set;
}

int myfscanf(myFILE *stream, const char *format, ...){
    if(strcmp(stream->mode, "r")!=0 && strcmp(stream->mode, "r+") != 0){
        printf("not on read mode\n");
        return -1;
    }
    char* buf;
    memset(buf, 0, sizeof(format));
    myfread(buf, 1, sizeof(format), stream);
    int i=0;
    int count = 0;
    float temp = 0;
    char* itr = format;
    va_list arg;
	va_start(arg, format);
    while(*itr != '\0')
	{
		if(*itr == '%')
		{
			switch(*(++itr))
            {
				case 'd':
                    temp =0;
					while(buf[i]!= ' '){
                        temp *=10;
                        temp += (float)(buf[i]-'0');
                        i++;
                    }
                    *(int*)arg = (int)temp;
                    i++;
					break;
				case 'c':
					*(char*)arg = buf[i];
                    i++;
					break;
                case 'f':
                    temp = 0;
                    while(buf[i]!= ' ' && buf[i] != '.'){
                        temp *=10;
                        temp += buf[i]-'0';
                        i++;
                    }
                    while(buf[i]!= ' '){
                        temp+=((float)(buf[i]-'0'))/pow(10, count);
                        count++;
                        i++;
                    }
                    *(float*)arg = temp;
			}
		}
		++itr;
	}
    return 0;
}

int myfprintf(myFILE *stream, const char *format, ...){
    if(strcmp(stream->mode, "r")==0 && strcmp(stream->mode, "c") == 0){
        printf("not on write mode\n");
        return -1;
    }
    char* buf;
    char* curr;
    memset(buf, 0, sizeof(format));
    char* itr = format;
    va_list arg;
	va_start(arg, format);

    while(*itr != '\0')
	{
		if(*itr == '%')
		{
			switch(*(++itr))
            {
				case 'd':
                    memset(curr, 0, sizeof(format));
                    sprintf(curr, "%d", va_arg(arg,int));
					strcat(buf, curr);
                    strcat(buf, " ");
					break;
				case 'c':
					strncat(buf, (char)va_arg(arg, char), 1);
                    strcat(buf, " ");
					break;
                case 'f':
                    memset(curr, 0, sizeof(format));
                    sprintf(curr, "%f", va_arg(arg,float));
					strcat(buf, curr);
                    strcat(buf, " ");
			}
		}
		++itr;
	}
    myfwrite(buf, 1, sizeof(buf), stream);
}