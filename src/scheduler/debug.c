#include <kernel/vfs.h>
#include <kernel/scheduler.h>
#include <kernel/info.h>


// #define PRINT_TIME_FORMAT "%04d-%02d-%01d-%02d:%02d:%02d"
// #define PRINT_TIME(t) (int)(t / 31556520), (int)((t % 31556520)/86400)/7, (int)((t % 31556520)/86400)%7, (int)(((t % 31556520)%86400)/3600), (int)(((t % 31556520)%3600)/60), (int)(((t % 31556520)%60))

#define PRINT_TIME_FORMAT "%4d-%02d:%02d:%02d"
#define PRINT_TIME(t) (int)(t/86400), (int)((t%86400)/3600), (int)((t%3600)/60), (int)(t%60)


void task_print () 
{
  kThread_t* thread;
  kProcess_t* process;
  kprintf ("PID   THREADS\n");
  kprintf ("          TID   STATE   IS LOcKED    ELAPSED\n");

  for_each (process, &kSYS.processes_, kProcess_t, procNd_) {
    kprintf ("%3d   %3d\n", process->pid_, process->runningTask_ );
    for_each (thread, &process->threads_, kThread_t, procNd_) {
      kprintf ("          %3d   %3d   %d   "PRINT_TIME_FORMAT"\n", thread->taskId_, thread->state_, 
        kislocked(&thread->lock_), PRINT_TIME(thread->elapsedUser_));
    }
  }
  
  int i;
  kprintf("\n");
  for (i=0; i < TASK_STATE_COUNT; ++i)
    kprintf( "-- %d] %d \n", i, kSYS.tasksCount_[i] );
}


