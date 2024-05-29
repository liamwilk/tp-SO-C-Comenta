#include <utils/entradasalida.h>

void switch_case_kernel_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO:
    {
        log_error(args->logger, "Kernel rechazó la identificación de Entrada/Salida porque ya se encontraba conectada.");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    case FINALIZAR_SISTEMA:
    {
        log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
        pthread_cancel(args->threads.thread_atender_kernel_stdout);
        liberar_conexion(&args->sockets.socket_kernel_stdout);
        break;
    }
    case KERNEL_IO_INTERRUPCION:
    {
        t_paquete *paquete = recibir_paquete(args->logger, &args->sockets.socket_kernel_generic);
        t_kernel_io_interrupcion *interrupcion = deserializar_t_kernel_io_interrupcion(paquete->buffer);
        log_warning(args->logger, "[KERNEL/INTERRUPCION/STDOUT] Se recibio una interrupcion con motivo: %s para el PID %d", interrupcion->motivo, interrupcion->pid);
        break;
    }
    default:
    {
        log_warning(args->logger, "Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->sockets.socket_kernel_stdout);
        break;
    }
    }
}