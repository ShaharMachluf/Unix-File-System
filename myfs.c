#include "myfs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct myfs* myFS;
myDIR* dirs[MAX_FILES];
struct myopenfile *open_files;

int empty_inode(){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    iNode* nodes = myFS->iNodes;
    if(myFS->superBlock->unused_iNodes == 0 || myFS->superBlock->unused_blocks == 0){
        printf("no available space");
        return -1;
    }
    for(int i=0; i<myFS->superBlock->total_iNodes; i++){//file does not exist yet
        if((nodes+i)->path == NULL){
            return i;
        }
    }
    return 1;
}

void mymkfs(long s){
    myFS = (struct myfs*)malloc(sizeof(struct myfs));
    myFS->superBlock = (sb*)malloc(sizeof(sb));
    myFS->iNodes = (iNode*)calloc(sizeof(iNode), (long)((10*s)/100));
    myFS->superBlock->total_blocks = (long)((s-((10*s)/100))/sizeof(block));
    myFS->superBlock->total_iNodes = (long)(((10*s)/100)/sizeof(iNode));
    myFS->superBlock->used_blocks = 0;
    myFS->superBlock->used_iNodes = 0;
    myFS->superBlock->unused_blocks = myFS->superBlock->total_blocks;
    myFS->superBlock->unused_iNodes = myFS->superBlock->total_iNodes;
    memset(dirs, 0, MAX_FILES);
    open_files = (struct myopenfile*)malloc(sizeof(struct myopenfile));
    open_files->openfiles = (int*)calloc(MAX_FILES, sizeof(int));
    FILE *fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    dirs[0] = myopendir("/");
}

int mymount(const char *source, const char *target,
 const char *filesystemtype, unsigned long
mountflags, const void *data){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    int fd = empty_inode();
    //add inode to the new directory
    strcpy(myFS->iNodes[fd].path, source);
    myFS->iNodes[fd].b = (block*)calloc(1, sizeof(block));
    myFS->iNodes[fd].b->next = NULL;
    myFS->superBlock->used_blocks++;
    myFS->superBlock->unused_blocks--;
    memset(myFS->iNodes[fd].b->data, 0, 1024);
    myFS->iNodes[fd].ptr = 0;
    myFS->iNodes[fd].state = CLOSED;
    myFS->superBlock->unused_iNodes--;
    myFS->superBlock->used_iNodes++;
    //add the new directory to target
    for(int i=0; i<MAX_FILES; i++){
        if(strcmp(dirs[i]->path, target) == 0){
            dirs[i]->files[dirs[i]->size] = (char*)calloc(sizeof(source), sizeof(char));
            strcpy(dirs[i]->files[dirs[i]->size], source);
            dirs[i]->indexes[dirs[i]->size] = fd;
            dirs[i]->size++;
            
        }
    }
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    return fd;
}

int myopen(const char *pathname, int flags){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    iNode* nodes = myFS->iNodes;
    char *token;
    char* dir = (char*)calloc(1, sizeof(pathname));
    char temp[1024];  
    int fd = 0;
    const char s[2] = "/";
    for(int i=0; i<myFS->superBlock->total_iNodes; i++){
        if(myFS->iNodes[i].path != NULL && strcmp(myFS->iNodes[i].path, pathname) == 0){
            myFS->iNodes[i].state = flags;
            myFS->iNodes[i].ptr = 0;
            open_files->openfiles[i] = 1;
            fs = fopen("fs.txt", "w");
            fwrite(myFS, 1, sizeof(struct myfs), fs);
            fclose(fs);
            return i;
        }
    }
    fd = empty_inode();
    if(fd == -1){
        return -1;
    }
    myFS->superBlock->unused_iNodes--;
    myFS->superBlock->used_iNodes++;
    /* get the first token */
   token = strtok(pathname, s);
   strcat(dir, s);

   if(token != NULL){
        memset(temp, 0 ,1024);
        strcpy(temp , token);
    
        token = strtok(NULL, s);
   }
   
   /* walk through other tokens */
    while( token != NULL ) {
        strcat(dir, temp);
        memset(temp, 0 ,1024);
        strcpy(temp , token);
    
        token = strtok(NULL, s);
    }
    for(int i=0; i<MAX_FILES; i++){
        if(dirs[i] != NULL && strcmp(dirs[i]->path, dir) == 0){
            dirs[i]->files[dirs[i]->size] = (char*)calloc(sizeof(pathname), sizeof(char));
            strcpy(dirs[i]->files[dirs[i]->size], pathname);
            dirs[i]->indexes[dirs[i]->size] = fd;
            dirs[i]->size++;
            break;
            
        }
    }
    //create inode for the file
    (nodes+fd)->path = (char*)calloc(sizeof(pathname), sizeof(char));
    strcpy((nodes+fd)->path, pathname);
    (nodes+fd)->state = flags;
    (nodes+fd)->b = (block*)calloc(1, sizeof(block));
    myFS->superBlock->unused_blocks--;
    myFS->superBlock->used_blocks++;
    memset((nodes+fd)->b->data, 0, 1024);
    (nodes + fd)->ptr = 0;
    open_files->openfiles[fd] = 1;
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    return fd;
}

int myclose(int myfd){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    iNode* nodes = myFS->iNodes;
    if((nodes+myfd)->state == CLOSED){
        printf("file is already closed\n");
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return -1;
    }
    (nodes+myfd)->state = CLOSED;
    open_files->openfiles[myfd] = 0;
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
}

ssize_t myread(int myfd, void *buf, size_t count){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    if(myFS->iNodes[myfd].state != O_RDONLY && myFS->iNodes[myfd].state != O_RDWR){
        printf("file not in read state");
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return -1;
    }
    memset(buf, 0, count);
    int i=1;
    ssize_t size= 0;//to return
    iNode* inodes = myFS->iNodes;
    block* b= inodes[myfd].b;
    while (inodes[myfd].ptr>i*1024)//find the block we need to start on
    {
        if(b->next == NULL){
            b->next = (block*)calloc(1, sizeof(b));
            memset(b->next->data, 0, 1024);
        }
        i++;
        b=b->next;
    }
    i=inodes[myfd].ptr-((i-1)*1024);//where we are in the block
    for(int j=0; j<=count && b->data[i] != 0; j++){
        if(i>=1024){//finished the block
            if(b->next == NULL){
                break;
            }
            b = b->next;
            i=0;
        }
        strncat(buf, b->data + i, 1);
        i++;
        inodes[myfd].ptr++;
        size++;
    }
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    return size;
}

ssize_t mywrite(int myfd, const void *buf, size_t count){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    if(myFS->iNodes[myfd].state == O_RDONLY || myFS->iNodes[myfd].state == CLOSED){
        printf("file not in write state");
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return -1;
    }
    if(count< strlen(buf)){
        printf("count must be larget then the size of buf");
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return -1;
    }
    int i=0;
    ssize_t size= 0;//to return
    iNode* inodes = myFS->iNodes;
    block* b= inodes[myfd].b;
    if(inodes[myfd].state == O_RDONLY || inodes[myfd].state == O_RDWR){//writing
        //reset the blocks
        block* curr = b;
        while(curr != NULL){
            block *temp = curr;
            curr = curr->next;
            free(temp);
            myFS->superBlock->unused_blocks++;
            myFS->superBlock->used_blocks--; 
        }
        inodes[myfd].b = (block*)malloc(sizeof(block));
        b = inodes[myfd].b;
        b->next == NULL;
        inodes[myfd].ptr = 0;
        memset(b->data, 0, 1024);
        while(inodes[myfd].ptr < count){
            if(i >= 1024){
                if(b->next == NULL){
                    b->next = (block*)calloc(1, sizeof(b));
                    myFS->superBlock->unused_blocks--;
                    myFS->superBlock->used_blocks++;
                }
                memset(b->next->data, 0, 1024);
                i=0;
                b=b->next;
            }
            strncat(b->data, buf+inodes[myfd].ptr, 1);
            i++;
            inodes[myfd].ptr++;
        }
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return size;
    }
    else{//appending
        while (inodes[myfd].ptr>i*1024)//find the block we need to start on
        {
            i++;
            b=b->next;
        }
        i=inodes[myfd].ptr-((i-1)*1024);//where we are in the block
        //bring ptr to the end of the file
        while(b->data[i]!=0){
            if(i>=1024){//finished the block
                b = b->next;
                i=0;
            }
            i++;
            inodes[myfd].ptr++;
        }
        //write
        for(int j=0; j<count; j++){
            if(i>=1024){//finished the block
                b->next = (block*)calloc(1, sizeof(block));
                myFS->superBlock->unused_blocks--;
                myFS->superBlock->used_blocks++;
                memset(b->next->data, 0, 1024);
                b = b->next;
                b->next = NULL;
                i=0;
            }
            strncat(b->data, buf + j, 1);
            i++;
            inodes[myfd].ptr++;
            size++;
        }
    }
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    return size;
}

off_t mylseek(int myfd, off_t offset, int whence){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    if(myFS->iNodes[myfd].state == CLOSED){
        printf("file is closed");
        return -1;
    }
    if(whence == SEEK_SET){
        if(offset<0){
            printf("out of the file");
            fs = fopen("fs.txt", "w");
            fwrite(myFS, 1, sizeof(struct myfs), fs);
            fclose(fs);
            return -1;
        }
        myFS->iNodes[myfd].ptr = offset;
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return offset;
    }
    if(whence == SEEK_CUR){
        if(myFS->iNodes[myfd].ptr + offset < 0){
            printf("out of the file");
            fs = fopen("fs.txt", "w");
            fwrite(myFS, 1, sizeof(struct myfs), fs);
            fclose(fs);
            return -1;
        }
        myFS->iNodes[myfd].ptr+=offset;
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return myFS->iNodes[myfd].ptr;
    }
    int i=0;
    iNode* inodes = myFS->iNodes;
    block* b = inodes[myfd].b;
    int temp = 0;
    while(b->data[i]!=0){
        if(i>=1024){//finished the block
            if(b->next == NULL){
                break;
            }
            b = b->next;
            i=0;
        }
        i++;
        temp++;
    }
    if(temp + offset < 0){
        printf("out of the file");
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return -1;
    }
    inodes[myfd].ptr = temp+offset;
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    return inodes[myfd].ptr;
}

myDIR *myopendir(const char *name){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    for(int i=0; i<MAX_FILES; i++){//check if dir exist
        if(dirs[i] != NULL && strcmp(dirs[i]->path, name) == 0){
            myFS->iNodes[dirs[i]->inode].state=OPEN;
            fs = fopen("fs.txt", "w");
            fwrite(myFS, 1, sizeof(struct myfs), fs);
            fclose(fs);
            return dirs[i];
        }
    }
    //set a directory inode
    int fd = empty_inode();
    myFS->iNodes[fd].b = (block*)calloc(1, sizeof(block));
    myFS->superBlock->unused_blocks--;
    myFS->superBlock->used_blocks++;
    memset(myFS->iNodes[fd].b->data, 0, 1024);
    myFS->iNodes[fd].path = (char*)calloc(sizeof(name), sizeof(char));
    strcpy(myFS->iNodes[fd].path, name);
    myFS->iNodes[fd].ptr = 0;
    myFS->iNodes[fd].state = OPEN;
    myFS->superBlock->unused_iNodes--;
    myFS->superBlock->used_iNodes++;
    open_files->openfiles[fd] = 1;
    //add directory to dirs
    int i=0;
    while(dirs[i] != NULL){
        i++;
    }
    myDIR *curr = (myDIR*)malloc(sizeof(myDIR));
    curr->path = (char*)calloc(sizeof(name), sizeof(char));
    strcpy(curr->path, name);
    curr->file_index = 0;
    curr->files = (char**)calloc(100, sizeof(char*));
    curr->indexes = (int*)malloc(sizeof(int)*100);
    curr->size = 0;
    curr->inode = fd;
    sprintf(myFS->iNodes[fd].b->data, "%d", i);
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    return curr;
}

struct mydirent *myreaddir(myDIR *dirp){
    if(dirp->file_index >= dirp->size){
        return NULL;
    }
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    struct mydirent* dirent = (struct mydirent*)malloc(sizeof(struct mydirent));
    *dirent->inode = myFS->iNodes[dirp->indexes[dirp->file_index]];
    dirp->file_index++;
    return dirent;
}

int myclosedir(myDIR *dirp){
    FILE *fs = fopen("fs.txt", "r");
    fread(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    if(dirp == NULL){
        fs = fopen("fs.txt", "w");
        fwrite(myFS, 1, sizeof(struct myfs), fs);
        fclose(fs);
        return -1;
    }
    myFS->iNodes[dirp->inode].state = CLOSED;
    open_files->openfiles[dirp->inode] = 0;
    dirp->file_index = 0;
    fs = fopen("fs.txt", "w");
    fwrite(myFS, 1, sizeof(struct myfs), fs);
    fclose(fs);
    return 0;
}