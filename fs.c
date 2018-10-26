#define FUSE_USE_VERSION 26
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fuse.h>

// START MACROS

#define NO_OF_INODES 90
#define DATA_BLOCK_SIZE 30

// END MACROS

// START DATA STRUCT

struct Data{
	char*data; 
	struct Data*next; 
};

// END DATA STRUCT

// START INODE STRUCT

struct Inode{
	char*name; 
	struct stat*metadata; //metadata from stat.h
	struct Data*head; // data block head pointer
	int node_type ; // 0 for file, 1 for dir , 2 for link(if implemented)
};

// START SUPERBLOCK STRUCT

struct Superblk{
	struct Inode*inode_arr; 
	int inode_arr_size; 
};

// END SUPERBLOCK STRUCT
// END INODE STRUCT


int main(){
	return 0 ; 
}