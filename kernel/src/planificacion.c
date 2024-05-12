#include <planificacion.h>

void planificacion_largo_plazo(t_kernel *kernel, t_diagrama_estados *estados, t_log *logger)
{
    int cant_en_new = list_size(estados->new);
    int cant_en_ready = list_size(estados->ready);

    if (cant_en_ready < kernel->gradoMultiprogramacion)
    {
        while (cant_en_ready < kernel->gradoMultiprogramacion && cant_en_new > 0)
        {
            t_pcb *proceso = proceso_pop_new(estados);
            if (proceso->memoria_aceptado == false)
            {
                log_warning(logger, "No se puede mover el proceso PID: <%d> a ready, ya que no fue aceptado por memoria", proceso->pid);
                continue;
            }
            proceso_push_ready(estados, proceso, logger);
            cant_en_ready++;
            cant_en_new--;
            log_debug(logger, "[PLANIFICADOR LARGO PLAZO] Proceso PID: <%d> movido a ready", proceso->pid);
        }
    }
    if (cant_en_new != 0)
    {
        log_warning(logger, "No se pueden mover mas procesos a ready, ya que se alcanzo el grado de multiprogramacion");
    }
};

void fifo(t_kernel *kernel, t_diagrama_estados *estados, t_log *logger)
{
    if (list_size(estados->ready) > 0 && list_size(estados->exec) == 0)
    {
        t_pcb *aux = kernel_transicion_ready_exec(estados, kernel, &mutex_ready_exec);
        log_debug(logger, "[FIFO]: Enviando proceso <PID: %d> a CPU", aux->pid);
        free(aux);
    }
}

void round_robin(t_kernel *kernel, t_diagrama_estados *estados, t_log *logger)
{
    if (list_size(estados->ready) == 0)
    {
        log_debug(logger, "[ROUND ROBIN]: No hay procesos en ready");
        return;
    }
    if (list_size(estados->exec) == 0)
    {

        t_pcb *aux = kernel_transicion_ready_exec(estados, kernel, &mutex_ready_exec);
        log_debug(logger, "[ROUND ROBIN]: Enviando proceso <PID: %d> a CPU", aux->pid);
        kernel_desalojar_proceso(estados, kernel, logger, &mutex_exec_ready, aux);
        return;
    }
}