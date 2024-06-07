#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <utils/kernel.h>
#include <string.h>

typedef struct t_recurso
{
    int instancias;
    t_list *procesos_bloqueados;
} t_recurso;

void recursos_inicializar(t_dictionary *diccionario_recursos, char *instanciasRecursos, char *recursos);

void recursos_log(hilos_args *args, t_dictionary *recursos);