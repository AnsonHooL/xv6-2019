#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#define SUCCESS 0
#define FAIL -1
char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(-1);
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

int
getcmd(char *buf, int nbuf)
{
  fprintf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int  
getcommand(char cmd[], char** buf_p, int size)
{
    int i;
    for(i=0;i<size && **buf_p;i++)
    {
        if(!strchr(whitespace, **buf_p))
        {
            cmd[i] = **buf_p;
            (*buf_p)++;
        }
        else if(**buf_p == ' ')
        {
            (*buf_p)++;
            break;
        }
        else if(**buf_p == '\n')
        {
            break;
        }
        
        
    }
    cmd[i] = '\0';
    if(i==0) return FAIL;
    else return SUCCESS;   
}


void 
runcmd(char* buf)
{
    char* myargv[MAXARG+1];
    int myargc = 0;
    int pos = 0;
    char cmd[30];
    // printf("buf:%s\n",buf);
    int c = getcommand(cmd,&buf,sizeof cmd);
    if(c == SUCCESS)
    {
        myargv[myargc] = cmd;
        // printf("%s\n",cmd);
    }
    else
    {
        panic("command prase fail.");
    }
    // printf("buf:%s\n",buf);
    while(*buf)
    {
        if(!strchr(" \n|<>",*buf))
        {
            if(pos == 0) 
            {
                myargc++;
                myargv[myargc] = buf;
                pos++;
                buf++;
                // *(myargv[myargc]+pos) = *buf;
            }
            else
            {
                pos++;
                buf++;
            }

        }
        else if (*buf == ' ')
        {
            *(myargv[myargc]+pos) = '\0';
            pos = 0;
            buf++;
        }
        else if(*buf == '\n')
        {
            if(pos!=0)
                *(myargv[myargc]+pos) = '\0';
            pos = 0;
            break;
        }
        else if(*buf == '<')
        {
            if(pos!=0)
                *(myargv[myargc]+pos) = '\0';
            pos = 0;
            buf++;
            while (strchr(" ",*buf))
            {
                buf++;
            }
            char readfilename[20];
            int r = getcommand(readfilename,&buf,20);
            if(r == FAIL) panic("Redirect read file fail.\n");
            // printf("readfilename:%sxxx\n",filename);
            close(0);
            int fd = open(readfilename,O_RDONLY);
            if(fd < 0)
            {
                fprintf(2,"opne file:%s fail.\n",readfilename);
            }
        }
        else if(*buf == '>')
        {
            if(pos!=0)
                *(myargv[myargc]+pos) = '\0';
            pos = 0;
            buf++;
            while (strchr(" ",*buf))
            {
                buf++;
            }
            char writefilename[20];
            int r = getcommand(writefilename,&buf,20);
            if(r == FAIL) panic("Redirect read file fail.\n");
            // printf("writefilename:%sxxx\n",writefilename);
            close(1);
            int fd = open(writefilename,O_WRONLY|O_CREATE);
            if(fd < 0)
            {
                fprintf(2,"opne file:%s fail.\n",writefilename);
            }
        }
        else if(*buf == '|')
        {
            if(pos!=0)
                *(myargv[myargc]+pos) = '\0';
            pos = 0;
            buf++;
            while (strchr(" ",*buf))
            {
                buf++;
            }
            int mypipe[2];
            pipe(mypipe);
            int pid = fork1();
            if(pid > 0)
            {
                close(mypipe[0]);
                close(1);
                dup(mypipe[1]);
                myargv[myargc+1] = 0;
                for(int j =0;myargv[j];j++)
                {
                    // fprintf(2,"argv:%s\n",myargv[j]);
                }
                exec(cmd,myargv);
                exit(0);
                wait(0);
            }
            else
            {
               close(mypipe[1]);
               close(0);
               dup(mypipe[0]);
               runcmd(buf);
            }

        }
    }       
    
    myargv[myargc+1] = 0;
    for(int j =0;myargv[j];j++)
    {
        // fprintf(2,"argv:%s\n",myargv[j]);
    }
    exec(cmd,myargv);
}

int
main(void)
{
  static char buf[100];
  int fd;

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Chdir must be called by the parent, not the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(2, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
    {
        // printf("%s\n",buf);
        runcmd(buf);
    }
    //   runcmd(parsecmd(buf));
        
    wait(0);
  }
  exit(0);
}