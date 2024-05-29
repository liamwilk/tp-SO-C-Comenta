#include <utils/entradasalida.h>

void switch_case_memoria_dialfs(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case MEMORIA_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO:
    {
        log_error(args->logger, "Memoria rechazó la identificación de Entrada/Salida porque ya se encontraba conectada.");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    default:
    {
        log_warning(args->logger, "Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->sockets.socket_memoria_dialfs);
        break;
    }
    }
}