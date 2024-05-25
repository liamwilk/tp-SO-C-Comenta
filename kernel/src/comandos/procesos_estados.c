#include <utils/kernel.h>

void procesos_estados(hilos_args *hiloArgs)
{
    t_list *keys = dictionary_keys(hiloArgs->estados->procesos);
    if (list_size(keys) == 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "No hay procesos en el sistema");
        return;
    }
    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        char *estado = proceso_estado(hiloArgs->estados, atoi(key));
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "PID: %s - Estado: %s", key, estado);
    }
    return;
}
