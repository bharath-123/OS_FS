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
#include "assert.h"

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
	newInode -> name = (char*)malloc(sizeof(char)*strlen(path));
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
		newInode -> metadata -> st_nlink = 1;
	}
	else if(type == 1){
		//dir
		newInode -> node_type = 1; 
		newInode -> metadata -> st_mode = S_IFDIR | 0755;
		newInode -> metadata -> st_nlink = 2 ; 
	}
	return newInode; 
}

int insert_inode_to_superblk_arr(struct Inode*newInode){
	// function to create add inode pointer to inode array
	if(superblk -> inode_arr_size < NO_OF_INODES){
		superblk -> inode_arr[(superblk -> inode_arr_size)++] = newInode; 
		return superblk -> inode_arr_size; // return the index of the inode entered 
	}
	return -1 ;  
}

int get_inode_index(const char*path){
	//get the inode index based on path 
	for(int i = 0; i < superblk -> inode_arr_size ; i++){
		struct Inode*temp = superblk -> inode_arr[i]; 
		if(strcmp(temp -> name,path) == 0)
			return i ; 
	}
	return -1; 
}

struct Inode*get_inode(int index){
	return superblk -> inode_arr[index];
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

struct Data*part_insert(struct Data*head, const char*data){
	int data_size = strlen(data);
	int no_of_blks ; 
	if(data_size <= DATA_BLOCK_SIZE)
		no_of_blks = 1; 
	else
		no_of_blks = ((data_size) /DATA_BLOCK_SIZE) + 1;
	int offset = 0 ; 
	while(no_of_blks--){
		const char*temp = data;
		char*buf = (char*)malloc(sizeof(char)*DATA_BLOCK_SIZE);
		strncpy(buf,temp + offset,DATA_BLOCK_SIZE);
		head = AddToEnd(head,buf);
		offset += 30; 
	} 
	return head; 
}

char**get_data(struct Data*head){
	char**text = (char**)malloc(sizeof(char*)*10);
	struct Data*temp = head; 
	int i = 0; 
	while(temp != NULL){
		text[i] = (char*)malloc(sizeof(char)*strlen(temp -> data));
		strcpy(text[i],temp -> data);
		temp = temp -> next; 
		i += 1;
	}
	text[i] = "\00";
	return text; 
}



// END DATA BLOCK LLIST FUNCTIONS

// START FUSE FUNCTIONS

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
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT;
	}
	return 0;
}

int fs_diropen(const char*path,struct fuse_file_info*fi){
	// need to check for path validity
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT;
	} 
	return 0 ;
}

int fs_create(const char*path,mode_t mode , struct fuse_file_info*fi){
	// need to check for path validity
	// should also add code to check out of memory but later
	// create the inode for the file
	printf("In create\n");
	printf("%s \n",path);
	if(get_inode_index(path) >= 0){ // path already exists
		return 0 ; 
	}
	struct Inode*newNode = createnewInode(path,0);
	int parent_node_index = get_inode_index("/");
	if(parent_node_index == -1){
		return 0;
	}
	int new_index = insert_inode_to_superblk_arr(newNode);
	struct Inode*parent = get_inode(parent_node_index);
	parent -> metadata -> st_nlink += 1; 
	// inserted inode no of child to parent
	parent -> head = part_insert(parent -> head,path);
	printf("Success create\n");
	return 0;

}

static int fs_getattr(const char*path,struct stat*st){
	printf("In getattr\n");
	printf("%s\n",path);
	int inode_index = get_inode_index(path);
	int parent_index = get_inode_index("/");
	if(parent_index == -1) return -ENOENT;
	printf("inode index is %d ",inode_index);
	if(inode_index == -1) return -ENOENT ; //node not found
	struct Inode* node = get_inode(inode_index);
	st -> st_uid = node -> metadata -> st_uid;
	printf("uid is %d \n",st -> st_uid);
	st -> st_gid = node -> metadata -> st_gid;
	printf("gid is %d \n",st -> st_gid);
	st -> st_mode = node -> metadata -> st_mode; 
	printf("mode is %d \n",st -> st_mode);
	st -> st_nlink = node -> metadata -> st_nlink;
	printf("nlink is %d \n",st -> st_nlink);
	st -> st_size = node -> metadata -> st_size; 
	printf("size is %d \n",st -> st_size);
	printf("Getattr success\n");
	return 0;
}

int fs_mkdir(const char*path,mode_t mode){
	// need to check for path validity
	// create inode for this dir
	// get the inode for it's parent dir
	// write this dir data(inode no) into parent dir
	printf("In mkdir\n");
	printf("%s\n",path);
	if(get_inode_index(path) >= 0){
		return 0 ; 
	}
	struct Inode*newNode = createnewInode(path,1);
	int parent_node_index = get_inode_index("/");
	if(parent_node_index == -1){
		return 0;
	}
	// add to superblock
	int new_index = insert_inode_to_superblk_arr(newNode);
	struct Inode*parent = get_inode(parent_node_index);
	parent -> metadata -> st_nlink += 1; 
	// inserted inode no of child to parent
	parent -> head = part_insert(parent -> head,path);
	printf("Success MKDIR\n");
	return 0; 
}

int fs_getxattr(const char*path,struct stat*st){
	return 0;
}

int fs_readdir(const char*path,void*buf,fuse_fill_dir_t filler , off_t offset, struct fuse_file_info*fi){
	printf("In readdir\n");
	printf("%s\n",path);
	filler(buf,".",NULL,0);
	filler(buf,"..",NULL,0);
	int inode_index = get_inode_index(path);
	if(inode_index == -1) return -ENOENT ; 
	struct Inode*node = get_inode(inode_index);
	char**node_data = get_data(node -> head);
	int i =0 ; 
	while(strcmp(node_data[i],"\00")){
		printf("node data is %s ",node_data[i]);
		filler(buf,node_data[i],NULL,0);
		i +=1 ; 
	}
	printf("Readdir success\n");
	return 0;
}

//END FUSE FUNCTIONS

// START OF FUSE STRUCT

static struct fuse_operations fs_oper = {
	.open = fs_open,
	.opendir = fs_diropen,
	.mkdir = fs_mkdir,
	.readdir = fs_readdir,
	.getattr = fs_getattr,
	.create = fs_create,
};

// END OF FUSE STRUCT
int main(int argc,char*argv[]){
	fs_start(); 
	fuse_main(argc,argv,&fs_oper,NULL);
	return 0 ; 
}