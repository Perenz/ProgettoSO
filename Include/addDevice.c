#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int add_device(char*, NodoPtr, NodoPtr);
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

int add_device(char* execPath, NodoPtr procList, NodoPtr dispList){
        pid_t pid, wpid;
        int fd[2];
        pipe(fd); //creo la pipe
        
        pid=fork();
        
        if (pid == 0) {
                // Child process
                char fdTmp[10];
                //close(fd[0]);

                sprintf(fdTmp,"%d",fd[1]);
                char *args[]={execPath,fdTmp,NULL}; 



                execvp(args[0],args); //passo gli argomenti incluso il puntatore al lato di scrittura della pipe
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        else{
                //Parent process
                //Indico al processgit po corrente (il padre) di gestire segnali in entrata di tipo SIGCONT con la funzione sign_handler
                signal(SIGCONT, sign_handler);
                //close(fd[1]); //chiudo il lato di scrittura;
                //Aggiungo alla lista dei processi quello appena generato, identificato dal suo pid
                //Devo aggiungere anche i fd per la pipe
                
                //DEBUG
                insertLast(dispList, pid, fd);
                
                /**
                 * TODO
                 * Nella funzione insertLast devo anche passare i FD cosi che vengano aggiunti al nodo
                 * */
                
                
                //Vado in pausa per permettere al figlio di generarsi
                pause();



                //char msg[30];
                // int rev = read(fd[0],msg,512);
                // printf("\nesito: %d\n", rev);
                // printf("\n\n\n\ndebug msg: %s\n\n\n",msg);
                
        }
        
        return 1;
}

int cen_add(char **args, NodoPtr procList, NodoPtr dispList){
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
                        return add_device(bultin_dev_path[i], procList, dispList);
            }
        }

        printf("Device indicato non riconosciuto\n");
        printf("Utilizzo: add <device>\n");
        printf("Digitare il comando 'device' per la lista di quelli disponibili\n");

        return 1;        
}

