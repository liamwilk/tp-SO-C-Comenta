#include <utils/entradasalida.h>

void switch_case_kernel_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case KERNEL_ENTRADA_SALIDA_IO_STDOUT_WRITE:
    {
        t_io_stdout_write *proceso_recibido = deserializar_t_io_stdout_write(buffer);

        log_debug(args->logger, "Se recibio orden de ejecucion de instruccion IO_STDOUT_WRITE asociado al proceso PID <%d>", proceso_recibido->pid);

        // Imprimo la direccion fisica recibida en proceos_recibido
        log_debug(args->logger, "Direccion fisica recibida a leer: %d", proceso_recibido->direccion_fisica);
        // Imprimo el desplazamiento
        log_debug(args->logger, "Desplazamiento dentro del marco %d: %d", proceso_recibido->desplazamiento, proceso_recibido->marco);
        // Imprimo el tamaño en bytes
        log_debug(args->logger, "Tamaño en bytes a leer desde la direccion fisica %d: %d", proceso_recibido->direccion_fisica, proceso_recibido->registro_tamanio);

        // Solicito a Memoria el dato de la direccion fisica recibida
        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_MEMORIA_IO_STDOUT_WRITE);
        t_io_memoria_stdout *paquete_enviar = malloc(sizeof(t_io_memoria_stdout));

        paquete_enviar->pid = proceso_recibido->pid;
        paquete_enviar->direccion_fisica = proceso_recibido->direccion_fisica;
        paquete_enviar->tamanio = proceso_recibido->registro_tamanio;

        serializar_t_io_memoria_stdout(&paquete, paquete_enviar);
        enviar_paquete(paquete, args->sockets.socket_memoria_stdout);
        eliminar_paquete(paquete);
        
        free(paquete_enviar);
        free(proceso_recibido->interfaz);
        free(proceso_recibido);

        break;
    }
    case KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO:
    {
        log_error(args->logger, "Kernel rechazó la identificación de Entrada/Salida porque ya se encontraba conectada.");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    case KERNEL_IO_INTERRUPCION:
    {
        t_paquete *paquete = recibir_paquete(args->logger, &args->sockets.socket_kernel_generic);
        t_kernel_io_interrupcion *interrupcion = deserializar_t_kernel_io_interrupcion(paquete->buffer);
        log_warning(args->logger, "[KERNEL/INTERRUPCION/STDOUT] Se recibio una interrupcion con motivo: %s para el PID %d", interrupcion->motivo, interrupcion->pid);
        break;
    }
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