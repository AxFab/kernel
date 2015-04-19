#pragma once
#include <smkos/kernel.h>
#include <smkos/memory.h>
#include <smkos/io.h>


#define CAP_SYSTEM    0xffff
#define CAP_ADMIN    0xffff


#define SCHED_ZOMBIE 0
#define SCHED_SLEEP 1 // Can't be interrupted
#define SCHED_BLOCKED 2 // Can be interupted
#define SCHED_READY 3
#define SCHED_EXEC 4
#define SCHED_ABORT 5



#define CAP_NOBODY 0
#define CAP_EVERYBODY 1



struct kAssembly {
  size_t entryPoint_;
  kInode_t* ino_;
  struct llhead sections_;
  atomic_t usage_;
};

struct kSection {
  size_t address_;
  size_t length_;
  size_t align_;
  size_t offset_;
  struct llnode node_;
  int flags_;
};

struct kScheduler {
  kThread_t        *anchor_;
  struct spinlock lock_;
  struct semaphore  taskSem_;
  atomic_t          totalWeight_;
};

struct kThread {
  kProcess_t *process_;
  kThread_t *schNext_;
  struct llnode taskNd_;
  kMemArea_t *kstack_;
  kMemArea_t *ustack_;
  int state_;
  size_t paramValue_;
  size_t paramEntry_;
  time_t start_;
};

struct kSession {
  kUser_t *user_;
  kInode_t *workingDir_;
  atomic_t usage_;
};

struct kProcess {
  kAssembly_t* assembly_;
  kSession_t* session_;
  time_t start_;
  kMemSpace_t mspace_;
  struct spinlock lock_;
  struct llnode allNd_;
  struct llnode siblingNd_;
  struct llhead children_;
  struct llhead threads_;
  // kProcess_t *parent_;
  int runningTask_;
  page_t pageDir_;
  int exitStatus_;
  
  int pagePrivate_;
  int pageShared_;
};





/* === ASSEMBLY ========================================================== */
/** Destroy an assembly. */
void destroy_assembly (kAssembly_t *image);
/** Read an image file and create the corresponding assembly.*/
kAssembly_t *load_assembly (kInode_t *ino);


/* === DEVICE ============================================================ */
int open_fs(kInode_t* ino);
int close_fs(kInode_t* ino);
void display_inodes();
int mount_device(const char* name, kDevice_t* dev, kDriver_t* fs);
void mount_alls ();
void initialize_vfs();
kDriver_t *register_driver(void (*init)(kDriver_t *));
kDevice_t *create_device(const char* name, kInode_t* underlying, SMK_stat_t *stat, void* info);
int fs_block_read(kInode_t *fp, void* buffer, size_t length, size_t offset);


/* === INODES ============================================================ */
/** Search an inode on the filetree. */
kInode_t *search_inode (const char *path, kInode_t *dir, int flags);
/** Try to add a new inode on the VFS tree. */
kInode_t *register_inode (const char *name, kInode_t *dir, SMK_stat_t *stat, bool unlock);
/** Create a new inode. */
kInode_t *create_inode(const char* name, kInode_t* dir, int mode, size_t lg);
/** Call the inode scavanger which will try to free cached data. */
int scavenge_inodes(int nodes);
/** Function to called to grab an inodes */
int inode_open (kInode_t *ino);
/** Function to release an inodes */
int inode_close (kInode_t *ino);
/** Give the inode a symbolic link is refering to. */
kInode_t *follow_symlink(kInode_t *ino, int *links);


/* === MEMORY AREA ======================================================= */
kMemArea_t* area_get(kMemSpace_t* sp, kInode_t* ino, size_t offset, size_t length);
/** Find the area holding an address */
kMemArea_t *area_find(kMemSpace_t* sp, size_t address);
/** Will allocate a new segment on the address space */
kMemArea_t* area_map(kMemSpace_t* sp, size_t length, int flags);
/** Will allocate a new segment at a fixed address on the address space */
kMemArea_t *area_map_at (kMemSpace_t* sp, size_t address, size_t length, int flags);
int area_attach(kMemArea_t* area, kInode_t* ino, size_t offset);
int area_grow (kMemSpace_t* sp, kMemArea_t *area, size_t extra_size);
kMemArea_t* area_map_ino(kMemSpace_t* sp, kInode_t* ino, size_t offset, size_t length, int flags);
void area_unmap(kMemSpace_t* sp, kMemArea_t* area);
/** Initialize a new address space structure with a first user-stack */
int area_init(kMemSpace_t* sp, size_t base, size_t length);
int area_assembly (kMemSpace_t *sp, kAssembly_t* assembly);
void scavenge_area(kMemSpace_t* sp);


/* === PAGE FAULT ======================================================== */
int page_fault (size_t address, int cause);


/* === SCHEDULER ========================================================= */
/** Thread a signal */
int sched_signal (int raise, size_t data);
/** Insert a new thread on the scheduler */
void sched_insert(kScheduler_t *sched, kThread_t *task);
void sched_remove(kScheduler_t *sched, kThread_t *thread);
/** Change the status of the current executing task and save the current registers */
void sched_stop (kScheduler_t *sched, kThread_t *thread, int state);
void sched_next(kScheduler_t *sched);


/* === TASKS ============================================================= */
kThread_t *create_thread(kProcess_t *process, size_t entry, size_t param);
void destroy_process (kProcess_t *process);
kProcess_t *create_logon_process(kInode_t* ino, kUser_t* user, kInode_t* dir, const char*cmd);
kProcess_t *create_child_process(kInode_t* ino, kProcess_t* parent, const char*cmd);
void thread_abort (kThread_t* thread);
void process_exit(kProcess_t *process, int status);


/* === USERS ============================================================= */
kUser_t *search_user (const char *name, const char *domain);
kUser_t *create_user(const char* username, int capacity);
void destroy_user (kUser_t *user);


