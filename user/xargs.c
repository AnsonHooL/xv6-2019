#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

void execommand(char* command,int argc,char* argv[])
{
  char tmp;
  int newargc = argc;
  int pos = 0;
  char* newargv[MAXARG] = {0};

  for(int i=0;i<argc && i<MAXARG;i++)
  {
      newargv[i] = argv[i];
  }

  while (newargc <= MAXARG)
  {
      if(read(0,&tmp,1))
      {
          if(tmp != '\n')
          {
            if(pos == 0) newargv[newargc] = (char*)malloc(500);
            *(newargv[newargc] + pos) = tmp;
            pos++;
          }
          else if(tmp == '\n')
          {
              *(newargv[newargc] + pos) = '\0';
              newargc++;
              pos = 0;
              int pid = fork();
              if(pid == 0)
              {
                exec(command,newargv);   //子进程执行命令         
              }
              else if(pid > 0)
              {
                  wait();
                  execommand(command,argc,argv);
                  exit();
              }
              else
              {
                  printf("wrong fork.\n");
              }
          }
          
      }
      else
      {
          exit();
      }
      
  }
  printf("too many arguments.\n");
}

int
main(int argc, char *argv[])
{
  int i,nweargc=0;
  char* newargv[MAXARG] = {0};

  if(argc < 2){
    printf("xargs [command] [arguments...].\n");
    exit();
  }
  nweargc = argc - 1;
  for(i=0;i<nweargc && i<MAXARG;i++)
  {
      newargv[i] = argv[i+1];
  }
  execommand(argv[1],nweargc,newargv);
  exit();
}