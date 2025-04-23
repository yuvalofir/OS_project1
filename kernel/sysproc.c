#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  char exitMsg[32];
  int n;
  argint(0, &n);
  argstr(1, exitMsg, sizeof(exitMsg));
  exit(n, exitMsg);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}
uint64
sys_forkn(void)
{ 
  int n;
  uint64 pids_ptr;
  argint(0, &n);
  argaddr(1, &pids_ptr);
  // if the number of the child processes is not in the range [1, 16]
  if (n < 1 || n > 16)
    return -1;

  struct proc *p = myproc();
  int pids[n];
  int created = 0;

  for (int i = 0; i < n; i++) {
    int pid = fork(); // create a child process using the fork system call
    if (pid < 0) {
      // failure in creating a child process and cleaning them all up
      for (int j = 0; j < i; j++)
        kill(pids[j]);
      return -1;
    }
    if (pid == 0)  // child process
      return i + 1; // return the index of the child process

    pids[i] = pid;
    created++;
  }

  //just the parent process reaches here because the child processes has returned
  // copy the PIDs to the userspace array
  if (copyout(p->pagetable, pids_ptr, (char*)pids, sizeof(int) * n) < 0)
    return -1;

  return 0; // the parent process returns 0
}

uint64
sys_waitall(void)
{
  uint64 n_addr;         // pointer to int where we write number of exited children
  uint64 statuses_addr;  // pointer to int array for exit statuses
  argaddr(0, &n_addr);
  argaddr(1, &statuses_addr);

  struct proc *p = myproc();
  int num_exited = 0;
  int statuses[NPROC];

  // this loop is used to find all the children of the current process
  // and check if they are in the ZOMBIE state
  // if they are, we free them and store their exit status
  for (struct proc *pp = proc; pp < &proc[NPROC]; pp++) {
    if (pp->parent == p) {
      acquire(&pp->lock);
      if (pp->state == ZOMBIE) {
        statuses[num_exited] = pp->xstate;
        freeproc(pp);
        release(&pp->lock);
        num_exited++;
      } else {
        release(&pp->lock);
      }
    }
  }
  // if no children have exited, we return 0
  if (num_exited == 0) {
    if (copyout(p->pagetable, n_addr, (char *)&num_exited, sizeof(int)) < 0)
      return -1;
    return 0;
  }

  // copy the number of exited children to the userspace variable
  if (copyout(p->pagetable, n_addr, (char *)&num_exited, sizeof(int)) < 0)
    return -1;
  // copy the exit statuses to the userspace array
  if (copyout(p->pagetable, statuses_addr, (char *)statuses, num_exited * sizeof(int)) < 0)
    return -1;

  return 0;

}


uint64
sys_wait(void)
{
  uint64 p;
  uint64 msg_addr;
  argaddr(0, &p);
  argaddr(1, &msg_addr);
  return wait(p, msg_addr);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_memsize(void) { 
  return myproc()->sz;
}
