#include "kernel/types.h"
#include "user/user.h"

int main() {
  printf("Initial mem size: %d\n", memsize());
  void *ptr = malloc(20000); 
  printf("After malloc mem size: %d\n", memsize());
  free(ptr);
  printf("After free mem size: %d\n", memsize());
  exit(0, "main returned");
}
