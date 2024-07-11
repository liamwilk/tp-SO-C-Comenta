#include "recursos.h"

int contar_cantidad_recursos(const char *str, char character)
{
    int count = 0;
    while (*str != '\0')
    {
        if (*str == character)
        {
            count++;
        }
        str++;
    }
    return count;
}

t_recurso *recurso_buscar(t_dictionary *diccionario_recursos, char *recursoSolicitado)
{
    t_recurso *recurso_encontrado = dictionary_get(diccionario_recursos, recursoSolicitado);
    return recurso_encontrado;
}

// Devuelve el recurso que tiene bloqueado a un proceso o NULL si no esta bloqueado ese proceso con ningun recurso
char *recurso_buscar_pid(t_dictionary *diccionario_recursos, uint32_t pid)
{
    t_list *keys = dictionary_keys(diccionario_recursos);
    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        t_recurso *recurso = dictionary_get(diccionario_recursos, key);
        for (int j = 0; j < list_size(recurso->procesos_bloqueados); j++)
        {
            t_pcb *pcb = list_get(recurso->procesos_bloqueados, j);
            if (pcb->pid == pid)
            {
                list_destroy(keys);
                return key;
            }
        }
    }
    list_destroy(keys);
    return NULL;
}

void recurso_init(t_dictionary *diccionario_recursos, char *instanciasRecursos, char *recursos)
{
    char *instancias_parsed_original = strdup(instanciasRecursos);
    char *instancias_parsed = instancias_parsed_original;
    instancias_parsed[strlen(instancias_parsed) - 1] = '\0';
    instancias_parsed++;

    int cantidadDeRecursos = contar_cantidad_recursos(instancias_parsed, ',') + 1;

    char *recursos_parsed_original = strdup(recursos);
    char *recursos_parsed = recursos_parsed_original;
    recursos_parsed[strlen(recursos_parsed) - 1] = '\0'; // Remover ']'
    recursos_parsed++;

    char **recursos_arr = string_split(recursos_parsed, ",");
    char **instancias_arr = string_split(instancias_parsed, ",");
    for (int i = 0; i < cantidadDeRecursos; i++)
    {
        t_recurso *recursoDiccionario = malloc(sizeof(t_recurso));
        recursoDiccionario->instancias = atoi(instancias_arr[i]);
        recursoDiccionario->procesos_bloqueados = list_create();
        dictionary_put(diccionario_recursos, recursos_arr[i], recursoDiccionario);
    }

    // Libero el recursos_arr
    for (int i = 0; recursos_arr[i] != NULL; i++)
    {
        free(recursos_arr[i]);
    }
    free(recursos_arr);

    // Libero el instancias_arr
    for (int i = 0; instancias_arr[i] != NULL; i++)
    {
        free(instancias_arr[i]);
    }
    free(instancias_arr);

    // Libero los punteros originales
    free(instancias_parsed_original);
    free(recursos_parsed_original);

    return;
}