#include "recursos.h"

void kernel_recurso_wait(hilos_args *args, t_dictionary *recursoDiccionario, uint32_t pid, char *recursoSolicitado)
{

    t_recurso *recurso_encontrado = kernel_recurso_buscar(recursoDiccionario, recursoSolicitado);
    t_pcb *proceso_en_exec = proceso_buscar_exec(args->estados, pid);

    if (recurso_encontrado == NULL)
    {
        proceso_en_exec->quantum = interrumpir_temporizador(args);
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro el recurso <%s> solicitado por CPU", recursoSolicitado);
        kernel_finalizar_proceso(args, pid, INVALID_RESOURCE);
        return;
    };

    if (recurso_encontrado->instancias < 0)
    {
        proceso_en_exec->quantum = interrumpir_temporizador(args);
        kernel_log_generic(args, LOG_LEVEL_INFO, "Se bloquea el proceso <%d> por falta de instancias del recurso <%s>", pid, recursoSolicitado);
        list_add(recurso_encontrado->procesos_bloqueados, proceso_en_exec);
        kernel_transicion_exec_block(args);
        return;
    }

    recurso_encontrado->instancias--;
    kernel_log_generic(args, LOG_LEVEL_INFO, "Se ocupo una instancia del recurso <%s> - Instancias restantes <%d>", recursoSolicitado, recurso_encontrado->instancias);
    kernel_manejar_ready(args, pid, EXEC_READY);
    return;
}

void kernel_recurso_signal(hilos_args *args, t_dictionary *recursoDiccionario, uint32_t pid, char *recurso, t_recurso_motivo_liberacion MOTIVO)
{
    t_recurso *recurso_encontrado = kernel_recurso_buscar(recursoDiccionario, recurso);
    if (recurso_encontrado == NULL)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro el recurso <%s> solicitado por CPU", recurso);
        kernel_finalizar_proceso(args, pid, INVALID_RESOURCE);
    };

    recurso_encontrado->instancias++;
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se libero una instancia del recurso <%s> - Instancias restantes <%d>", recurso, recurso_encontrado->instancias);
    if (list_size(recurso_encontrado->procesos_bloqueados) > 0 && recurso_encontrado->instancias >= 0)
    {
        t_pcb *pcb = list_remove(recurso_encontrado->procesos_bloqueados, 0);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se desbloquea el proceso <%d> por liberacion de recurso <%s>", pcb->pid, recurso);
        if (MOTIVO == SIGNAL_RECURSO)
        {
            // Mando a ready el que esta bloqueado pues se libero un recurso
            kernel_manejar_ready(args, pcb->pid, BLOCK_READY);
        }
        else
        {
            // Solamente libero el recurso ya que lo mande a exit el proceso
            return;
        }
    }

    kernel_manejar_ready(args, pid, EXEC_READY);
    return;
}

void kernel_recurso_log(hilos_args *args)
{
    t_list *keys = dictionary_keys(args->recursos);
    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        t_recurso *recurso = dictionary_get(args->recursos, key);
        int cantidadProcesosBloqueados = list_size(recurso->procesos_bloqueados);
        kernel_log_generic(args, LOG_LEVEL_INFO, "Recurso <%s> - Instancias <%d> - Cantidad Procesos bloqueados <%d>", key, recurso->instancias, cantidadProcesosBloqueados);
    }
    return;
}

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

void kernel_recurso_init(t_dictionary *diccionario_recursos, char *instanciasRecursos, char *recursos)
{
    char *instancias_parsed = strdup(instanciasRecursos);
    instancias_parsed[strlen(instancias_parsed) - 1] = '\0';
    instancias_parsed++;
    int cantidadDeRecursos = contar_cantidad_recursos(instancias_parsed, ',') + 1;

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

t_recurso *kernel_recurso_buscar(t_dictionary *diccionario_recursos, char *recursoSolicitado)
{
    t_recurso *recurso_encontrado = dictionary_get(diccionario_recursos, recursoSolicitado);
    return recurso_encontrado;
}

// Devuelve el recurso que tiene bloqueado a un proceso o NULL si no esta bloqueado ese proceso con ningun recurso
char *kernel_recurso(hilos_args *args, uint32_t pid)
{
    t_list *keys = dictionary_keys(args->recursos);
    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        t_recurso *recurso = dictionary_get(args->recursos, key);
        for (int j = 0; j < list_size(recurso->procesos_bloqueados); j++)
        {
            t_pcb *pcb = list_get(recurso->procesos_bloqueados, j);
            if (pcb->pid == pid)
            {
                return key;
            }
        }
    }
    return NULL;
}
