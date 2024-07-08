#include <utils/kernel/main.h>

// Función de comparación para ordenar las claves numéricas
bool comparar_claves_numericas(void *a, void *b)
{
    int int_a = atoi((char *)a);
    int int_b = atoi((char *)b);
    return int_a < int_b;
}

void procesos_estados(hilos_args *hiloArgs)
{
    t_list *keys = dictionary_keys(hiloArgs->estados->procesos);
    if (list_size(keys) == 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "No hay procesos en el sistema");
        return;
    }

    // Ordenar las claves numéricamente
    list_sort(keys, comparar_claves_numericas);

    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        char *estado = proceso_estado(hiloArgs->estados, atoi(key));
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "PID: %s - Estado: %s", key, estado);
    }
    list_destroy_and_destroy_elements(keys, free);
    return;
}