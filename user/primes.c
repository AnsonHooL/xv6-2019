#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void receive(int p[2])
{
    int num,firstnumber;
    close(p[1]);
    // printf("in\n");
    int bytes = read(p[0],&num,sizeof(int));
    // printf("out\n");
    if(bytes>0) 
    {   
        firstnumber = num;
        printf("prime %d\n",num);
    }
    else
    {
        
        exit();
    } 
    int newp[2];
    pipe(newp);
    int pid = fork();
    if(pid == 0)
    {  
       close(newp[0]);
       while(1)
        {
            int c = read(p[0],&num,sizeof(int));
            // printf("R\n");
            if(c <= 0) break;
            if(num % firstnumber != 0)
            {
                write(newp[1],&num,sizeof(int));
                
            }
        }
        close(p[0]);
        close(newp[1]);
        exit();
    }
    else if(pid > 0)
    {
        receive(newp);
        exit();
    }
    else
    {
        printf("fail fork");
    }   
}


int
main(int argc, char **argv)
{
    int p[2];
    pipe(p);
    int pid = fork();
    if(pid == 0)
    {
        close(p[0]);
        for(int i=2;i<=35;i++)
        {
            write(p[1],&i,sizeof(int));
            // printf("W\n");
        }
        close(p[1]);
    }
    else if(pid > 0)
    {
        receive(p);
    }
    else
    {
        printf("fail fork");
    }
    exit();
}