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

        if (entrada_salida == NULL)
        {
            log_error(args->logger, "No se pudo enviar el paquete a la interfaz %s porque aun no se conecto.", sleep->interfaz);
            break;
        }

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz %s", sleep->interfaz);

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);

        t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
        unidad->pid = sleep->pid;
        unidad->unidad_de_trabajo = sleep->tiempo;

        serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);

        enviar_paquete(paquete, entrada_salida->socket);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio la instruccion de IO_GEN_SLEEP de %d segundos para el PID %d en la interfaz %s", sleep->tiempo, sleep->pid, sleep->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, sleep->pid);

        // Hago adapter de los registros de la CPU (no puntero) a los registros del PCB (puntero)

        pcb->registros_cpu->pc = sleep->registros.pc;
        pcb->registros_cpu->eax = sleep->registros.eax;
        pcb->registros_cpu->ebx = sleep->registros.ebx;
        pcb->registros_cpu->ecx = sleep->registros.ecx;
        pcb->registros_cpu->edx = sleep->registros.edx;
        pcb->registros_cpu->si = sleep->registros.si;
        pcb->registros_cpu->di = sleep->registros.di;
        pcb->registros_cpu->ax = sleep->registros.ax;
        pcb->registros_cpu->bx = sleep->registros.bx;
        pcb->registros_cpu->cx = sleep->registros.cx;
        pcb->registros_cpu->dx = sleep->registros.dx;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_GEN_SLEEP.", sleep->pid);
        kernel_transicion_exec_block(args);

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
