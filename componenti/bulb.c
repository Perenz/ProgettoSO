#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(char **args){
    int id=getpid();

    printf("Lampadina creata\n");
    printf("Pid: %d\nPid padre: %d\n", getpid(), getppid());

    while(1);
    


    exit(0);
}