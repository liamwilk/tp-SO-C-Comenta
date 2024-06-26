#include <utils/entradasalida.h>

void switch_case_kernel_dialfs(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO:
    {
        log_error(args->logger, "Kernel rechazó la identificación de Entrada/Salida porque ya se encontraba conectada.");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    case KERNEL_IO_INTERRUPCION:
    {
        t_kernel_io_interrupcion *interrupcion = deserializar_t_kernel_io_interrupcion(buffer);
        log_warning(args->logger, "[KERNEL/INTERRUPCION/DIALFS] Se recibio una interrupcion con motivo: %s para el PID %d", interrupcion->motivo, interrupcion->pid);
        break;
    }
    case KERNEL_ENTRADA_SALIDA_IO_DIALFS_CREATE:
    {
        t_kernel_entrada_salida_fs_create *create = deserializar_t_kernel_entrada_salida_fs_create(buffer);
        log_info(args->logger, "PID: <%d> - Crear Archivo: <%s>", create->pid, create->nombre);
        free(create);
        // Peticion de crear un archivo
    }
    case FINALIZAR_SISTEMA:
    {
        log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
        pthread_cancel(args->threads.thread_atender_kernel_dialfs);
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    default:
    {
        log_warning(args->logger, "Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    }
}