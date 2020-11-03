#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
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

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

typedef void(*handlertype)();

uint64
sys_sigalarm(void)
{
  // struct proc *p = myproc();
  uint64 handler;
  int interval;
  if(argint(0,&interval)<0){
    return -1;
  }
  if(argaddr(1,&handler)<0){
    return -1;
  }
  myproc()->handler = (handlertype)handler;
  myproc()->tickinterval = interval;
 
  
  // printf("handler %p\n",handler);
  // printf("interval %d\n",interval); 
  // p->handler = (handlertype)addr;
  // p->tickinterval = interval;
  // printf("i %d\n",p->tickinterval);
  // printf("h %p\n",p->handler);

  // uint64 addr;
  // void (*handler)();
  // int n;
  // argint(0,&n);
  // argaddr(1,&addr);
  // handler = (functype)addr;
  // printf("hello\n");
  return 0;
}

uint64
sys_sigreturn(void)
{
  // myproc()->tf->epc = myproc()->timepc;
  *(myproc()->tf) = myproc()->oldframe;
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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
