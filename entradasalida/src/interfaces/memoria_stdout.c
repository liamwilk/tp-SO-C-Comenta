#include <utils/entradasalida.h>

void *atender_memoria_stdout(void *args_void)
{
    t_io *args = (t_io *)args_void;
    while (1)
    {
        pthread_testcancel();
        
        log_debug(args->logger, "Esperando paquete de Memoria en socket %d", args->sockets.socket_memoria_stdout);

        interfaz_identificar(MEMORIA_ENTRADA_SALIDA_IDENTIFICACION,args->identificador, args->sockets.socket_memoria_stdout);

        t_paquete *paquete = recibir_paquete(args->logger, &args->sockets.socket_memoria_stdout);

        if (paquete == NULL)
        {
            log_info(args->logger, "Memoria se desconecto.");
            break;
        }

        revisar_paquete(paquete, args->logger, "Memoria");

        switch (paquete->codigo_operacion)
        {
        case FINALIZAR_SISTEMA:
        {
            log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
            pthread_cancel(args->threads.thread_atender_memoria_stdout);
            liberar_conexion(&args->sockets.socket_memoria_stdout);
            break;
        }
        default:
        {
            log_warning(args->logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
            eliminar_paquete(paquete);
            liberar_conexion(&args->sockets.socket_memoria_stdout);
            pthread_exit(0);
        }
        }
        eliminar_paquete(paquete);
    }
    pthread_exit(0);
}
