#include <utils/entradasalida.h>

void switch_case_kernel_stdin(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case KERNEL_ENTRADA_SALIDA_IO_STDIN_READ:
    {
        t_kernel_io_stdin_read *proceso_recibido = deserializar_t_kernel_io_stdin_read(buffer);

        log_debug(args->logger, "Se recibio orden de ejecucion de instruccion IO_STDIN_READ asociado al proceso PID <%d>", proceso_recibido->pid);

        // Imprimo la direccion fisica recibida en proceos_recibido
        log_debug(args->logger, "Direccion fisica recibida a escribir: <%d>", proceso_recibido->direccion_fisica);
        // Imprimo el tamaño en bytes
        log_debug(args->logger, "Tamaño a escribir: <%ld>", sizeof(proceso_recibido->input));

        // Solicito a Memoria el dato de la direccion fisica recibida
        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_MEMORIA_IO_STDIN_READ);
        t_io_memoria_stdin *paquete_enviar = malloc(sizeof(t_io_memoria_stdin));

        paquete_enviar->pid = proceso_recibido->pid;
        paquete_enviar->direccion_fisica = proceso_recibido->direccion_fisica;
        paquete_enviar->input = strdup(proceso_recibido->input);

        serializar_t_io_memoria_stdin(&paquete, paquete_enviar);
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
        log_warning(args->logger, "[KERNEL/INTERRUPCION/STDIN] Se recibio una interrupcion con motivo: %s para el PID %d", interrupcion->motivo, interrupcion->pid);
        break;
    }
    case FINALIZAR_SISTEMA:
    {
        log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
        pthread_cancel(args->threads.thread_atender_kernel_stdin);
        liberar_conexion(&args->sockets.socket_kernel_stdin);
        break;
    }
    default:
    {
        log_warning(args->logger, "Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->sockets.socket_kernel_stdin);
        break;
    }
    }
}