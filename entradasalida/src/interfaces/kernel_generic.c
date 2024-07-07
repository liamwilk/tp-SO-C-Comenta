#include <utils/entradasalida.h>

void switch_case_kernel_generic(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP:
    {
        t_kernel_entrada_salida_unidad_de_trabajo *unidades = deserializar_t_kernel_entrada_salida_unidad_de_trabajo(buffer);

        args->unidades = unidades->unidad_de_trabajo;
        log_info(args->logger, "PID : <%u> - Operacion a realizar: <IO_GEN_SLEEP %s %d>", unidades->pid, args->identificador, unidades->unidad_de_trabajo);

        args->pid = unidades->pid;

        interfaz_iniciar_temporizador(args, unidades->unidad_de_trabajo);
        break;
    }
    case KERNEL_IO_INTERRUPCION:
    {
        interfaz_interrumpir_temporizador(args);
        t_kernel_io_interrupcion *interrupcion = deserializar_t_kernel_io_interrupcion(buffer);
        log_info(args->logger, "Se interrumpe la instruccion <IO_GEN_SLEEP> <%s> <%d> asociada al PID <%d> por motivo <%s>", args->identificador, args->unidades, args->pid, interrupcion->motivo);
        break;
    }
    case KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO:
    {
        log_error(args->logger, "Kernel rechazó la identificación de Entrada/Salida porque ya se encontraba conectada.");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    case FINALIZAR_SISTEMA:
    {
        log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
        pthread_cancel(args->threads.thread_atender_kernel_generic);
        liberar_conexion(&args->sockets.socket_kernel_generic);
        break;
    }
    default:
    {
        log_warning(args->logger, "[Kernel] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->sockets.socket_kernel_generic);
        break;
    }
    }
}