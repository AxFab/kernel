#ifndef KCONFIG_H__
#define KCONFIG_H__


#define MAX_STRING_LENGTH   256
#define MAX_LOOP_STREAM     32
#define MAX_LOOP_BUCKET     512


#define MAX_TMPFILE_NAME    32
#define MAX_SYMLINK_LOOP  64

#define PTR_POISON  ((void*)0xdeaddead)

#define USR_SPACE_BASE  0x00800000
#define USR_SPACE_LIMIT 0xD0000000

#define ROOT_UID 0x1593

#define FS_DEV_NODE "dev"
#define FS_MNT_NODE "mnt"

#define FILENAME_SEPARATORS "/\\"
#define VOLUME_SEPARATOR ':'

#define HEAP_START ((void*)(256 * _Mb_))
#define STACK_DEFAULT (1 * _Mb_)

#define FILE_MAP_SIZE  (8 * _Kb_)

#define CLOCK_HZ  128

enum SYS_STATE
{
  SYS_STATE_ON = 0,
  SYS_STATE_OFF,
};

enum CPU_STATE
{
  CPU_STATE_NONE = 0,
  CPU_STATE_USER,
  // CPU_STATE_SYSTEM,
  CPU_STATE_NICE,
  CPU_STATE_IDLE,
  CPU_STATE_IOWAIT,
  CPU_STATE_HARDWARE_IRQ,
  CPU_STATE_SOFTWARE_IRQ,
  CPU_STATE_STEALTIME,

  /** This mode is valid only when the CPU are on user-space and should never
   *  be set on kernel code.
   */
  CPU_STATE_USERMODE,

  /** State given at the entry into a system call.
   *  The function called on this state must check for user privileges.
   */
  CPU_STATE_SYSCALL,

  /** State of the cpu, when no task are running.
   *  This state can only be given by the function StopTask, which will save
   *  the task-cpu state.
   *  The only function that should be called here is PickNext().
   */
  CPU_STATE_SYSTEM,

  /** This is the mode given to the CPU when driver is called.
   *  the mode is keeped for routines called from a drivers.
   *  Other function are not allowed and will raise a user-exception.
   */
  CPU_STATE_HARDWARE,

  CPU_STATE_COUNT,
};

#define KRN_LOADAVG_COUNT 3

enum TASK_STATE
{
  TASK_STATE_NONE = 0,
  TASK_STATE_ZOMBIE,
  TASK_STATE_BLOCKED,
  TASK_STATE_WAITING,
  TASK_STATE_EXECUTING,
  TASK_STATE_ABORTING,
  TASK_STATE_COUNT,
};

enum TASK_EVENT
{
  TASK_EVENT_NONE = 0,
  TASK_EVENT_SLEEP,
  TASK_EVENT_COUNT
};


enum LOCK
{
  LOCK_NONE,

  // Inodes
  LOCK_FS_LOOK,
  LOCK_FS_REGISTER,

  // Memory
  LOCK_VMA_MMAP,
  LOCK_VMA_GROW,

  // Scheduler
  LOCK_ATTACH_PROCESS,
  LOCK_PROCESS_CREATION,
  LOCK_PROCESS_ADD_THREAD,
  LOCK_PROCESS_DESTROY,
  LOCK_SCHED_INSERT,
  LOCK_SCHED_REMOVE,
  LOCK_TASK_DESTROY,
  LOCK_TASK_NEXT,
  LOCK_THREAD_ABORT,
  LOCK_EVENT_REGISTER,
  LOCK_TIMER_REGISTER_SLEEP,
  LOCK_TIMER_CANCEL_SLEEP,
  LOCK_TIMER_UP,


  // Syscalls
  LOCK_SYSFILE_FD,
};

#define KSTATS_PRECISION  (100)

#define KLOG_FS 0
#define KLOG_PF 0
#define KLOG_VFS 0
#define KLOG_SCH 0
#define KLOG_SYC 1

#endif /* KCONFIG_H__ */
