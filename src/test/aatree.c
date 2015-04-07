#include <kernel/aatree.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Obj Obj_t;
struct Obj {
  aanode_t node_;
  const char *name_;
};

Obj_t o [256];

aatree_t tree;

int verbose = 0;

void printo (int *ref, Obj_t *o)
{
  if (o->node_.left_)
    printo (ref, (Obj_t *)o->node_.left_);

  if (*ref > o->node_.value_)
    printf ("ERROR\n");

  if (verbose)
    printf ("  %5ld \t %s (%d)\n", o->node_.value_, o->name_, o->node_.level_);

  *ref = o->node_.value_;

  if (o->node_.right_)
    printo (ref, (Obj_t *)o->node_.right_);
}


void set_obj (int idx, long value, const char *name)
{
  if (verbose)
    printf ("ACTION -- Set object [%d] for %s <%ld>\n", idx, name, value);

  o[idx].node_.value_ = value;
  o[idx].name_ = name;

  aa_insert (&tree, &o[idx].node_);

  int ref = 0;
  printo (&ref, (Obj_t *)tree.root_);
}


void rm_obj (int idx)
{
  if (verbose)
    printf ("ACTION -- Remove object [%d] for %s <%ld>\n", idx, o[idx].name_, o[idx].node_.value_);

  aa_delete (&tree, &o[idx].node_);

  int ref = 0;
  printo (&ref, (Obj_t *)tree.root_);
}

void throw()
{
}

int main ()
{

  set_obj (0, 84, "A");
  set_obj (1, 945, "B");
  set_obj (2, 5, "C");
  set_obj (3, 15, "D");
  set_obj (4, 235, "E");
  set_obj (5, 3, "F");
  set_obj (6, 1265, "G");
  set_obj (7, 546, "H");
  // rm_obj();
  rm_obj(5);
  set_obj (5, 783, "I");
  rm_obj(6);
  set_obj (6, 763, "J");
  rm_obj(4);
  verbose = 1;
  set_obj (4, 7, "K");

  return 0;
}














