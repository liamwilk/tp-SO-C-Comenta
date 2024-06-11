#include "../hilos.h"

void switch_case_kernel_entrada_salida_dialfs(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case ENTRADA_SALIDA_KERNEL_IDENTIFICACION:
    {
        t_entrada_salida_identificacion *identificacion = deserializar_t_entrada_salida_identificacion(buffer);

        if (kernel_entrada_salida_buscar_interfaz(io_args->args, identificacion->identificador) != NULL)
        {
            kernel_entradasalida_rechazo(io_args, modulo, identificacion->identificador);
            liberar_conexion(&io_args->entrada_salida->socket);
            break;
        }

        entrada_salida_agregar_identificador(io_args, identificacion->identificador);

        kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/%s/%d] Se recibio identificador válido.", modulo, identificacion->identificador, io_args->entrada_salida->orden);
        free(identificacion->identificador);
        free(identificacion);
        break;
    }
    default:
    {
        kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/%s/%d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden);
        liberar_conexion(&io_args->entrada_salida->socket);
        break;
    }
    }
}