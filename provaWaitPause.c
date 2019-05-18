#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

void sighandler(int sig){
    if(sig==SIGCONT){
        return;
    }
    else if(sig==SIGTSTP){
        pause();
    }
}

int main(){
    signal(SIGCONT, sighandler);
    signal(SIGTSTP, sighandler);

    /*int i=0;
	while(1){
        printf("Aspettato %d\n", ++i);
		sleep(5);		
	}*/
    int msec = 0, trigger = 10000; /* 10ms */
    clock_t before = clock();

    do {
    /*
    * Do something to busy the CPU just here while you drink a coffee
    * Be sure this code will not take more than `trigger` ms
    */

    clock_t difference = clock() - before;
    msec = difference * 1000 / CLOCKS_PER_SEC;
    } while ( msec < trigger );

    printf("Time taken %d seconds %d milliseconds\n",
    msec/1000, msec%1000);
}