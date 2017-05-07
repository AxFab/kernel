#include <klib/memory.h>
#include <check.h>


// Test mmap a source file, mmap stacks, mmap library, mmap heap,
// mmap stacks, mmap heap, mmap a file, unmap the file, mmap a heap,
START_TEST(test_memory_1) {

  memvmap_t *vma1, *vma2;
  memspace_t *mspace = Memory_create();
  ck_assert(0 == Memory_append(mspace, NULL, 4*_Mib_, NULL, 0, 0, &vma1));
  printf("get address: %08zx-%08zx\n", vma1->node_.value_, vma1->length_);
  ck_assert(0 == Memory_append(mspace, NULL, 4*_Mib_, NULL, 0, 0, &vma2));
  printf("get address: %08zx-%08zx\n", vma2->node_.value_, vma2->length_);
} END_TEST


// Test mmap file, mmap stack, mmap heap until crash

// Test mmap stack, unmap unexisting heap


void fixture_memory(Suite *s) {
  TCase *tc = tcase_create("Memory");
  tcase_add_test(tc, test_memory_1);
  suite_add_tcase(s, tc);
}
