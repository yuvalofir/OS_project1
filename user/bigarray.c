#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

#define SIZE (1 << 16) // 2^16
#define NUM_CHILDREN 4

int arr[SIZE];

int
main(void)
{
  // Initialize the array with some values
  for (int i = 0; i < SIZE; i++) {
    arr[i] = i;
  }

  int pids[NUM_CHILDREN];
  int child_index = forkn(NUM_CHILDREN, pids);

  if (child_index < 0) {
    fprintf(2, "forkn failed\n");
    exit(1, "");
  }

  if (child_index == 0) {
    // we are in the parent process
    printf("Parent: created %d children\n", NUM_CHILDREN);
    for (int i = 0; i < NUM_CHILDREN; i++) {
      printf("Child PID: %d\n", pids[i]);
    }

    int n;
    int statuses[NPROC]; // array to store exit statuses of children

    if (waitall(&n, statuses) < 0) {
      fprintf(2, "waitall failed\n");
      exit(1, "");
    }

    int total_sum = 0;
    for (int i = 0; i < n; i++) {
      total_sum += statuses[i];
    }

    printf("Total sum from all children: %d\n", total_sum);
    exit(0, "");
  } else {
    // we are in a child process
    int start = (child_index - 1) * (SIZE / NUM_CHILDREN);
    int end = start + (SIZE / NUM_CHILDREN);
    int local_sum = 0;

    for (int i = start; i < end; i++) {
      local_sum += arr[i];
    }

    printf("Child %d calculated sum: %d\n", getpid(), local_sum);
    exit(local_sum, "");
  }
}
