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

// START DATA BLOCK LLIST FUNCTIONS

struct Data*createnewnode(char *text){
	struct Data*newNode = (struct Data*)malloc(sizeof(struct Data));
	newNode -> next = NULL; 
	newNode -> data = (char*)malloc(sizeof(char)*35);
	strcpy(newNode -> data, text);
	return newNode; 
}

struct Data*AddToEnd(struct Data*head,char*data){
	struct Data*newNode = createnewnode(data);
	if(head == NULL){
		head = newNode;
	}
	else{
		struct Data*temp = head;
		while(temp -> next != NULL){
			temp = temp -> next; 
		}
		temp -> next = newNode; 
	}
	return head;
}

struct Data*part_insert(struct Data*head, char*data){
	int data_size = strlen(data);
	int no_of_blks ; 
	if(data_size <= 30)
		no_of_blks = 1; 
	else
		no_of_blks = 3;
	int offset = 0 ; 
	while(no_of_blks--){
		char*temp = data;
		char*buf = (char*)malloc(sizeof(char)*30);
		strncpy(buf,temp + offset,30);
		head = AddToEnd(head,buf);
		offset += 30; 
	} 
	return head; 
}


// END DATA BLOCK LLIST FUNCTIONS

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
	// need to create inode
	// need to write to inode block of directory
	// add inode to the superblock array
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