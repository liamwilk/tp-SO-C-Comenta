#include <planificacion.h>

void planificacion_largo_plazo(hilos_args *hiloArgs)
{
    int cant_en_new = list_size(hiloArgs->estados->new);
    int cant_en_ready = list_size(hiloArgs->estados->ready);
    if (cant_en_ready < hiloArgs->kernel->gradoMultiprogramacion)
    {
        while (cant_en_ready < (hiloArgs->kernel->gradoMultiprogramacion) && cant_en_new > 0)
        {
            kernel_transicion_new_ready(hiloArgs);
            cant_en_ready++;
            cant_en_new--;
        }
    }
    if (cant_en_new != 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[PLANIFICADOR LARGO PLAZO] No se pueden mover mas procesos a ready, ya que se alcanzo el grado de multiprogramacion");
    }

    // TODO: Hacer la transición a exit
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
            kernel_desalojar_proceso(kernel_hilos_args);
        }
        return;
    }
}