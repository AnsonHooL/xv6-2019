#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int
main(int argc, char **argv)
{
  int pid;
  int parent_pipe[2];
  int child_pipe[2];
  pipe(parent_pipe);
  pipe(child_pipe);
  pid = fork();
  if(pid>0)
  {
     char buf[20];
     read(parent_pipe[0],buf,sizeof(buf));
     printf("%d: received ",getpid());
     printf("%s\n",buf);
     write(child_pipe[1],"pong\n",4);
     close(parent_pipe[0]);
     close(child_pipe[0]);
     close(parent_pipe[1]);
     close(child_pipe[1]);
     wait();   
  }
  else if(pid == 0)
  {
     char buf[20];
     write(parent_pipe[1],"ping",4);
     read(child_pipe[0],buf,sizeof(buf));
     printf("%d: received ",getpid());
     printf("%s\n",buf);
     close(parent_pipe[0]);
     close(child_pipe[0]);
     close(parent_pipe[1]);
     close(child_pipe[1]); 
  }
  else
  {
      printf("fork error!\n");
  }
  exit();
}