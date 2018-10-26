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

// END INODE STRUCT

// START SUPERBLOCK STRUCT

struct Superblk{
	struct Inode**inode_arr; 
	int inode_arr_size; 
};

struct Superblk*superblk;

// END SUPERBLOCK STRUCT

// START HELPER FUNCTIONS

struct Inode* createnewInode(const char*path,int type){ // 0 for file, 1 for dir, 2 for links
	struct Inode*newInode = (struct Inode*)malloc(sizeof(struct Inode));
	newInode -> name = (char*)malloc(sizeof(char)*50);
	strcpy(newInode -> name,path);
	newInode -> head = NULL;
	newInode -> metadata = (struct stat*)malloc(sizeof(struct stat));
	newInode -> metadata -> st_uid = getuid(); 
	newInode -> metadata -> st_gid = getgid(); 
	newInode -> metadata -> st_size = sizeof(struct Inode) + sizeof(struct stat);
	// type specific methods
	if(type == 0){
		// file
		newInode -> node_type = 0 ; 
		newInode -> metadata -> st_mode = S_IFREG | 0755;
	}
	else if(type == 1){
		//dir
		newInode -> node_type = 1; 
		newInode -> metadata -> st_mode = S_IFDIR | 0755;
	}
	return newInode; 
}

int insert_inode_to_superblk_arr(struct Inode*newInode){
	// function to create add inode pointer to inode array
	if(superblk -> inode_arr_size < NO_OF_INODES){
		superblk -> inode_arr[(superblk -> inode_arr_size)++] = newInode; 
		return 1; 
	}
	return 0 ;  
}

int get_inode_index(const char*path){
	//get the inode index based on path 
	for(int i = 0; i < NO_OF_INODES ; i++){
		struct Inode*temp = superblk -> inode_arr[i]; 
		if(strcmp(temp -> name,path) == 0)
			return i ; 
	}
	return -1; 
}

// END HELPER FUNCTIONS

void fs_start(){
	//This function is to initialise the superblock structure
	superblk = (struct Superblk*)malloc(sizeof(struct Superblk));
	//init the inode array
	superblk -> inode_arr = (struct Inode**)malloc(sizeof(struct Inode*)*NO_OF_INODES);
	//init the inode array size to 0
	superblk -> inode_arr_size = 0 ;  
	// inode_arr[0] will be reserved for the root
	struct Inode*newInode = createnewInode("/",1);
	insert_inode_to_superblk_arr(newInode);
}

int fs_open(const char*path,struct fuse_file_info*fi){
	// need to check for path validity
	return 0;
}

int fs_diropen(const char*path,struct fuse_file_info*fi){
	// need to check for path validity
	return 0 ; 
}

int fs_create(const char*path,mode_t mode , struct fuse_file_info*fi){
	// need to check for path validity
	// should also add code to check out of memory but later
	// create the inode for the file

	return 0;

}

// START OF FUSE STRUCT

static struct fuse_operations fs_oper = {
	.open = fs_open,
	.opendir = fs_diropen,
	.create = fs_create
};

// END OF FUSE STRUCT
int main(int argc,char*argv[]){
	fs_start(); 
	fuse_main(argc,argv,&fs_oper,NULL);
	return 0 ; 
}