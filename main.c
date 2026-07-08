#include "rmalloc.h"

int main(void) {
  // hello
  int x = 10;
  int *ptr = (int *)rmalloc(sizeof(x));
  int *ptr2 = rmalloc(sizeof(x)*3);
  int *ptr4 = rmalloc(sizeof(x)*3);
  int *ptr3 = rmalloc(sizeof(x)*3);
  rfree(ptr);
  rfree(ptr4);
  rfree(ptr3);
  rfree(ptr2);
  int *ptr5 = rmalloc(sizeof(x)*3);
  rfree(ptr5);
  return 0;
}


