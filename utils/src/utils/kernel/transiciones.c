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