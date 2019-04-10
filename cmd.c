#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(char **args){
    printf("Questo è il nuovo processo creato\nCOMPILAZIONE DA MIKEFILE APPORTATA\n");
    printf("Pid: %d\nPid padre: %d\n", getpid(), getppid());

    exit(0);
}