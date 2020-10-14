#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


int
main(int argc, char *argv[])
{
  int i=0;
  i++;
  i = 10*10;
  int c =20+i;
  for(int j = 0;j<100;j++)
  {
      printf("hell:%d\n",j);
  }
  printf("hell:%d\n",i+c);

  exit(0);
}
