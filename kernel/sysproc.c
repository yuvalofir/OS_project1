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
    int pid = fork();
    if (pid < 0) {
      // failure in creating a child process and cleaning them up
      for (int j = 0; j < i; j++)
        kill(pids[j]);
      return -1;
    }
    if (pid == 0)  // תהליך בן
      return i + 1;

    pids[i] = pid;
    created++;
  }

  // רק ההורה מגיע לפה – מעתיקים את הפידים למשתמש
  if (copyout(p->pagetable, pids_ptr, (char*)pids, sizeof(int) * n) < 0)
    return -1;

  return 0; // הורה מחזיר 0
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
