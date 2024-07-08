#include "cpu_interrupt.h"
void switch_case_cpu_interrupt(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_IO_GEN_SLEEP:
    {
        t_cpu_kernel_io_gen_sleep *sleep = deserializar_t_cpu_kernel_io_gen_sleep(buffer);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, sleep->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, sleep->pid);

        if (pcb == NULL)
        {
            free(sleep);
            break;
        }

        if (entrada_salida == NULL)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_GEN_SLEEP> <%s> <%d> del PID <%d> a la interfaz <%s> porque no está conectada.", sleep->interfaz, sleep->tiempo, sleep->pid, sleep->interfaz);
            kernel_finalizar_proceso(args, sleep->pid, INVALID_INTERFACE);
            free(sleep);
            break;
        }

        if (entrada_salida->tipo != ENTRADA_SALIDA_GENERIC)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_GEN_SLEEP> <%s> <%d> del PID <%d> a la interfaz <%s> porque no es IO_GENERIC.", sleep->interfaz, sleep->tiempo, sleep->pid, sleep->interfaz);
            kernel_finalizar_proceso(args, sleep->pid, INVALID_INTERFACE);
            free(sleep);
            break;
        }

        if (entrada_salida->ocupado)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_WARNING, "No se pudo enviar la instrucción <IO_GEN_SLEEP> <%s> <%d> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", sleep->interfaz, sleep->tiempo, sleep->pid, sleep->interfaz, entrada_salida->pid);

            proceso_actualizar_registros(pcb, sleep->registros);
            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", sleep->pid, sleep->interfaz);

            kernel_transicion_exec_block(args);

            // Actualizar campo tiene_proxima_io
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                pcb->proxima_io->tiene_proxima_io = true;
            }
            pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
            pcb->proxima_io->tipo = ENTRADA_SALIDA_GENERIC;
            pcb->proxima_io->args = list_create();

            list_add(pcb->proxima_io->args, string_itoa(sleep->tiempo));

            avisar_planificador(args);

            free(sleep);
            break;
        }

        pcb->quantum = interrumpir_temporizador(args);
        proceso_actualizar_registros(pcb, sleep->registros);

        entrada_salida->ocupado = 1;
        entrada_salida->pid = sleep->pid;
        kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", sleep->pid, sleep->interfaz);
        kernel_transicion_exec_block(args);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Retransmito instruccion <IO_GEN_SLEEP> <%s> <%d> del PID <%d> a interfaz <%s>", sleep->interfaz, sleep->tiempo, sleep->pid, sleep->interfaz);

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);
        t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
        unidad->pid = sleep->pid;
        unidad->unidad_de_trabajo = sleep->tiempo;

        serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);

        enviar_paquete(paquete, entrada_salida->socket);
        avisar_planificador(args);
        eliminar_paquete(paquete);

        free(unidad);
        free(sleep);
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
