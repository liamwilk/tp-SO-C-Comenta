#include "cpu_interrupt.h"
void switch_case_cpu_interrupt(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_IO_GEN_SLEEP:
    {
        t_cpu_kernel_io_gen_sleep *sleep = deserializar_t_cpu_kernel_io_gen_sleep(buffer);

        log_debug(logger, "Recibí la solicitud de CPU para activar IO_GEN_SLEEP en la Interfaz %s por %d unidades de trabajo asociado al PID %d", sleep->interfaz, sleep->tiempo, sleep->pid);

        if (args->kernel->sockets.entrada_salida_generic == 0)
        {
            log_error(args->logger, "No se pudo enviar el paquete a IO Generic porque aun no se conecto.");
            break;
        }

        // TODO: Implementar mapeo de logica de IDs de IO a sockets de IO. Actualmente solo se envia a IO Generic.

        log_debug(args->logger, "Se envia un sleep de 10 segundos a IO Generic ID %s", sleep->interfaz);
        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);

        t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
        unidad->pid = sleep->pid;
        unidad->unidad_de_trabajo = sleep->tiempo;

        serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);

        enviar_paquete(paquete, args->kernel->sockets.entrada_salida_generic);
        log_debug(args->logger, "Se envio el paquete a IO Generic");

        eliminar_paquete(paquete);
        free(unidad);

        break;
    }
    default:
    {
        log_warning(args->logger, "[CPU Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_interrupt);
        break;
    }
    }
}
