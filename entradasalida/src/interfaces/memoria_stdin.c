#include <utils/entradasalida.h>


void switch_case_memoria_stdin(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case MEMORIA_ENTRADA_SALIDA_IO_STDIN_READ:
    {
        t_memoria_entrada_salida_io_stdin_read *proceso_recibido = deserializar_t_memoria_entrada_salida_io_stdin_read(buffer);

        // Imprimo lo que memoria me envio
        log_debug(args->logger, "Se recibio respuesta de Memoria sobre la escritura de espacio de usuario asociada la instruccion IO_STDIN_READ para el proceso PID <%d>", proceso_recibido->pid);

        // Notifico a Kernel que se completo la operacion

        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_STDIN_READ);
        t_entrada_salida_kernel_io_stdin_read *proceso_enviar = malloc(sizeof(t_entrada_salida_kernel_io_stdin_read));

        proceso_enviar->pid = proceso_recibido->pid;
        proceso_enviar->resultado = 1;

        serializar_t_entrada_salida_kernel_io_stdin_read(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->sockets.socket_kernel_stdin);
        eliminar_paquete(paquete);

        free(proceso_enviar);
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
        liberar_conexion(&args->sockets.socket_memoria_stdin);
        break;
    }
    }
}