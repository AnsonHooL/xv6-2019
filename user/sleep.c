#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{

  if(argc != 2){
    fprintf(2, "usage: sleep n(seconds)...\n");
    exit(1);
  }
//   int i = uptime();
//   printf("time:%d\n",i);
  sleep(atoi(argv[1]));
//   i = uptime();
//   printf("time:%d\n",i);
  printf("sleep done.\n");
  exit(0);
}
