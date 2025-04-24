#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define ARRAY_SIZE (1 << 16)  // 2^16 = 65536
#define NUM_CHILDREN 4

int main() {
  int *arr = malloc(ARRAY_SIZE * sizeof(int));
  if (!arr) {
      printf("malloc failed\n");
      exit(1, "oom");
  }
  int child_pids[NUM_CHILDREN];
  int n_finished;
  int statuses[NPROC];
  int child_index;
  int i, start, end;
  int sum = 0;
  
  // Initialize array with consecutive numbers
  for (i = 0; i < ARRAY_SIZE; i++) {
    arr[i] = i;
  }
  int ans = forkn(NUM_CHILDREN, child_pids);
  // Fork NUM_CHILDREN child processes
  if (ans  < 0) {
    printf("Error: forkn failed\n");
    exit(1, "forkn failed");
  }
  else if (ans == 0) {
    // This is the parent process
    
    
    // Wait for all children to finish
    if (waitall(&n_finished, statuses) < 0) {
      printf("Error: waitall failed\n");
      exit(1, "waitall failed");
    }
    printf("Parent created %d child processes:\n", NUM_CHILDREN);
    // Print child PIDs
    for (i = 0; i < NUM_CHILDREN; i++) {
      printf("%d\n", child_pids[i]);
    }

    // Verify that all children finished
    if (n_finished != NUM_CHILDREN) {
      printf("Error: expected %d children, but only %d finished\n", 
             NUM_CHILDREN, n_finished);
      exit(1, "child count mismatch");
    }
    
    // Calculate total sum from child exit statuses
    sum = 0;
    for (i = 0; i < n_finished; i++) {
      sum += statuses[i];
    }
    
    printf("Total sum: %d\n", (int)sum);
    printf("Expected sum: %llu\n", ((unsigned long long)ARRAY_SIZE * (ARRAY_SIZE - 1)) / 2);
    
    exit(0, "Calculation completed");
  }
  else {
    child_index = ans;
    // This is a child process - calculate partial sum
    start = (child_index - 1) * (ARRAY_SIZE / NUM_CHILDREN);
    end = child_index * (ARRAY_SIZE / NUM_CHILDREN);
    
    // Handle any remainder in the last child
    if (child_index == NUM_CHILDREN)
      end = ARRAY_SIZE;
    
    // Calculate sum for this section
    sum = 0;
    for (i = start; i < end; i++) {
      sum += arr[i];
    }
    
    printf("Child %d (PID %d) calculated sum %d for elements %d to %d\n", 
           child_index, getpid(), (int)sum, start, end - 1);
    
    
    // Return sum as exit status
    exit(sum, "");
  } 
  
  return 0;
}