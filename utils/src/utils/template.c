#include "template.h"

void hilo_ejecutar(t_log *logger, int socket, char *modulo, t_funcion_ptr switch_case_atencion)
{
    while (1)
    {
        pthread_testcancel();

        log_debug(logger, "Esperando paquete de Kernel en socket %d", socket);

        t_paquete *paquete = recibir_paquete(logger, socket);

        if (paquete == NULL)
        {
            log_warning(logger, "%s se desconecto del socket %d.", modulo, socket);
            break;
        }

        revisar_paquete(paquete, logger, modulo);

        switch_case_atencion(logger, paquete->codigo_operacion, paquete->buffer);

        eliminar_paquete(paquete);
    }
}

void hilo_ejecutar_kernel(int socket, hilos_args *args, char *modulo, t_funcion_kernel_ptr switch_case_atencion)
{
    while (1)
    {
        log_debug(args->logger, "Esperando paquete de Kernel en socket %d", socket);

        t_paquete *paquete = recibir_paquete(args->logger, socket);

        if (paquete == NULL)
        {
            log_warning(args->logger, "%s se desconecto del socket %d.", modulo, socket);
            break;
        }

        revisar_paquete(paquete, args->logger, modulo);

        switch_case_atencion(args->logger, paquete->codigo_operacion, args, paquete->buffer);

        eliminar_paquete(paquete);
    }
}