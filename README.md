# OS_FS
A text based filesystem using FUSE

Design : 

We will be using an array to store the inodes. Each element of the array will be of type struct inode*.

struct inode{
	char *name; //name of the inode. can be file or directory
	struct stat*metadata; // use stat.h to store the file metadata
	struct Data*head; // pointer to the head of the data linked list
	int nodetype; // 0 for file, 1 for directory, 2 for links(if we implement)
};

We will be using a linked list to store our data. Each inode will have a linked list associated with it. The size of each node of the linked
list will be 30 characters. The structure of the data block will be 

struct Data{
	char*data;
	struct Data*next;
};

We will be accessing all the filesystem data through a superblock struct which will make things more organised.
Files will have text.
Directory Data will contain the files present in the directory

struct superblock{
	struct inode *inode_arr;
	int inode_arr_size; 	
	int filesys_memory; // the available memory to the filesystem
};

The superblock will be initialised before we mount our filesystem. Before we call the fuse_main function.

We will keep some macros.
1) no_of_inodes = 90
2) data_block_size = 30


I have an idea of using a trie to check for the validity of the path of a file or dir.
eg : if we have a path "/tmp/dir/file.txt" . This will be an error if dir is not present. So we can a trie to such error checks. 

The names in the Data block and Inode blocks will be the absolute paths of the file
