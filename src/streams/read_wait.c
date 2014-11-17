#include <kernel/streams.h>
#include <kernel/scheduler.h>

int stream_wait_regist(kTask_t* task)
{
  int fd = (int)task->eventParam_;
  kStream_t* stream = kstm_get_fd (fd, R_OK);
  if (stream == NULL) 
    return __seterrno(EBADF);


  return __noerror();
}


int stream_wait_cancel(kTask_t* task)
{

}

int stream_wait_trigger(long param1, long param2)
{

}
