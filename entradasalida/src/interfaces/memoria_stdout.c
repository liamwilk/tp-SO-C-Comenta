#include <utils/entradasalida.h>

void switch_case_memoria_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case MEMORIA_ENTRADA_SALIDA_IO_STDOUT_WRITE:
    {
        t_memoria_io_stdout *proceso_recibido = deserializar_t_memoria_io_stdout(buffer);

        log_debug(args->logger, "Se recibio respuesta de Memoria sobre la lectura de espacio de usuario asociada la instruccion IO_STDOUT_WRITE para el proceso PID <%d>", proceso_recibido->pid);

        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_STDOUT_WRITE);
        t_entrada_salida_kernel_io_stdout_write *proceso_enviar = malloc(sizeof(t_entrada_salida_kernel_io_stdout_write));

        if (proceso_recibido->resultado)
        {
            log_info(args->logger, "Mensaje recuperado de Memoria: %s", proceso_recibido->dato);
            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 1;
        }
        else
        {
            log_error(args->logger, "No se pudo leer del espacio de usuario");
            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
        }

        serializar_t_entrada_salida_kernel_io_stdout_write(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->sockets.socket_kernel_stdout);
        eliminar_paquete(paquete);

        free(proceso_enviar);
        free(proceso_recibido->dato);
        free(proceso_recibido);

        break;
    }
    case MEMORIA_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO:
    {
        log_error(args->logger, "Memoria rechazó la identificación de Entrada/Salida porque ya se encontraba conectada.");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
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