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

uint64
sys_forkn(void)
{
  int n;
  uint64 pids_ptr;
  argint(0, &n);
  argaddr(1, &pids_ptr);
  return forkn((int)n, (int*)pids_ptr);
}

uint64
sys_waitall(void)
{
  uint64 n_ptr;
  uint64 statuses_ptr;
  argaddr(0, &n_ptr);
  argaddr(1, &statuses_ptr);
  return waitall((int*)n_ptr, (int*)statuses_ptr);
}
