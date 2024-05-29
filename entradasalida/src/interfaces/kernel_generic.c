#include <utils/entradasalida.h>

void *atender_kernel_generic(void *args_void)
{
    t_io *args = (t_io *)args_void;
    while (1)
    {
        pthread_testcancel();

        log_debug(args->logger, "Esperando paquete de Kernel en socket %d", args->sockets.socket_kernel_generic);

        interfaz_identificar(KERNEL_ENTRADA_SALIDA_IDENTIFICACION,args->identificador, args->sockets.socket_kernel_generic);

        t_paquete *paquete = recibir_paquete(args->logger, &args->sockets.socket_kernel_generic);

        if (paquete == NULL)
        {
            log_info(args->logger, "Kernel se desconecto.");
            break;
        }

        revisar_paquete(paquete, args->logger, "Kernel");

        switch (paquete->codigo_operacion)
        {
        case KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP:
        {

            t_kernel_entrada_salida_unidad_de_trabajo *unidades = deserializar_t_kernel_entrada_salida_unidad_de_trabajo(paquete->buffer);

            log_info(args->logger, "PID : <%u> - Operacion a realizar: KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP", unidades->pid);

            log_debug(args->logger, "Durmiendo %d unidades de trabajo", unidades->unidad_de_trabajo);
            usleep(unidades->unidad_de_trabajo * args->tiempoUnidadDeTrabajo);

            t_paquete *aviso_sleep = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP);

            t_entrada_salida_kernel_unidad_de_trabajo *unidad = malloc(sizeof(t_entrada_salida_kernel_unidad_de_trabajo));
            unidad->terminado = true;
            unidad->pid = unidades->pid;

            serializar_t_entrada_salida_kernel_unidad_de_trabajo(&aviso_sleep, unidad);

            enviar_paquete(aviso_sleep, args->sockets.socket_kernel_generic);

            log_debug(args->logger, "Se envió aviso a Kernel de que termino el sleep.");

            free(unidad);
            eliminar_paquete(aviso_sleep);
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
            eliminar_paquete(paquete);
            liberar_conexion(&args->sockets.socket_kernel_generic);
            pthread_exit(0);
        }
        }
        eliminar_paquete(paquete);
    }
    pthread_exit(0);
}