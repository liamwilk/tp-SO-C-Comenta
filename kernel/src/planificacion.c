#include <planificacion.h>

void planificacion_largo_plazo(t_kernel *kernel, diagrama_estados *estados, t_log *logger)
{
    int cant_en_new = list_size(estados->new->cola);
    int cant_en_ready = list_size(estados->ready->cola);

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
            proceso_push_ready(estados, proceso);
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

void planificacion_corto_plazo(t_kernel *kernel, diagrama_estados *estados, t_log *logger)
{
    if (strcmp(kernel->algoritmoPlanificador, "FIFO") == 0)
    {
        fifo(kernel, estados, logger);
    }
};

void fifo(t_kernel *kernel, diagrama_estados *estados, t_log *logger)
{
    while (1)
    {
        t_paquete *paquete = recibir_paquete(kernel->sockets.cpu_dispatch); // Recibo paquete de CPU para ver si hay que ejecutar algo de IO o termino

        if (list_size(estados->ready->cola) > 0 && list_size(estados->exec->cola) == 0)
        {
            t_pcb *aux = proceso_transicion_ready_exec(estados);
            log_debug(logger, "[FIFO]: Enviando proceso <PID: %d> a CPU", aux->pid);
            t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);

            serializar_t_registros_cpu(&paquete, aux->pid, aux->registros_cpu);
            enviar_paquete(paquete, kernel->sockets.cpu_dispatch);
            free(paquete);
            free(aux);
        }
        if (list_size(estados->block->cola) == 0 && paquete->codigo_operacion == CPU_KERNEL_IO_GEN_SLEEP)
        {
            t_cpu_kernel_io_gen_sleep *proceso = deserializar_t_cpu_kernel_io_gen_sleep(paquete);
                }
    }
}