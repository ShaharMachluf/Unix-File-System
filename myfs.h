#include <sys/types.h>
#include <unistd.h>

#define CLOSED 0
#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR 3
#define O_APPEND 4
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define MAX_FILES 10000
#define OPEN 1

typedef struct SuperBlock{
    // int max_size;
    long total_iNodes;
    long total_blocks;
    long used_iNodes;
    long used_blocks;
    long unused_iNodes;
    long unused_blocks;
}sb;

typedef struct Block{
    char data[1024];
    struct Block* next;
}block;

typedef struct INode{
    char* path;
    block* b;
    int state;
    off_t ptr;
}iNode;

typedef struct myfs{
    sb *superBlock;
    iNode* iNodes;
};

typedef struct my_dir{
    char* path;
    // int dir_index;
    int file_index;//index of current file in mydirent
    int inode;//inode representing the directory
    int* indexes;//indexes of the files in the directory
    char** files;//paths of the files in h directory 
    int size;//number of files in the directory
}myDIR;

struct mydirent{
    iNode* inode;
};

struct myopenfile{
    int* openfiles;
};

void mymkfs(long s);
int mymount(const char *source, const char *target,
 const char *filesystemtype, unsigned long
mountflags, const void *data);
int myopen(const char *pathname, int flags);
int myclose(int myfd);
ssize_t myread(int myfd, void *buf, size_t count);
ssize_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
struct mydirent *myreaddir(myDIR *dirp);
int myclosedir(myDIR *dirp);
