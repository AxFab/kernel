#include <kernel/core.h>
#include <kernel/task.h>


// TODO Domain means local or use a service, we should have something for 
// service.

kUser_t* search_user (const char* name, const char* domain) 
{
  kUser_t* user;
  for_each (user, &kSYS.userList_, kUser_t, allNd_) {
    if (strcmp (user->name_, name) == 0)
      return user;
  }

  return NULL;
}

kUser_t* create_user (const char* name, int privileges)
{
  kUser_t* user = search_user(name, NULL);
  if (user != NULL) {
    __seterrno(EEXIST);
    return NULL;
  }

  user = KALLOC(kUser_t);
  user->name_ = kstrdup(name);
  user->privileges_ = privileges;
  klist_push_back(&kSYS.userList_, &user->allNd_);
  return user;
}

void destroy_user (kUser_t* user)
{
  assert (user->processCount_ == 0);
  kfree(user->name_);
  kfree(user);
}


