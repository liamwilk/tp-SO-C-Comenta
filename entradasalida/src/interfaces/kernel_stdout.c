#include <utils/entradasalida.h>

void switch_case_kernel_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case FINALIZAR_SISTEMA:
    {
        log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
        pthread_cancel(args->threads.thread_atender_kernel_stdout);
        liberar_conexion(&args->sockets.socket_kernel_stdout);
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