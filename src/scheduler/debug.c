#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <kernel/info.h>


// #define PRINT_TIME_FORMAT "%04d-%02d-%01d-%02d:%02d:%02d"
// #define PRINT_TIME(t) (int)(t / 31556520), (int)((t % 31556520)/86400)/7, (int)((t % 31556520)/86400)%7, (int)(((t % 31556520)%86400)/3600), (int)(((t % 31556520)%3600)/60), (int)(((t % 31556520)%60))

#define PRINT_TIME_FORMAT "%4d-%02d:%02d:%02d"
#define PRINT_TIME(t) (int)(t/86400), (int)((t%86400)/3600), (int)((t%3600)/60), (int)(t%60)


void task_print () 
{
  // kProcess_t* proc = kSYS.allProcFrst_;
  // kprintf ("PID   THREADS\n");
  // kprintf ("          TID   STATE   \n");

  // while (proc) {
  //   kprintf ("%3d   %3d\n", proc->pid_, proc->runningTask_ );
  //   kThread_t* thread = proc->threadFrst_;
  //   while (thread) {
  //     kprintf ("          %3d   %3d   %d   "PRINT_TIME_FORMAT"\n", thread->taskId_, thread->state_, 
  //       kislocked(&thread->lock_), PRINT_TIME(thread->elapsedUser_));
  //     thread = thread->nextPr_;
  //   }

  //   proc = proc->nextAll_;
  // }

  // int i;
  // kprintf("\n");
  // for (i=0; i < TASK_STATE_COUNT; ++i)
  //   kprintf( "-- %d] %d \n", i, kSYS.tasksCount_[i] );
}


