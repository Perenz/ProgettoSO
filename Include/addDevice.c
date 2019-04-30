#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int add_device(char*, NodoPtr);
void sign_handler(int);

//device list
char *builtin_device[]={
        "bulb",
        "window",
        "fridge",
        "timer",
        "hub"
};

char *bultin_dev_path[]={
        "./componenti/BULB",
        "./componenti/WINDOW",
        "./componenti/FRIDGE",
        "./componenti/TIMER",
        "./componenti/HUB"
};

int device_number(){
        return sizeof(builtin_device)/sizeof(char*);
}

//Signal handler per gestire l'arrivo di segnali
void sign_handler(int sig){
        return;
}

int add_device(char* execPath, NodoPtr procList){
        pid_t pid, wpid;
        int fd[2]; //fd[0]legge fd[1]scrive
        pipe(fd); //creo la pipe
        
        pid=fork();
        if (pid == 0) {
                // Child process
                char *args[]={execPath,NULL,NULL}; 
                sprintf(args[1],"%d",fd[1]);
                printf("\naaaa%s\n",args[1]);




                execvp(args[0],args); //passo gli argomenti incluso il puntatore al lato di scrittura della pipe
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        else{
                //Parent process
                //Indico al processo corrente (il padre) di gestire segnali in entrata di tipo SIGCONT con la funzione sign_handler
                signal(SIGCONT, sign_handler);
                close(fd[1]); //chiudo il lato di scrittura;
                //Aggiungo alla lista dei processi quello appena generato, identificato dal suo pid
                insertLast(procList, pid);
                char* msg = NULL;
                
                //Vado in pausa per permettere al figlio di generarsi
                pause();



                
                read(fd[0],msg,1024);
                printf("\n\n\n\ndebug msg: %s\n\n\n",msg);
                
        }
        
        return 1;
}

int cen_add(char **args, NodoPtr procList){
        //Implementazione del comando spostabile in un altro file.c
        if(args[1]==NULL){
                printf("Argomenti non validi\n");
                printf("Utilizzo: add <device>\n");
                printf("Comando 'device' per vedere la lista di quelli disponibili\n");
                return 1;
        }
        //3 devide disponibili: bulb, window, fridge
        else{
            int i=0;
            for(i=0; i<device_number(); i++){
                if(strcmp(args[1], builtin_device[i])==0)
                        return add_device(bultin_dev_path[i], procList);
            }
        }

        printf("Device indicato non riconosciuto\n");
        printf("Utilizzo: add <device>\n");
        printf("Digitare il comando 'device' per la lista di quelli disponibili\n");

        return 1;        
}

