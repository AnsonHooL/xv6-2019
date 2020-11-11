#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
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

uint64
sys_mmap(void)
{
  uint64 mmapaddress,mmapsize; //mmaaddress == 0 
  int prot,flags,fd;
  int offset;
  int i;

  if(argaddr(0, &mmapaddress) < 0)
    return -1;
 
  if(argaddr(1, &mmapsize) < 0)
    return -1;
  
  if(argint(2, &prot) < 0)
    return -1;

  if(argint(3, &flags) < 0)
    return -1;
  
  if(argint(4, &fd) < 0)
    return -1;

  if(argint(5, &offset) < 0)
    return -1;

  
  
  struct proc *p = myproc();

  if(prot & PROT_WRITE && !(flags & MAP_PRIVATE))
  {
    if(p->ofile[fd]->writable == 0)
      return -1;
  }

  for(i = 0;i < MMAPNUM; i++)
  {
    if((p->mymmap)[i].valid)
    {
      (p->mymmap)[i].valid = 0;
      break;
    }
  }

  if(i == MMAPNUM)
  {
    printf("mmap alloc fail\n");
    return -1;
  }
  struct inode *ip;

  p->mymmap[i].prot     = prot;
  p->mymmap[i].flags    = flags;
  p->mymmap[i].uppermap = p->upperlevel;
  p->mymmap[i].downmap  = p->upperlevel - mmapsize;
  p->mymmap[i].offset   = offset;
  
  ip = (p->ofile[fd])->ip;
  ip->ref++;
  // begin_op(ROOTDEV);
  // ilock(ip);
  // ip->nlink++;
  // iupdate(ip);
  // iunlock(ip);
  // end_op(ROOTDEV);

  // printf("mmap file size:%d\n",ip->size);
  // printf("mmap low address:%p\n",p->mymmap[i].downmap);
  // printf("mmap high address:%p\n",p->mymmap[i].uppermap);
  p->mymmap[i].fileinode = ip;

  p->upperlevel = p->upperlevel - PGROUNDUP(mmapsize);

  return p->mymmap[i].downmap;
}




uint64
sys_munmap(void)
{
  uint64 address;
  int size;
  int i;
  struct proc *p = myproc();

  if(argaddr(0, &address) < 0)
    return -1;
 
  if(argint(1, &size) < 0)
    return -1;

  for(i=0; i<MMAPNUM; i++)
  {
    if(address >= p->mymmap[i].downmap && address < p->mymmap[i].uppermap)
    {
      break;
    }
  }

  if(i == MMAPNUM)
  {
    printf("unmmap alloc fail\n");
    return -1;
  }

  if(p->mymmap[i].prot & PROT_WRITE && !(p->mymmap[i].flags & MAP_PRIVATE))
  {
    //TODO
    struct inode *ip = p->mymmap[i].fileinode;
    begin_op(ROOTDEV);
    ilock(ip);
    uint offset = PGROUNDDOWN(address) - p->mymmap[i].downmap + p->mymmap[i].offset;
    writei(ip,1,PGROUNDDOWN(address),offset,size);
    iunlock(ip);
    end_op(ROOTDEV);
  }

  uvmunmap(p->pagetable, PGROUNDDOWN(address), size, 1);

  if(address == p->mymmap[i].downmap)
  {
    p->mymmap[i].downmap += size; 
    p->mymmap[i].offset  += size;
  }

  if(p->mymmap[i].downmap  == p->mymmap[i].uppermap)
  {
    //todo
    struct inode *ip = p->mymmap[i].fileinode;
    begin_op(ROOTDEV);
    // ilock(ip);
    // ip->nlink--;
    // iupdate(ip);
    // iunlock(ip);
    iput(ip);
    end_op(ROOTDEV);
    p->mymmap[i].valid = 1;
  }

  return 0;
}