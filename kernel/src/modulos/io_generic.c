#include "../hilos.h"

void switch_case_kernel_entrada_salida_generic(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer)
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
    case ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP:
    {
        t_entrada_salida_kernel_unidad_de_trabajo *unidad = deserializar_t_entrada_salida_kernel_unidad_de_trabajo(buffer);

        // Verifico si este proceso no ha ya sido marcado como eliminado  en kernel
        if (kernel_verificar_proceso_en_exit(io_args->args, unidad->pid))
        {
            break;
        }

        if (unidad->terminado)
        {
            t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(io_args->args, unidad->pid);
            if (io == NULL)
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_ERROR, "No se encontro el tipo GENERIC para el proceso PID <%d>", unidad->pid);
            }
            else
            {
                kernel_manejar_ready(io_args->args, unidad->pid, BLOCK_READY);
                // Aviso a la IO que esta desocupada
                io_args->entrada_salida->ocupado = 0;
                io_args->entrada_salida->pid = 0;
                // Envio el proximo proceso a esa IO
                kernel_proximo_io_generic(io_args->args, io);
            }
        }
        avisar_planificador(io_args->args);
        free(unidad);
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