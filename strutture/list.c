#include "listH.h"
NodoPtr listInit(int data){
    NodoPtr toRtn = malloc(sizeof(Nodo));
    toRtn->data=data;
    toRtn->next=NULL;

    return toRtn;
}

void printList(NodoPtr list){
    while(list != NULL){
        printf("%d ", list->data);
        printf("Canale scrittura %d\nCanale lettura %d\n", list->fd_reader, list->fd_writer);
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

NodoPtr insertLast(NodoPtr list, int data, int fd_reader,int fd_writer){
    if(list==NULL){
        NodoPtr toRtn = listInit(data);
        toRtn->fd_reader=fd_reader;
        toRtn->fd_writer=fd_writer;

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
    tmp->fd_reader=fd_reader;
    tmp->fd_writer=fd_writer;
    n->next=tmp;



    return list;
}
void removeNode(NodoPtr list, int data){
    NodoPtr nodo = list;
    while(nodo->next != NULL){
        if(nodo->next->data == data){
            NodoPtr tmp = nodo->next;
            nodo->next = tmp->next;
            return;
        }
    }
}
