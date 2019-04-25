#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int dev_bulb();
int dev_window();
int dev_fridge();
int dev_timer();
int dev_hub();

//device list
char *builtin_device[]={
        "bulb",
        "window",
        "fridge",
        "timer",
        "hub"
};

int (*builtin_addDevice[]) () = {
        &dev_bulb,
        &dev_window,
        &dev_fridge,
        &dev_timer,
        &dev_hub
};

int device_number(){
        return sizeof(builtin_device)/sizeof(char*);
}

int dev_bulb(){
        pid_t pid, wpid;

        pid = fork();
        if (pid == 0) {
                // Child process
                char *args[]={"./componenti/BULB",NULL};
                execvp(args[0],args);
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        return 1;
}

int dev_window(){
        pid_t pid, wpid;

        pid = fork();
        if (pid == 0) {
                // Child process
                char *args[]={"./componenti/WINDOW",NULL};
                execvp(args[0],args);
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        return 1;
}

int dev_fridge(){
        pid_t pid, wpid;

        pid = fork();
        if (pid == 0) {
                // Child process
                char *args[]={"./componenti/FRIDGE",NULL};
                execvp(args[0],args);
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        return 1;
}

int dev_timer(){
        pid_t pid, wpid;

        pid = fork();
        if (pid == 0) {
                // Child process
                char *args[]={"./componenti/TIMER",NULL};
                execvp(args[0],args);
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        return 1;
}

int dev_hub(){
        pid_t pid, wpid;

        pid = fork();
        if (pid == 0) {
                // Child process
                char *args[]={"./componenti/HUB",NULL};
                execvp(args[0],args);
        } else if (pid < 0) {
                // Errore nell'operazione di fork
                perror("errore fork");
        } 
        return 1;
}

int cen_add(char **args){
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
                        return builtin_addDevice[i]();
            }
        }

        printf("Device indicato non riconosciuto\n");
        printf("Utilizzo: add <device>\n");
        printf("Digitare il comando 'device' per la lista di quelli disponibili\n");

        return 1;        
}

