#include <utils/entradasalida.h>

void switch_case_memoria_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case PLACEHOLDER:
    {
        // Placeholder
        break;
    }
    default:
    {
        log_warning(args->logger, "Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->sockets.socket_memoria_stdout);
        break;
    }
    }
}