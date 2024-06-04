#include "cpu_interrupt.h"
void switch_case_cpu_interrupt(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_IO_GEN_SLEEP:
    {
        t_cpu_kernel_io_gen_sleep *sleep = deserializar_t_cpu_kernel_io_gen_sleep(buffer);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se recibio la instruccion de IO_GEN_SLEEP de %d segundos para el PID %d en la interfaz %s", sleep->tiempo, sleep->pid, sleep->interfaz);

        t_kernel_entrada_salida *entrada_salida = entrada_salida_buscar_interfaz(args, sleep->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, sleep->pid);

        if (pcb == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "[KERNEL/IO_GEN_SLEEP] Posible condiciones de carrera, el proceso <%d> no se encuentra en EXEC", sleep->pid);
            break;
        }

        if (entrada_salida == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque no existe.", sleep->interfaz);
            kernel_finalizar_proceso(args, sleep->pid, INVALID_INTERFACE);
            break;
        }

        if (entrada_salida->tipo != ENTRADA_SALIDA_GENERIC)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque no es del tipo IO_GENERIC.", sleep->interfaz);
            kernel_finalizar_proceso(args, sleep->pid, INVALID_INTERFACE);
            break;
        }

        if (entrada_salida->ocupado)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque esta ocupada con el proceso %d.", sleep->interfaz, entrada_salida->pid);
            // Si tenemos RR o VRR finalizo el timer
            proceso_avisar_timer(args->kernel->algoritmoPlanificador, pcb);
            kernel_transicion_exec_exit(args);
            break;
        }

        entrada_salida->ocupado = 1;
        entrada_salida->pid = sleep->pid;

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);

        t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));

        unidad->pid = sleep->pid;
        unidad->unidad_de_trabajo = sleep->tiempo;

        serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio la instruccion de IO_GEN_SLEEP de %d segundos para el PID %d en la interfaz %s", sleep->tiempo, sleep->pid, sleep->interfaz);

        // 2000
        kernel_transicion_exec_block(args);

        // Si tengo RR o VRR interrumpo el nanosleep
        if (determinar_algoritmo(args->kernel->algoritmoPlanificador) == RR || determinar_algoritmo(args->kernel->algoritmoPlanificador) == VRR)
        {
            proceso_interrumpir_quantum(pcb->sleeping_thread);
        }

        // Si tenemos RR o VRR finalizo el timer
        proceso_avisar_timer(args->kernel->algoritmoPlanificador, pcb);

        proceso_actualizar_registros(pcb, sleep->registros);

        enviar_paquete(paquete, entrada_salida->socket);

        sem_post(&args->kernel->planificador_iniciar);
        eliminar_paquete(paquete);
        free(unidad);

        break;
    }
    default:
    {
        kernel_log_generic(args, LOG_LEVEL_WARNING, "Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_interrupt);
        break;
    }
    }
}
