#include "../hilos.h"

void switch_case_kernel_entrada_salida_generic(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case KERNEL_ENTRADA_SALIDA_IDENTIFICACION:
    {
        t_entrada_salida_identificacion *identificacion = deserializar_t_entrada_salida_identificacion(buffer);

        if (entrada_salida_buscar_interfaz(io_args->args, identificacion->identificador) != NULL)
        {
            entrada_salida_procesar_rechazado(io_args, "no identificada");
            kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/%d] Se rechazo identificacion, identificador %s ocupado. Cierro hilo.", modulo, io_args->entrada_salida->orden, identificacion->identificador);
            io_args->entrada_salida->valido = false;
            io_args->args->kernel->sockets.id_entrada_salida--;
            avisar_rechazo_identificador(io_args->entrada_salida->socket);
            liberar_conexion(&io_args->entrada_salida->socket);
            break;
        }

        entrada_salida_agregar_identificador(io_args, identificacion->identificador);

        kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/%s/%d] Se recibio identificador válido.", modulo, identificacion->identificador, io_args->entrada_salida->orden);

        break;
    }
    case ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP:
    {
        t_entrada_salida_kernel_unidad_de_trabajo *unidad = deserializar_t_entrada_salida_kernel_unidad_de_trabajo(buffer);

        // Verifico si este proceso no ha ya sido marcado como eliminado  kernel
        t_pcb *pcb = proceso_buscar_exit(io_args->args->estados, unidad->pid);
        // Se verifica que el proceso que se deseo eliminar es el que la io esta devolviendo y que ademas se encuentra en la cola de exit
        if (pcb != NULL)
        {
            io_args->entrada_salida->ocupado = 0;
            io_args->entrada_salida->pid = 0;
            proceso_matar(io_args->args->estados, string_itoa(pcb->pid));
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INTERRUPTED_BY_USER>", pid);
            sem_post(&io_args->args->kernel->planificador_iniciar);
            break;
        }

        if (unidad->terminado)
        {
            io_args->entrada_salida->ocupado = 0;
            kernel_transicion_block_ready(io_args, modulo, unidad);
            sem_post(&io_args->args->kernel->planificador_iniciar);
        }

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