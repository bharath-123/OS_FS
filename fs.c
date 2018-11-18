#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 30
#endif
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

// START STAT STRUCT

struct st_stat{
	time_t atime;
	time_t mtime;
	time_t ctime;
	uid_t st_uid;
	gid_t st_gid; 
	off_t st_size; 
	mode_t st_mode;
	int st_nlink;
	int blocks;
};


// END STAT STRUCT

// START INODE STRUCT

struct Inode{
	char*name; 
	struct st_stat*metadata; //metadata from st_stat struct
	struct Data*head; // data block head pointer
	int node_type ; // 0 for file, 1 for dir 
	int diskfile_no; // this is the number of the diskfile to which the data is being written to.
	// diskfile_no.metadata diskfile_no.data
};

// END INODE STRUCT

// START SUPERBLOCK STRUCT

struct Superblk{
	struct Inode**inode_arr; 
	int inode_arr_size; 
	int diskfiles_alloc; // keep track of the no of diskfiles allocated
};

struct Superblk*superblk;

// END SUPERBLOCK STRUCT

// START DISKFILE MANAGEMENT FUNCTIONS

int get_diskfileno(){
	if(superblk -> diskfiles_alloc + 1 == NO_OF_INODES){
		return -1;
	}
	else{
		int x = superblk -> diskfiles_alloc;
		superblk -> diskfiles_alloc++;
		return x; 
	}
}

char*ret_metadata_filename(int diskfile_no){
	// get metadata filename given number
	char*metadata_filename = (char*)malloc(50);
	int mf = snprintf(metadata_filename,50,"./diskfiles/%d.metadata",diskfile_no);
	if(mf > 0){
		return metadata_filename;
	}
	return NULL;
}

char*ret_data_filename(int diskfile_no){
	// get data filename given number
	char*data_filename = (char*)malloc(50);
	int mf = snprintf(data_filename,50,"./diskfiles/%d.data",diskfile_no);
	if(mf > 0){
		return data_filename;
	}
	return NULL;
}

char*get_metadata_filename(struct Inode*newInode){
	// get the metadata filename for an inode
	int diskfile_no = newInode -> diskfile_no;
	char*metadata_filename = (char*)malloc(50);
	int mf = snprintf(metadata_filename,50,"./diskfiles/%d.metadata",diskfile_no);
	if(mf > 0){
		return metadata_filename;
	}
	return NULL;
}

char*get_data_filename(struct Inode*newInode){
	// get the data filename for an inode
	int diskfile_no = newInode -> diskfile_no;
	char*data_filename = (char*)malloc(50);
	int df = snprintf(data_filename,50,"./diskfiles/%d.data",diskfile_no);
	if(df > 0){
		return data_filename;
	}
	return NULL;
}

int check_diskfile(struct Inode*newInode){
	// function to check if the inodes diskfile is present
	// return 1 if present else return -1

	// try to open the file
	int diskfile_no = newInode -> diskfile_no;
	char*metadata_filename = get_metadata_filename(newInode);
	char*data_filename = get_data_filename(newInode);
	if(metadata_filename && data_filename){
		FILE*mfp = fopen(metadata_filename,"r");
		FILE*dfp = fopen(data_filename,"r");
		if(mfp != NULL && dfp != NULL){
			return 1; 
		}
		return -1;
	}
	return 0;

}

int check_diskfile_no(int diskfile_no){
	// function to check if diskfile is present given the inode no
	// return 1 if present else return -1
	char*metadata_filename = ret_metadata_filename(diskfile_no);
	char*data_filename = ret_data_filename(diskfile_no);
	if(metadata_filename && data_filename){
		if(metadata_filename && data_filename){
			FILE*mfp = fopen(metadata_filename,"r");
			FILE*dfp = fopen(data_filename,"r");
			if(mfp != NULL && dfp != NULL){
				return 1;
			}
			return 0;
		}
	}
	return 0;
}

int remove_diskfileno(int diskfile_no){
	// function to remove diskfile of a given number
	// return 1 if success else return 0
	int check = check_diskfile_no(diskfile_no);
	char*metadata_filename = ret_metadata_filename(diskfile_no);
	char*data_filename = ret_data_filename(diskfile_no);
	if(check == 1){
		int mret = remove(metadata_filename);
		int dret = remove(data_filename);
		if(mret == 0 && dret == 0){
			return 1;
		}
		else{
			return 0;
		}
	}
	return 0;
}

int remove_diskfile(struct Inode*newInode){
	// function to remove diskfile of the inode
	// useful if inode is deleted
	// return 0 on failure, return 1 on success, -2 if inode is invalid

	if(newInode == NULL){
		return -2;
	}
	else{
		// check if diskfile is present or not
		int check = check_diskfile(newInode);
		if(check == 1){
			char*metadata_filename = get_metadata_filename(newInode);
			char*data_filename = get_data_filename(newInode);
			int mret = remove(metadata_filename);
			int dret = remove(data_filename);
			if(mret == 0 && dret == 0){
				return 1; 
			}
		}
		else{
			return 0;
		}

	}
}


int create_diskfile(struct Inode*newInode){
	/* This function is to create a diskfile
	   If there is an error creating a diskfile then it will return -1
	   else it will return 1 on success
	*/
	if(newInode == NULL){
		return -1;
	}
	else{
		// we got the diskfile no for this Inode
		// now create the metadata and data files.

		// assign the diskfile_no to the Inode. THIS IS DONE DURING INODE CREATION

		// now assemble the diskfile names
		char*metadata_filename = get_metadata_filename(newInode);
		char*data_filename = get_data_filename(newInode);
		if(metadata_filename != NULL && data_filename != NULL){
			// files successfullyy named
			FILE*mfp = fopen(metadata_filename,"ab+");
			FILE*dfp = fopen(data_filename,"ab+");
			if(mfp && dfp){
				return 1; 
			}
			else{
				return -1;
			}
		}

	}
}

// END DISKFILE MANAGEMENT FUNCTIONS

// START HELPER FUNCTIONS

struct Inode* createnewInode(const char*path,int type){ // 0 for file, 1 for dir, 2 for links
	struct Inode*newInode = (struct Inode*)malloc(sizeof(struct Inode));
	newInode -> name = malloc(sizeof(char)*(strlen(path)));
	strcpy(newInode -> name,path);
	newInode -> head = NULL;
	newInode -> metadata = (struct st_stat*)malloc(sizeof(struct st_stat));
	newInode -> metadata -> st_uid = getuid(); 
	newInode -> metadata -> st_gid = getgid(); 
	newInode -> metadata -> st_size = sizeof(struct Inode) + sizeof(struct st_stat);
	newInode -> metadata -> atime = time(NULL);
	newInode -> metadata -> mtime = time(NULL);
	newInode -> metadata -> ctime = time(NULL);
	newInode -> metadata -> blocks = 1; // there is one block for the inode
	// create the diskfile for this inode
	int diskfile_no = get_diskfileno();
	if(diskfile_no != -1)
		newInode -> diskfile_no = diskfile_no;
	else
		// could not allocated diskfile
		return NULL;
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
	// if we find out that the inode array is full then do a 2nd poll to check if any NULL values were introduced because of deletion
	if(superblk -> inode_arr_size < NO_OF_INODES){
		superblk -> inode_arr[(superblk -> inode_arr_size)++] = newInode; 
		return superblk -> inode_arr_size; // return the index of the inode entered 
	}
	for(int i = 0 ; i < NO_OF_INODES ;i++){
		if(superblk -> inode_arr[i] == NULL){
			// node has been deleted
			superblk -> inode_arr[i] = newInode;
			return i;
		}
	}
	return -1;   
}

int get_inode_index(const char*path){
	//get the inode index based on path 
	for(int i = 0; i < superblk -> inode_arr_size ; i++){
		struct Inode*temp = superblk -> inode_arr[i]; 
		if(temp != NULL){
			if(strcmp(temp -> name,path) == 0)
				return i ; 
		}
	}
	return -1; 
}

struct Inode*get_inode(int index){
	return superblk -> inode_arr[index];
}

char* ret_file(const char* path)
{   
    int l = strlen(path);
    char *prev=(char*)malloc(sizeof(char)*(l+1));
    int i = l-1;
    while(i>0 && path[i]!='/'){
        i--;
    } 
    int j = 0;
    int k=0;
    for(j=i+1;j<l;j++)
    {
        prev[k]=path[j];
        k++;
    }
    prev[k]='\0';
    return prev;
}

char* ret_dir(const char* path)
{
   if(strcmp(path,"/") == 0){
   		char *str1 = (char*)malloc(sizeof(char)*(strlen(path) + 1));
   		strcpy(str1,"/");
   		return str1;
   }
   int count = 0 ; 
   for(int i = 0 ; i < strlen(path); i++){
      if(path[i] == '/'){
      	count += 1; 
      }
   }
   if(count == 1){
   		char *str1 = (char*)malloc(sizeof(char)*(strlen(path) + 1));
   		strcpy(str1,"/");
   		return str1;
   }
   else{
	   int l = strlen(path);
	   int i = l-1;
	   while(i>0 && path[i]!='/'){
	        i--;
	   } 
	   int j =0 ;
	   char *str1 = (char*)malloc(sizeof(char)*(strlen(path) + 1));  
	   
	   for(j=0;j<i;j++)
	   {
	    str1[j]=path[j];
	   }
	   str1[j] = '\0';
	   return str1;
	}
}


void free_inode(struct Inode*node,int inode_index){
	free(node -> head);
	free(node -> metadata);
	free(node -> name);
	free(node);
	// now mark the inode_index with NULL
	superblk -> inode_arr[inode_index] = NULL;
}

// END HELPER FUNCTIONS

// START DATA BLOCK LLIST FUNCTIONS

struct Data*createnewnode(char *text){
	struct Data*newNode = (struct Data*)malloc(sizeof(struct Data));
	newNode -> next = NULL; 
	newNode -> data = (char*)malloc(sizeof(char)*(strlen(text) + 1));
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

void deleteKey(struct Data **head_ref, const char*key) 
{ 
    // Store head node 
    struct Data* temp = *head_ref, *prev; 
  
    // If head node itself holds the key or multiple occurrences of key 
    while (temp != NULL && (strcmp(temp -> data,key)) == 0) 
    { 
        *head_ref = temp->next;   // Changed head 
        free(temp);               // free old head 
        temp = *head_ref;         // Change Temp 
    } 
  
    // Delete occurrences other than head 
    while (temp != NULL) 
    { 
        // Search for the key to be deleted, keep track of the 
        // previous node as we need to change 'prev->next' 
        while (temp != NULL && strcmp(temp -> data,key) != 0) 
        { 
            prev = temp; 
            temp = temp->next; 
        } 
  
        // If key was not present in linked list 
        if (temp == NULL) return; 
  
        // Unlink the node from linked list 
        prev->next = temp->next; 
  
        free(temp);  // Free memory 
  
        //Update Temp for next iteration of outer loop 
        temp = prev->next; 
    } 
} 

// will insert to 30 bytes to linked list
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
		char*buf = (char*)malloc(sizeof(char)*(DATA_BLOCK_SIZE + 1));
		strncpy(buf,temp + offset,DATA_BLOCK_SIZE);
		head = AddToEnd(head,buf);
		offset += 30; 
	} 
	return head; 
}

// function to get data from linked list
char**get_data(struct Data*head){
	char**text = (char**)malloc(sizeof(char*)*10);
	struct Data*temp = head; 
	int i = 0; 
	while(temp != NULL){
		text[i] = (char*)malloc(sizeof(char)*(strlen(temp -> data) + 1));
		strcpy(text[i],temp -> data);
		temp = temp -> next; 
		i += 1;
	}
	text[i] = "\0";
	return text; 
}

// END DATA BLOCK LLIST FUNCTIONS

// START DISKFILE I/O FUNCTIONS

int write_metadata_to_diskfile(struct Inode*newNode){
	// helper function to write the inode metadata to the diskfile
	// return 1 on success else returns -1
	char*metadata_filename = get_metadata_filename(newNode);
	FILE*fp = fopen(metadata_filename,"w+");
	int fd = fileno(fp);
	// could afford to write in blocks as all the sizes in the inode struct is of a fixed size
	// now write the metadata to the file
	fwrite(newNode -> name,sizeof(char),50,fp);
	fsync(fd);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fwrite(&(newNode -> node_type),sizeof(int),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> atime),sizeof(time_t),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> mtime),sizeof(time_t),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> ctime),sizeof(time_t),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> st_uid),sizeof(uid_t),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> st_gid),sizeof(gid_t),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> st_size),sizeof(off_t),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> st_mode),sizeof(mode_t),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> st_nlink),sizeof(int),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	fwrite(&(newNode -> metadata -> blocks),sizeof(int),1,fp);
	//fwrite("\n",sizeof(char),strlen("\n"),fp);
	fsync(fd);
	return 1;
}

int write_data_to_diskfile(struct Inode*newNode){
	// helper function to write the inode data to the diskfile
	// return 1 on success else returns -1
	if(newNode -> head == NULL){
		// Case to handle if the node has no data. 
		// We still want the data file for the inode for consistency 
		char*data_filename = get_data_filename(newNode);
		remove(data_filename);
		FILE*fp = fopen(data_filename,"w");
		return -1;
	}
	else{
		// could not write the node data in blocks like I did for metadata 
		// as we don't know the size of the data beforehand.
		char*data_filename = get_data_filename(newNode);
		FILE*fp = fopen(data_filename,"w");
		// get the data from each block
		char**block_data = get_data(newNode -> head);
		// now write all the blocks of the data to the text file
		// seperate each block data by a line
		int i = 0; 
		while(strcmp(block_data[i],"\0")){
			fprintf(fp,"%s\n",block_data[i]);
			i += 1; 
		}
		return 1; 
	}
}


int write_to_diskfile(struct Inode*newNode){
	// function to be called when we are writing to the diskfile
	// returns 1 if write is successful else returns -1

	// This function just includes calls to write_metadata_to_diskfile and write_data_to_diskfile

	// check if diskfile is present or not. Offload that responsibility from the write_metadata_to_diskfile and write_data_to_diskfile
	if(check_diskfile(newNode)){
		int met_err = write_metadata_to_diskfile(newNode);
		int data_err = write_data_to_diskfile(newNode);
		if(met_err == 1 && data_err == 1){
			return 1;
		}
		return 0;
	}
	else{
		return -1;
	}
}


struct Inode*read_metadata(struct Inode*newNode,int diskfile_no){
	if(newNode == NULL){
		return NULL;
	}
	else{
		// the file will be read in blocks. Since the inode struct has fields of well defined sizes 
		// unlike the data block.
		char*metadata_filename = ret_metadata_filename(diskfile_no);
		FILE*fp = fopen(metadata_filename,"r");
		// now read the file
		char*name = (char*)malloc(50);
		fread(name,sizeof(char),50,fp);
		strcpy(newNode -> name,name);
		int nodetype;
		fread(&nodetype,sizeof(int),1,fp);
		newNode -> node_type = nodetype;
		time_t atime;
		fread(&atime,sizeof(time_t),1,fp);
		newNode -> metadata -> atime = atime;
		time_t mtime;
		fread(&mtime,sizeof(time_t),1,fp);
		newNode -> metadata -> mtime = mtime;
		time_t ctime;
		fread(&ctime,sizeof(time_t),1,fp);
		newNode -> metadata -> ctime = ctime;
		uid_t uid;
		fread(&uid,sizeof(uid_t),1,fp);
		newNode -> metadata -> st_uid = uid;
		gid_t gid;
		fread(&gid,sizeof(gid_t),1,fp);
		newNode -> metadata -> st_gid = gid; 
		off_t size;
		fread(&size,sizeof(off_t),1,fp);
		newNode -> metadata -> st_size = size;
		mode_t mode;
		fread(&mode,sizeof(mode_t),1,fp);
		newNode -> metadata -> st_mode = mode; 
		int nlink;
		fread(&nlink,sizeof(int),1,fp);
		newNode -> metadata -> st_nlink = nlink;
		int blocks;
		fread(&blocks,sizeof(int),1,fp);
		newNode -> metadata -> blocks = blocks;
		return newNode;
	}
}

struct Inode*read_data(struct Inode*newNode,int diskfile_no){
	// read the data line by line
	// don't check if diskfile is present or not again, it's already done by the load_Inode function
	char*data_filename = ret_data_filename(diskfile_no);
	FILE*fp = fopen(data_filename,"r");
	char*line = NULL;
	size_t len = 0;
	ssize_t read;
	while((read = getline(&line,&len,fp)) != -1){
		if(strcmp(line,"\n")){
			strtok(line,"\n");
			newNode -> head = part_insert(newNode -> head,line);
		}
	}
	return newNode;
}

struct Inode* load_Inode(int diskfile_no){
	// function to load inode data from diskfile
	// returns a new inode if success else returns NULL

	// dont need to check if diskfile is present as that is taken care in the FS reloader.
	// But I ll implement it anyways lol cause we might use this somewhere else
	if(check_diskfile_no(diskfile_no)){
		// load the metdata into the inode
		struct Inode*newNode;
		// just create the inode with any values, they will be overwritten anyways
		newNode = createnewInode("",1);
		newNode = read_metadata(newNode,diskfile_no);
		// now load the data into the inode
		newNode = read_data(newNode,diskfile_no);
		return newNode;	

	}
}

// START FS INITIALIZER

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
	// here we create the diskfille for the root inode.
	// init the superblk diskfiles_alloc
	superblk -> diskfiles_alloc = 1; 
	int err = create_diskfile(newInode);
	write_to_diskfile(newInode);
}

// END FS INITIALIZER

// START FS RELOADER

int reload_FS(){
	// here we reload the filesystem from the diskfiles
	// first check if the superblock diskfile is present
	// if is is not there just start the FS using fs_start() 
	// else we have to reload everything from diskfiles
	if(check_diskfile_no(0)){
		//This function is to initialise the superblock structure
		superblk = (struct Superblk*)malloc(sizeof(struct Superblk));
		//init the inode array
		superblk -> inode_arr = (struct Inode**)malloc(sizeof(struct Inode*)*NO_OF_INODES);
		//init the inode array size to 0
		superblk -> inode_arr_size = 0 ;  
		superblk -> diskfiles_alloc = 0;
		// we have finished initialising the superblock
		// now we need to load into the superblock
		for(int i = 0 ; i < NO_OF_INODES ;i++){
			// check if diskfile is present
			if(check_diskfile_no(i)){
				// create the newInode
				struct Inode*newNode;
				// now load data into this inode
				newNode = load_Inode(i);
				// now insert this inode to the superblock array
				insert_inode_to_superblk_arr(newNode);
				// now reassign the diskfileno to this inode
				newNode -> diskfile_no = i;
			}
		}
	}
	else{
		fs_start();
		return 1;
	}

}

// END FS RELOADER

// END DISKFILE I/O FUNCTIONS



// START FUSE FUNCTIONS

int fs_open(const char*path,struct fuse_file_info*fi){
	// need to check for path validity
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT;
	}
	struct Inode*node = get_inode(inode_index);
	return 0;
}

int fs_diropen(const char*path,struct fuse_file_info*fi){
	// need to check for path validity
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT;
	} 
	return 0;
}

int fs_create(const char*path,mode_t mode , struct fuse_file_info*fi){
	// need to check for path validity
	// should also add code to check out of memory but later
	// create the inode for the file
	printf("In create\n");
	printf("%s \n",path);
	if(get_inode_index(path) >= 0){ // path already exists
		return -EEXIST ; 
	}
	struct Inode*newNode = createnewInode(path,0);
	char*parent_dir = ret_dir(path);
	int parent_node_index = get_inode_index(parent_dir);
	if(parent_node_index == -1){
		return -ENOENT;
	}
	int new_index = insert_inode_to_superblk_arr(newNode);
	// now create the diskfile
	int err = create_diskfile(newNode);
	// update the newly created inodes diskfile
	write_to_diskfile(newNode);
	struct Inode*parent = get_inode(parent_node_index);
	parent -> metadata -> st_nlink += 1; 
	// inserted inode no of child to parent
	parent -> head = part_insert(parent -> head,path);
	// also update the parents diskfile
	write_to_diskfile(parent);
	printf("Success create\n");
	return 0;

}

int fs_getattr(const char*path,struct stat*st){
	printf("In getattr\n");
	printf("%s\n",path);
	int inode_index = get_inode_index(path);
	char*parent = ret_dir(path);
	int parent_index = get_inode_index(parent);
	if(parent_index == -1) return -ENOENT;
	printf("inode index is %d ",inode_index);
	if(inode_index == -1) return -ENOENT; //node not found
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
	st -> st_blocks = node -> metadata -> blocks;
	st -> st_atime = node -> metadata -> atime;
	st -> st_mtime = node -> metadata -> mtime;
	st -> st_ctime = node -> metadata -> ctime;
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
		return -EEXIST; 
	}
	struct Inode*newNode = createnewInode(path,1);
	char*parent = ret_dir(path);
	int parent_node_index = get_inode_index(parent);
	if(parent_node_index != -1){
		// add to superblock
		int new_index = insert_inode_to_superblk_arr(newNode);
		// create the diskfile
		int err = create_diskfile(newNode);
		// now update its diskfile
		write_to_diskfile(newNode);
		struct Inode*parent = get_inode(parent_node_index);
		parent -> metadata -> st_nlink += 1; 
		// inserted inode no of child to parent
		// update the number of the blocks used by the directory.
		int no_of_blks = (strlen(path)/(DATA_BLOCK_SIZE)) + 1;
		parent -> metadata -> blocks += no_of_blks;
		parent -> head = part_insert(parent -> head,path);
		// now write to its diskfile
		write_to_diskfile(parent);
		printf("Success MKDIR\n");
	}	
	return 0; 
}

int fs_readdir(const char*path,void*buf,fuse_fill_dir_t filler , off_t offset, struct fuse_file_info*fi){
	printf("In readdir\n");
	printf("%s\n",path);
	filler(buf,".",NULL,0);
	filler(buf,"..",NULL,0);
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT ; 
	}
	struct Inode*node = get_inode(inode_index);
	// check if node is a directory or file
	if(node -> node_type != 1){
		return -ENOTDIR;
	}
	char**node_data = get_data(node -> head);
	int i =0 ; 
	while(strcmp(node_data[i],"\0")){
		printf("node data is %s ",node_data[i]);
		char*temp = ret_file(node_data[i]);
		filler(buf,temp,NULL,0);
		i +=1 ; 
	}
	printf("Readdir success\n");
	// we need to update the diskfile of the particular node
	write_to_diskfile(node);
	return 0;
}

static int fs_write(const char*path,const char*buf,size_t size,off_t offset,struct fuse_file_info*fi){
	printf("In write\n");
	//get the inode to write to
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT;
	}
	struct Inode*node = get_inode(inode_index);
	// write works only for files. We cant have 2 files of the same name
	if(node -> node_type != 0){
		return -ENOENT;
	}
	// update the mtime and atime of the node
	node -> metadata-> mtime = time(NULL);
	node -> metadata -> atime = time(NULL);
	//update the size of the file
	node -> metadata -> st_size += size;
	// strip the buf of the \n
	strtok(buf,"\n");
	// add the data to the node
	// first create a temp buffer to store the data
	char*temp = (char*)malloc(sizeof(char)*(strlen(buf) + 1));
	// clear the node head
	node -> head = NULL;
	//memset(temp + offset,0,size);
	strcpy(temp,buf);
	printf("%d\n",strlen(temp));
	// now insert to the linked list
	// lets update the number of blocks since we know how many blocks will be allocated by length of buf
	int no_of_blks = (strlen(temp)/(DATA_BLOCK_SIZE)) + 1;
	node -> metadata -> blocks = no_of_blks;
	node -> head = part_insert(node -> head,temp);
	// now set the buffer to 0
	// update the diskfile of the node
	write_to_diskfile(node);
	//memset((char*)buf,0,strlen(buf));
	return size; 
}

int fs_read(const char*path,char*buf,size_t size,off_t offset,struct fuse_file_info*fi){
	printf("In read\n");
	// get the inode to read
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT;
	}
	struct Inode*node = get_inode(inode_index);
	// check if node is a file 
	if(node -> node_type != 0){
		return -ENOENT;
	}
	//update the access time of the file
	node -> metadata -> atime = time(NULL);
	// get the data from the linked list
	char**node_data = get_data(node -> head);
	int temp_size = 0;
	int i =0 ; 
	int off = 0;
	while(strcmp(node_data[i],"\0")){
		printf("node data is %s ",node_data[i]);
		temp_size += strlen(node_data[i]);
		strcpy(buf + off,node_data[i] + offset);
		i += 1 ;
		off += 30; 
	}
	if(temp_size == 0) return 0;
	// we need to update the atime and all that
	write_to_diskfile(node);
	return temp_size; 

}

int fs_rmdir(const char*path){
	printf("In rmdir\n");
	// get the inode to be deleted
	int inode_index = get_inode_index(path);
	// check if the inode is present
	if(inode_index == -1){
		return -ENOENT;
	}
	// get the inode
	struct Inode*node = get_inode(inode_index);
	if(node -> node_type !=1){
		return -ENOENT;
	}
	// node will be considered for deletion only if it is empty
	if(node -> head != NULL){
		return -ENOTEMPTY;
	}
	// need to update the parent dir now
	// get the parent inode
	char*parent = ret_dir(path);
	int parent_index = get_inode_index(parent);
	struct Inode*parent_inode = get_inode(parent_index);
	// now need to delete the value from the parent inodes linked list
	deleteKey(&(parent_inode -> head),path);
	// now reduce the link of the parent inode
	parent_inode -> metadata -> st_nlink -= 1; 
	// remove the diskfile of the inode before deallocating it
	int err = remove_diskfile(node);
	// now we need to deallocate the original inode
	free_inode(node,inode_index);
	// update the diskfile of the parent
	write_to_diskfile(parent_inode);
	return 0;	
}

int fs_unlink(const char*path){
	printf("In unlink\n");
	// get the inode to be deleted
	int inode_index = get_inode_index(path);
	// check if the inode is present
	if(inode_index == -1){
		return -ENOENT;
	}
	//get the inode
	struct Inode*node = get_inode(inode_index);
	if(node -> node_type != 0){
		return -ENOENT;
	}
	// need to update the parent dir now
	// get the parent inode
	char*parent = ret_dir(path);
	int parent_index = get_inode_index(parent);
	struct Inode*parent_inode = get_inode(parent_index);
	// now need to delete the value from the parent inodes linked list
	deleteKey(&(parent_inode -> head),path);
	// now reduce the link of the parent inode
	parent_inode -> metadata -> st_nlink -= 1;
	// remove the diskfile of the inode before deallocating it
	int err = remove_diskfile(node);
	// now we need to deallocate the original inode
	free_inode(node,inode_index);
	// now update the parents diskfile
	write_to_diskfile(parent_inode);
	return 0;

}

int fs_chmod(const char*path,mode_t mode){
	int inode_index = get_inode_index(path);
	if(inode_index == -1){
		return -ENOENT;
	}
	struct Inode*node = get_inode(inode_index);
	// now i need to change the permissions
	if(node -> node_type == 0){
		node -> metadata -> st_mode = S_IFREG | mode; 
	}
	else{
		node -> metadata -> st_mode = S_IFDIR | mode; 
	}
	// update the nodes diskfile
	write_to_diskfile(node);
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
	.write = fs_write,
	.read = fs_read,
	.rmdir = fs_rmdir,
	.unlink = fs_unlink,
	.chmod = fs_chmod
};

// END OF FUSE STRUCT
int main(int argc,char*argv[]){
	// here reload the superblock if it is present
	// if it is present then assign the superblock struct from the file to the superblock point
	// if it is not present then run fs_start() to init the superblock and create the superblock files
	int err = reload_FS();
	fuse_main(argc,argv,&fs_oper,NULL);
	return 0 ; 
}