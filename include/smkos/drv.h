
/* Refresh inode info (used for distributed data) */
int drvfs_refresh(inode_t *ino); 
/* Free inode-related driver meta-data */
int drvfs_release(inode_t *ino);
/* Read data from block, size and offset should be align on block size */
size_t drvfs_readblk(inode_t * ino, void *buf, size_t lg, off_t off);
/* Write data on block, size and offset should be align on block size */
size_t drvfs_writeblk(inode_t * ino, const void *buf, size_t lg, off_t off);
/* Either allocate or truncate the file */
int drvfs_resize(inode_t *ino, off_t size);  
/* Stream read -- (can blocked) */
size_t drvfs_read(inode_t * ino, void *buf, size_t lg, int flags);
/* Stream write */
size_t drvfs_write(inode_t * ino, const void *buf, size_t lg, int flags);
/* Search for the presence of an inode on the device */
int drvfs_lookup(const TCHAR *name, inode_t* dir, inode_t *ino);
/* Create a new inode */
int drvfs_mknod(const TCHAR *name, dirent_t* dir, int mode);
// int drvfs_mknod(const TCHAR *name, inode_t* dir, inode_t *ino);
/* Remove an existing inode */ 
int drvfs_rmnod(const TCHAR *name, inode_t* dir, inode_t *ino);
/*  */
int drvfs_link(const TCHAR *name, inode_t* dir, inode_t *ino);
/* */
int drvfs_unlock(const TCHAR *name, inode_t* dir, inode_t *ino);
/* Remove all of children contains by the inode */
int drvfs_dropall(inode_t *ino);
/* Create an iterator for files */
void *drvfs_opendir(inode_t *dir);
/* Iterate over directory  */
int drvfs_readdir(viod *it, inode_t *ino, const TCHAR *buf, int lg);
/*  */
int drvfs_flush(inode_t *ino);
/* */
int drvfs_prompt(inode_t *ino, const TCHAR *value); 

int drvfs_map(inode_t *ino, page_t *pg, off_t off, int psz);

int drvfs_flip(inode_t *ino);
// /* Create a record type of inode */
// int drvfs_mkrcd(const TCHAR *name, inode_t* dir, inode_t *ino, const void *buf, int lg);
// /* */
// int drvfs_getrcd(inode_t *ino, const TCHAR *buf, size_t lg);

int drvfs_readlink(inode_t *ino, TCHAR *buf, int lg)
