#include "recursos.h"

int count_character_occurrences(const char *str, char character)
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

void recursos_inicializar(t_dictionary *diccionario_recursos, char *instanciasRecursos, char *recursos)
{
    char *instancias_parsed = strdup(instanciasRecursos);
    instancias_parsed[strlen(instancias_parsed) - 1] = '\0';
    instancias_parsed++;
    int cantidadDeRecursos = count_character_occurrences(instancias_parsed, ',') + 1;

    char *recursos_parsed = strdup(recursos);
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
    return;
}

void recursos_log(hilos_args *args, t_dictionary *recursos)
{
    t_list *keys = dictionary_keys(recursos);
    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        t_recurso *recurso = dictionary_get(recursos, key);
        kernel_log_generic(args, LOG_LEVEL_INFO, "Recurso <%s> - Instancias <%d>", key, recurso->instancias);
    }
    return;
}