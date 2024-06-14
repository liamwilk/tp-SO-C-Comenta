#include <planificacion.h>

void planificacion_largo_plazo(hilos_args *hiloArgs)
{
    int cant_en_new = list_size(hiloArgs->estados->new);
    int cant_en_ready = list_size(hiloArgs->estados->ready);
    int cant_en_exec = list_size(hiloArgs->estados->exec);
    int cant_en_block = list_size(hiloArgs->estados->block);
    int cant_en_ready_prioridad = list_size(hiloArgs->estados->ready_mayor_prioridad);
    int cantidadDeProcesosEnConcurrente = cant_en_ready + cant_en_exec + cant_en_block + cant_en_ready_prioridad;

    if (cantidadDeProcesosEnConcurrente < hiloArgs->kernel->gradoMultiprogramacion)
    {
        while (cantidadDeProcesosEnConcurrente < (hiloArgs->kernel->gradoMultiprogramacion) && cant_en_new > 0)
        {
            kernel_transicion_new_ready(hiloArgs);
            cantidadDeProcesosEnConcurrente++;
            cant_en_new--;
        }
    }
    if (cant_en_new == 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_WARNING, "[PLANIFICADOR LARGO PLAZO] No hay mas procesos en new");
    }
};

void fifo(hilos_args *kernel_hilos_args)
{
    if (list_size(kernel_hilos_args->estados->exec) == 0 && list_size(kernel_hilos_args->estados->ready) > 0)
    {
        t_pcb *aux = kernel_transicion_ready_exec(kernel_hilos_args);
        if (aux != NULL)
        {
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_DEBUG, "[FIFO]: Enviando proceso <PID: %d> a CPU", aux->pid);
        }
        return;
    }
}

void round_robin(hilos_args *kernel_hilos_args)
{
    if (list_size(kernel_hilos_args->estados->exec) == 0 && list_size(kernel_hilos_args->estados->ready) > 0)
    {
        t_pcb *aux = kernel_transicion_ready_exec(kernel_hilos_args);
        if (aux != NULL)
        {
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_DEBUG, "[ROUND ROBIN]: Enviando proceso <PID: %d> a CPU", aux->pid);
            kernel_desalojar_proceso(kernel_hilos_args, aux, kernel_hilos_args->kernel->quantum);
        }
        return;
    }
}

void virtual_round_robin(hilos_args *hiloArgs)
{
    if (list_size(hiloArgs->estados->ready_mayor_prioridad) > 0 && list_size(hiloArgs->estados->exec) == 0)
    {
        t_pcb *ready_exec_prioridad = kernel_transicion_ready_exec_mayor_prioridad(hiloArgs);
        int quantum;
        if (ready_exec_prioridad->quantum <= 0)
        {
            quantum = hiloArgs->kernel->quantum;
        }
        else
        {
            quantum = ready_exec_prioridad->quantum;
        }
        if (ready_exec_prioridad != NULL)
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[VIRTUAL ROUND ROBIN]: Enviando proceso con MAYOR PRIORIDAD <PID: %d> a CPU, QUANTUM: <%d> milisegundos", ready_exec_prioridad->pid, quantum);
            kernel_desalojar_proceso(hiloArgs, ready_exec_prioridad, hiloArgs->kernel->quantum);
        }
        return;
    }
    if (list_size(hiloArgs->estados->ready) > 0 && list_size(hiloArgs->estados->ready_mayor_prioridad) == 0 && list_size(hiloArgs->estados->exec) == 0)
    {
        t_pcb *ready_exec = kernel_transicion_ready_exec(hiloArgs);
        if (ready_exec != NULL)
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[VIRTUAL ROUND ROBIN]: Enviando proceso <PID: %d> a CPU", ready_exec->pid);
            kernel_desalojar_proceso(hiloArgs, ready_exec, hiloArgs->kernel->quantum);
        }
        return;
    }
}

void planificacion_corto_plazo(hilos_args *hiloArgs)
{
    switch (determinar_algoritmo(hiloArgs->kernel->algoritmoPlanificador))
    {
    case FIFO:
    {
        fifo(hiloArgs);
        break;
    }
    case RR:
    {
        round_robin(hiloArgs);
        break;
    }
    case VRR:
    {
        virtual_round_robin(hiloArgs);
        break;
    }
    default:
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Algoritmo de planificacion no reconocido.");
        break;
    }
    }
}

void avisar_planificador(hilos_args *hilos_args)
{
    sem_post(&hilos_args->kernel->planificador_iniciar);
}