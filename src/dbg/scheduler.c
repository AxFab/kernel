#include <scheduler.h>
#include <kinfo.h>

void top ()
{
  printf(" TOP - %lld        -- Load average: %.2f, %.2f, %.2f\n", ltime(NULL),
    kSYS.loadAvg_[0], kSYS.loadAvg_[1], kSYS.loadAvg_[2]);

  printf("Tasks: %d total, %d running, %d waiting, %d blocked, %d zombie\n",
    kSYS.taskCount_[0], kSYS.taskCount_[TASK_STATE_EXECUTING],
    kSYS.taskCount_[TASK_STATE_WAITING], kSYS.taskCount_[TASK_STATE_BLOCKED],
    kSYS.taskCount_[TASK_STATE_ZOMBIE]);


}

void syscall()
{

}


int main ()
{
  // NEW
  ltime_t     lastTicks_;

  ksch_create_process (NULL, NULL);
  ksch_init ();

  while (kSYS.state_ == 0)
  {
    assert (knolock());
    if (lastTicks_ < ltime(NULL)) {
      lastTicks_ += 10;
      ksch_ticks();
      top();
    } else {
      syscall();
    }

  }

}


