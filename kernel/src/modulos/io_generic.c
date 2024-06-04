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
        free(identificacion->identificador);
        free(identificacion);
        break;
    }
    case ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP:
    {
        t_entrada_salida_kernel_unidad_de_trabajo *unidad = deserializar_t_entrada_salida_kernel_unidad_de_trabajo(buffer);

        // Verifico si este proceso no ha ya sido marcado como eliminado  en kernel
        t_pcb *pcb = proceso_buscar_exit(io_args->args->estados, unidad->pid);
        if (pcb != NULL)
        {
            // Si tenemos RR o VRR finalizo el timer
            interrumpir_temporizador(io_args->args);
            io_args->entrada_salida->ocupado = 0;
            io_args->entrada_salida->pid = 0;
            proceso_matar(io_args->args->estados, string_itoa(pcb->pid));
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INTERRUPTED_BY_USER>", pid);
            avisar_planificador(io_args->args);
            break;
        }

        if (unidad->terminado)
        {
            io_args->entrada_salida->ocupado = 0;
            kernel_manejar_ready(io_args->args, unidad->pid, BLOCK_READY);
            io_args->entrada_salida->pid = 0;
            avisar_planificador(io_args->args);
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