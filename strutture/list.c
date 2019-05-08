#include "listH.h"

NodoPtr listInit(int data){
    NodoPtr toRtn = malloc(sizeof(Nodo));
    toRtn->data=data;
    toRtn->next=NULL;

    return toRtn;
}

void printList(NodoPtr list){
    while(list != NULL){
        if(list->data != -1)
            printf("%d ", list->data);
            
        list=list->next;
    }
    printf("\n");
}

//Dealloco tutta ma meoria allocata dinamicamente per i nodi della lista
void freeList(NodoPtr list){
    NodoPtr node;
    while(list!=NULL){
        node=list;
        list=list->next;
        free(node);
    }
}

NodoPtr insertLast(NodoPtr list, int data, int fd[2]){
    if(list==NULL){
        NodoPtr toRtn = listInit(data);
        toRtn->fd[0]=fd[0];
        toRtn->fd[1]=fd[1];

        return toRtn;
    }
    
    //Scorro la lista fino all'ultima posizione
    NodoPtr n= list;
    while (n->next != NULL)
    {
        n = n->next;
    }
    
    //Inserisco il nuovo nodo in ultima posizione
    NodoPtr tmp = listInit(data);
    tmp->fd[0]=fd[0];
    tmp->fd[1]=fd[1];
    n->next=tmp;



    return list;
}