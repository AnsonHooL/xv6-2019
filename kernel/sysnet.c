//
// network system calls.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "net.h"

struct sock {
  struct sock *next; // the next socket in the list
  uint32 raddr;      // the remote IPv4 address
  uint16 lport;      // the local UDP port number
  uint16 rport;      // the remote UDP port number
  struct spinlock lock; // protects the rxq
  struct mbufq rxq;  // a queue of packets waiting to be received
};

static struct spinlock lock;
static struct sock *sockets; //头插法，维护系统的socket队列

void
sockinit(void)
{
  initlock(&lock, "socktbl");
}

int
sockalloc(struct file **f, uint32 raddr, uint16 lport, uint16 rport)
{
  struct sock *si, *pos;

  si = 0;
  *f = 0;
  if ((*f = filealloc()) == 0)
    goto bad;
  if ((si = (struct sock*)kalloc()) == 0)
    goto bad;

  // initialize objects
  si->raddr = raddr;
  si->lport = lport;
  si->rport = rport;
  initlock(&si->lock, "sock");
  mbufq_init(&si->rxq);
  (*f)->type = FD_SOCK;
  (*f)->readable = 1;
  (*f)->writable = 1;
  (*f)->sock = si;

  // add to list of sockets
  acquire(&lock);
  pos = sockets;
  while (pos) {
    if (pos->raddr == raddr &&
        pos->lport == lport &&
	pos->rport == rport) {
      release(&lock);
      goto bad;
    }
    pos = pos->next;
  }
  si->next = sockets;
  sockets = si;
  release(&lock);
  return 0;

bad:
  if (si)
    kfree((char*)si);
  if (*f)
    fileclose(*f);
  return -1;
}

//
// Your code here.
//
// Add and wire in methods to handle closing, reading,
// and writing for network sockets.
//
int
sockwrite(struct sock* f, uint64 srca, uint len)
{
  if(len > MBUF_SIZE - MBUF_DEFAULT_HEADROOM - 20)
  {
    printf("socket write too much.\n");
    return -1;
  }

  struct mbuf* m = mbufalloc(MBUF_DEFAULT_HEADROOM); //分配足够空间写UDP、IP、以太网帧头

  if(copyin(myproc()->pagetable, m->head, srca, len) == -1) // 用户空间 写入 kernel
  {
    mbuffree(m);
    return -1;
  }
  mbufput(m,len);

  net_tx_udp(m, f->raddr, f->lport, f->rport); //使用udp协议将 应用层的数据传输

  return len;
}

int
sockclose(struct sock* f)
{
  acquire(&lock);
  struct sock* pos = sockets;
  if(pos == f)
  {
    sockets = sockets->next;
  }
  else
  {
    while (pos->next)
    {
      if(pos->next == f)
      {
        pos->next = f->next;
        break;
      }
      pos = pos->next;
    }  
  }
  release(&lock); 
  while (!mbufq_empty(&f->rxq))
  {
    struct mbuf* m = mbufq_pophead(&f->rxq);
    if(m)
      mbuffree(m);
  }
  if(f)
    kfree(f);
  
  return 0;
}

int
sockread(struct sock* f, uint64 dsta, uint len)
{
  // acquire(&lock);
  
  acquire(&f->lock);
  while (mbufq_empty(&f->rxq))
  {
    // printf("sleep\n");
    sleep(f, &f->lock);   //消费者等待网络数据
    // printf("wake\n");
  }

  if(len < f->rxq.head->len) //一次读取的数据少于，一个UDP包存放的数据
  {
    char*src = mbufpull(f->rxq.head, len);
    if(copyout(myproc()->pagetable, dsta, src, len) == -1)
    {
      return -1;
    }
    return len;
  }
  else //一次读取的数据大于等于，一个UDP包存放的数据，还是只读取一个包，并返回实际读取数据
  {
    struct mbuf* m = mbufq_pophead(&f->rxq);
    int readlen = m->len;
    if(copyout(myproc()->pagetable, dsta, m->head, readlen) == -1)
    {
      mbuffree(m);
      release(&f->lock);
      return -1;
    }
    mbuffree(m);
    release(&f->lock);
    return readlen;
  }
}

// called by protocol handler layer to deliver UDP packets
void
sockrecvudp(struct mbuf *m, uint32 raddr, uint16 lport, uint16 rport)
{
  //
  // Your code here.
  //
  // Find the socket that handles this mbuf and deliver it, waking
  // any sleeping reader. Free the mbuf if there are no sockets
  // registered to handle it.
  //

  /** Test receive ping packet**/
  // static int num = 0;
  // printf("receive %d : %s\n",num++,m->head);

  acquire(&lock);
  struct sock* pos = sockets;
  while (pos)
  {
    if (pos->raddr == raddr &&  pos->lport == lport && pos->rport == rport) 
    {
      acquire(&pos->lock);

      mbufq_pushtail(&pos->rxq, m); //找到udp包->对应的socket，加入到其读取mbuf队列中
    
      wakeup(pos);          //唤醒

      release(&pos->lock);
      release(&lock);  

      return;
    }  
    pos = pos->next;
  }

  release(&lock);
  mbuffree(m);
}
