#include "transiciones.h"

t_pcb *kernel_transicion_exec_ready(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_exec_ready);

    t_pcb *proceso = proceso_pop_exec(kernel_hilos_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de exec a ready fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready);
        return NULL;
    }
    proceso_push_ready(kernel_hilos_args->estados, proceso);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <READY>", proceso->pid);

    if (determinar_algoritmo(kernel_hilos_args->kernel->algoritmoPlanificador) == VRR)
    {
        kernel_log_ready(kernel_hilos_args, true);
    }
    // Log oficial de la catedra (En procedimiento)
    kernel_log_ready(kernel_hilos_args, false);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready);
    return proceso;
};

t_pcb *kernel_transicion_ready_exec(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_ready_exec);

    t_pcb *proceso = proceso_pop_ready(kernel_hilos_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de ready a exec fallida. No hay procesos en ready.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec);
        return NULL;
    }
    proceso_push_exec(kernel_hilos_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXEC>", proceso->pid);

    // Enviar paquete a cpu dispatch
    t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
    serializar_t_registros_cpu(&paquete, proceso->pid, proceso->registros_cpu);
    enviar_paquete(paquete, kernel_hilos_args->kernel->sockets.cpu_dispatch);
    free(paquete);

    return proceso;
};

t_pcb *kernel_transicion_block_ready(hilos_args *kernel_hilos_args, uint32_t pid)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_block_ready);
    t_pcb *proceso = proceso_remover_block(kernel_hilos_args->estados, pid);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de block a ready fallida. No hay procesos en block.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready);
        return NULL;
    }
    proceso_push_ready(kernel_hilos_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <BLOCK> - Estado Actual: <READY>", proceso->pid);

    // Log oficial de la catedra (En procedimiento)
    kernel_log_ready(kernel_hilos_args, false);
    return proceso;
};

t_pcb *kernel_transicion_exec_block(hilos_args *kernel_hilo_args)
{
    pthread_mutex_lock(&kernel_hilo_args->estados->mutex_exec_block);
    // Recorrer la lista de exec
    t_pcb *proceso = proceso_pop_exec(kernel_hilo_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de exec a block fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_block);
        return NULL;
    }
    proceso_push_block(kernel_hilo_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_block);
    // Log obligatorio de la catedra
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <BLOCK>", proceso->pid);
    return proceso;
};

t_pcb *kernel_transicion_exec_exit(hilos_args *kernel_hilo_args)
{
    pthread_mutex_lock(&kernel_hilo_args->estados->mutex_exec_exit);

    t_pcb *proceso = proceso_pop_exec(kernel_hilo_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_ERROR, "[ESTADOS/TRANSICION] Transicion de EXEC a EXIT fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_exit);
        return NULL;
    }
    proceso_push_exit(kernel_hilo_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_exit);

    // log oficial de la catedra
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <EXIT>", proceso->pid);

    // Se añade el signal para que siga ejecutando el planificador
    sem_post(&kernel_hilo_args->kernel->planificador_iniciar);

    return proceso;
};

void kernel_transicion_block_exit(hilos_args *kernel_hilos_args, uint32_t pid)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_block_exit);

    t_pcb *proceso = proceso_remover_block(kernel_hilos_args->estados, pid);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS/TRANSICION] Transicion de BLOCK a EXIT fallida. PID <%d> no encontrado en la cola de block", pid);
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_exit);
        return;
    }

    proceso_push_exit(kernel_hilos_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_exit);

    // log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <BLOCK> - Estado Actual: <EXIT>", proceso->pid);
}

t_pcb *kernel_transicion_new_ready(hilos_args *kernel_hilo_args)
{
    pthread_mutex_lock(&kernel_hilo_args->estados->mutex_new_ready);

    t_pcb *proceso = list_get(kernel_hilo_args->estados->new, 0);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_ERROR, "[ESTADOS/TRANSICION] Transicion de NEW a READY fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_new_ready);
        return NULL;
    }
    if (proceso->memoria_aceptado == false)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_WARNING, "No se puede mover el proceso PID: <%d> a ready, ya que no fue aceptado por memoria", proceso->pid);
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_new_ready);
        return NULL;
    }
    proceso_pop_new(kernel_hilo_args->estados);
    proceso_push_ready(kernel_hilo_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_new_ready);

    // log oficial de la catedra
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <NEW> - Estado Actual: <READY>", proceso->pid);

    // log oficial de la catedra (En procedimiento)
    kernel_log_ready(kernel_hilo_args, false);
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_DEBUG, "[PLANIFICADOR LARGO PLAZO] Proceso PID: <%d> movido a ready", proceso->pid);
    return proceso;
}

t_pcb *kernel_transicion_block_ready_mayor_prioridad(hilos_args *kernel_hilos_args, uint32_t pid)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_block_ready_mayor_prioridad);
    t_pcb *proceso = proceso_remover_block(kernel_hilos_args->estados, pid);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de block a ready de mayor prioridad fallida. No hay procesos en block.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready_mayor_prioridad);
        return NULL;
    }
    proceso_push_cola_prioritaria(kernel_hilos_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready_mayor_prioridad);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <BLOCK> - Estado Actual: <READY_PRIORIDAD>", proceso->pid);

    // Log oficial de la catedra (En procedimiento)
    kernel_log_ready(kernel_hilos_args, true);  // Se muestra la cola de mayor prioridad
    kernel_log_ready(kernel_hilos_args, false); // Se muestra la cola de menor prioridad
    return proceso;
}

t_pcb *kernel_transicion_ready_exec_mayor_prioridad(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_ready_exec_mayor_prioridad);

    t_pcb *proceso = proceso_pop_cola_prioritaria(kernel_hilos_args->estados);

    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de ready a exec fallida. No hay procesos en cola prioritaria");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec_mayor_prioridad);
        return NULL;
    }

    proceso_push_exec(kernel_hilos_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec_mayor_prioridad);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <READY_PRIORIDAD> - Estado Actual: <EXEC>", proceso->pid);

    // Enviar paquete a cpu dispatch
    t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
    serializar_t_registros_cpu(&paquete, proceso->pid, proceso->registros_cpu);
    enviar_paquete(paquete, kernel_hilos_args->kernel->sockets.cpu_dispatch);
    free(paquete);

    return proceso;
}
t_pcb *kernel_transicion_exec_ready_mayor_prioridad(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_exec_ready_mayor_prioridad);

    t_pcb *proceso = proceso_pop_exec(kernel_hilos_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de exec a ready de mayor prioridad fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready_mayor_prioridad);
        return NULL;
    }
    proceso_push_cola_prioritaria(kernel_hilos_args->estados, proceso);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <READY_PRIORIDAD>", proceso->pid);

    // Log oficial de la catedra (En procedimiento)
    kernel_log_ready(kernel_hilos_args, true);  // Se muestra la cola de mayor prioridad
    kernel_log_ready(kernel_hilos_args, false); // Se muestra la cola de menor prioridad
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready_mayor_prioridad);
    return proceso;
}

void kernel_manejar_ready(hilos_args *args, uint32_t pid, t_transiciones_ready TRANSICION)
{
    switch (TRANSICION)
    {
    case EXEC_READY:
        t_pcb *pcb = proceso_buscar_exec(args->estados, pid);

        if (pcb == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "[KERNEL/MANEJAR_EXEC_READY] Se quiere buscar el proceso <%d> en exec y no se encuentra, posible condicion de carrera", pid);
        }
        if (determinar_algoritmo(args->kernel->algoritmoPlanificador) == FIFO)
        {
            kernel_transicion_exec_ready(args);
            return;
        }

        int quantum_restante = interrumpir_temporizador(args);

        if (proceso_tiene_prioridad(args->kernel->algoritmoPlanificador, quantum_restante))
        {
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "El PID <%d> le sobraron <%d> milisegundos de quantum", pcb->pid, quantum_restante);
            // Almacenar el quantum sobrante
            pcb->quantum = quantum_restante;
            kernel_transicion_exec_ready_mayor_prioridad(args);
        }
        else
        {
            kernel_transicion_exec_ready(args);
        }

        break;
    case BLOCK_READY:
        pcb = proceso_buscar_block(args->estados, pid);
        if (pcb == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "[KERNEL/MANEJAR_BLOCK_READY] Se quiere buscar el proceso <%d> en block y no se encuentra, posible condicion de carrera", pid);
            break;
        }
        if (determinar_algoritmo(args->kernel->algoritmoPlanificador) == FIFO)
        {
            // El mismo algoritmo lo maneja
            kernel_transicion_block_ready(args, pid);
            break;
        }

        if (proceso_tiene_prioridad(args->kernel->algoritmoPlanificador, pcb->quantum))
        {
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "El PID <%d> sobro <%d> ms va a READY_PRIORIDAD", pcb->pid, pcb->quantum);
            kernel_transicion_block_ready_mayor_prioridad(args, pid);
        }
        else
        {
            kernel_transicion_block_ready(args, pid);
        }
        break;
    default:
        kernel_log_generic(args, LOG_LEVEL_ERROR, "[KERNEL/MANEJAR_READY] Transicion no reconocida");
        break;
    }
}

bool kernel_finalizar_proceso(hilos_args *kernel_hilos_args, uint32_t pid, KERNEL_MOTIVO_FINALIZACION MOTIVO)
{
    char *estado = proceso_estado(kernel_hilos_args->estados, pid);
    if (estado == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "El PID <%d> no existe", pid);
        return false;
    }

    if (strcmp(estado, "EXIT") == 0)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "El PID <%d> YA ha sido eliminado", pid);
        return false;
    }

    switch (MOTIVO)
    {
    case INTERRUPTED_BY_USER:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            interrumpir_temporizador(kernel_hilos_args);
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_WARNING, "El proceso <%d> se encuentra en ejecucion, se procede a desalojarlo", pid);
            kernel_interrumpir_cpu(kernel_hilos_args, pid, "FINALIZAR_PROCESO");
            kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
            kernel_transicion_exec_exit(kernel_hilos_args);
            return false;
        }
        if (strcmp(estado, "BLOCK") == 0)
        {
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_WARNING, "El proceso <%d> se encuentra bloqueado, se procede a desbloquearlo", pid);
            kernel_transicion_block_exit(kernel_hilos_args, pid);
            // TODO: Esto no tendria que hacerse en las transiciones
            //  Verificamos que el proceso este blockeado por recurso y no por I/O
            char *recurso_bloqueado = kernel_recurso(kernel_hilos_args, pid);
            if (recurso_bloqueado != NULL)
            {
                kernel_log_generic(kernel_hilos_args, LOG_LEVEL_DEBUG, "El proceso <%d> se encuentra bloqueado por recurso <%s>", pid, recurso_bloqueado);
                kernel_recurso_signal(kernel_hilos_args, kernel_hilos_args->recursos, pid, recurso_bloqueado, INTERRUPCION);
            }
            else
            {
                kernel_interrumpir_io(kernel_hilos_args, pid, "FINALIZAR_PROCESO");
            }
            kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
            return false;
        }

        // Esta en ready o en new por lo tanto se puede eliminar tranquilamente
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INTERRUPTED_BY_USER>", pid);
        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    case INVALID_INTERFACE:
    {
        t_pcb *pcb_en_exit = kernel_transicion_exec_exit(kernel_hilos_args);
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INVALID_INTERFACE>", pcb_en_exit->pid);
        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pcb_en_exit->pid));
        return true;
    }
    case SUCCESS:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            kernel_transicion_exec_exit(kernel_hilos_args);
        }

        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <SUCCESS>", pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    case INVALID_RESOURCE:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            kernel_transicion_exec_exit(kernel_hilos_args);
        }
        else
        {
            kernel_transicion_block_exit(kernel_hilos_args, pid);
        }
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INVALID_RESOURCE>", pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
    }
    case OUT_OF_MEMORY:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            kernel_transicion_exec_exit(kernel_hilos_args);
        }
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <OUT_OF_MEMORY>", pid);
        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    default:

        return false;
    }
}

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

void manejador_interrupciones(union sigval arg)
{
    timer_args_t *timerArgs = (timer_args_t *)arg.sival_ptr;

    if (list_size(timerArgs->args->estados->exec) > 0)
    {
        t_pcb *pcb = list_get(timerArgs->args->estados->exec, 0);
        kernel_log_generic(timerArgs->args, LOG_LEVEL_INFO, "PID: <%d> - Desalojado por fin de Quantum", pcb->pid);
        kernel_interrumpir_cpu(timerArgs->args, pcb->pid, "FIN DE QUANTUM");
    }
}

void inicializar_temporizador(hilos_args *argumentos, timer_args_t *temporizador)
{
    // Configura la estructura sigevent
    argumentos->sev.sigev_notify = SIGEV_THREAD;
    argumentos->sev.sigev_value.sival_ptr = temporizador;
    argumentos->sev.sigev_notify_function = manejador_interrupciones;
    argumentos->sev.sigev_notify_attributes = NULL;
}

void kernel_interrumpir_cpu(hilos_args *kernel_hilos_args, uint32_t pid, char *motivo)
{
    t_paquete *paquete = crear_paquete(KERNEL_CPU_INTERRUPCION);
    t_kernel_cpu_interrupcion interrupcion = {.pid = pid, .motivo = motivo, .len_motivo = strlen(motivo) + 1};
    serializar_t_kernel_cpu_interrupcion(&paquete, &interrupcion);
    enviar_paquete(paquete, kernel_hilos_args->kernel->sockets.cpu_interrupt);
    free(paquete);
}

void kernel_avisar_memoria_finalizacion_proceso(hilos_args *args, uint32_t pid)
{
    t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
    t_kernel_memoria_finalizar_proceso *proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
    proceso->pid = pid;
    serializar_t_kernel_memoria_finalizar_proceso(&paquete, proceso);
    enviar_paquete(paquete, args->kernel->sockets.memoria);
    eliminar_paquete(paquete);
    free(proceso);
}

void kernel_interrumpir_io(hilos_args *args, uint32_t pid, char *motivo)
{
    t_paquete *paquete = crear_paquete(KERNEL_IO_INTERRUPCION);
    t_kernel_io_interrupcion interrupcion = {.pid = pid, .motivo = motivo, .len_motivo = strlen(motivo) + 1};
    serializar_t_kernel_io_interrupcion(&paquete, &interrupcion);

    t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz(args, pid);

    io->ocupado = 0;
    io->pid = 0;

    enviar_paquete(paquete, io->socket);
    eliminar_paquete(paquete);
}

t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, uint32_t pid)
{
    for (int i = 0; i < list_size(args->kernel->sockets.list_entrada_salida); i++)
    {
        t_kernel_entrada_salida *modulo = list_get(args->kernel->sockets.list_entrada_salida, i);
        if (modulo->pid == pid)
        {
            return modulo;
        }
    }
    return NULL;
}