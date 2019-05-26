#include "comandiH.h"
/*
void initArray(array_risposte *a, size_t initialSize) {
  a->array = (risp *)malloc(initialSize * sizeof(risp));
  a->used = 0;
  a->size = initialSize;
}

void insertArray(array_risposte *a, risp element) {
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (risp *)realloc(a->array, a->size * sizeof(risp));
  }
  a->array[a->used++] = element;
}

void freeArray(array_risposte *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}


void print_risp(risp ans){
  printf("\nCons: %d, profondita: %d, terminaz: %d, eliminato %d\n", ans.considera, ans.profondita, ans.terminazione, ans.eliminato);
  printf("Info: %s\n", ans.info);
  printf("ID: %d, id_padre: .NaN.\n", ans.id);
}
void print_cmd(cmd comando){
  printf("\nTipo %c ", comando.tipo_comando);
  if(comando.tipo_comando != 'l'){
    printf(", ID: %d\n", comando.id);
  }
}*/