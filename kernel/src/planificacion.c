#include <planificacion.h>

void planificacion_largo_plazo(hilos_args *hiloArgs, t_diagrama_estados *estados, t_log *logger)
{
    int cant_en_new = list_size(estados->new);
    int cant_en_ready = list_size(estados->ready);

    if (cant_en_ready < hiloArgs->kernel->gradoMultiprogramacion)
    {
        while (cant_en_ready < (hiloArgs->kernel->gradoMultiprogramacion) && cant_en_new > 0)
        {
            t_pcb *proceso = proceso_pop_new(estados);
            if (proceso->memoria_aceptado == false)
            {
                log_generic(hiloArgs, LOG_LEVEL_WARNING, "No se puede mover el proceso PID: <%d> a ready, ya que no fue aceptado por memoria", proceso->pid);
                continue;
            }
            proceso_push_ready(estados, proceso, logger);
            cant_en_ready++;
            cant_en_new--;
            log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[PLANIFICADOR LARGO PLAZO] Proceso PID: <%d> movido a ready", proceso->pid);
        }
    }
    if (cant_en_new != 0)
    {
        log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[PLANIFICADOR LARGO PLAZO] No se pueden mover mas procesos a ready, ya que se alcanzo el grado de multiprogramacion");
    }

    // TODO: Hacer la transición a exit
};

void fifo(hilos_args *hiloArgs, t_diagrama_estados *estados, t_log *logger)
{
    if (list_size(estados->ready) == 0)
    {
        log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[FIFO]: No hay procesos en ready");
        return;
    }

    if (list_size(estados->ready) > 0 && list_size(estados->exec) == 0)
    {
        t_pcb *aux = kernel_transicion_ready_exec(estados, hiloArgs->kernel);
        log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[FIFO]: Enviando proceso <PID: %d> a CPU", aux->pid);
        free(aux);
    }
}

void round_robin(hilos_args *hiloArgs, t_diagrama_estados *estados, t_log *logger)
{
    if (list_size(estados->ready) == 0)
    {
        log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[ROUND ROBIN]: No hay procesos en ready");
        return;
    }
    if (list_size(estados->exec) == 0)
    {

        t_pcb *aux = kernel_transicion_ready_exec(estados, hiloArgs->kernel);
        log_generic(hiloArgs, LOG_LEVEL_DEBUG, "[ROUND ROBIN]: Enviando proceso <PID: %d> a CPU", aux->pid);
        kernel_desalojar_proceso(estados, hiloArgs->kernel, logger, aux);
        return;
    }
}