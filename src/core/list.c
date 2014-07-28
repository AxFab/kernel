#include <kernel/core.h>


// Linked List ------------------------------------------

typedef struct klist klist_t;

// #define offsetof (type,member)  ((int)(&((type*)0)->member))
#define klist_dataof(ptr, type, member) \
    ((type) (((char*)(ptr)) - offsetof(type,member)))

void klist_init(klist_t* head);
void klist_add (klist_t* new, klist_t* head);
void klist_addback (klist_t* new, klist_t* head);
void klist_del (klist_t* node);
void klist_replace (klist_t* old, klist_t* new);

// -------------------------------------------------------

struct klist {
  klist_t*  prev_;
  klist_t*  next_;
};

void klist_init(klist_t* head)
{
  head->next_ = head;
  head->prev_ = head;
}

void klist_add (klist_t* new, klist_t* head)
{
  head->next_->prev_ = new;
  new->next_ = head->next_;
  new->prev_ = head;
  head->next_ = new;
}

void klist_addback (klist_t* new, klist_t* head)
{
  head->prev_->next_ = new;
  new->prev_ = head->prev_;
  new->prev_ = head;
  head->prev_ = new;
}

void klist_del (klist_t* node)
{
  node->next_->prev_ = node->prev_;
  node->prev_->next_ = node->next_;
  node->prev_ = node->next_ = PTR_POISON;
}

void klist_replace (klist_t* old, klist_t* new)
{
  new->next_ = old->next_;
  new->next_->prev_ = new;
  new->prev_ = old->prev_;
  new->prev_->next_ = new;
}

