#include "generic.h"

void procesar_entradasalida_gen(t_entradasalida entradasalida, t_log *logger, char *nombre_modulo)
{
    // La interfaz generica tiene realmente un unico hilo de ejecucion.
    // No seria necesario crear otro thread para atender la conexion con el kernel.

    int socket_kernel = crear_conexion(logger, entradasalida.ipKernel, entradasalida.puertoKernel);
    t_handshake resultado = crear_handshake(logger, socket_kernel, KERNEL_ENTRADA_SALIDA, "Kernel");
    if (resultado != CORRECTO)
    {
        liberar_conexion(socket_kernel);
        pthread_exit(0);
    }
    identificar_modulo(nombre_modulo, socket_kernel);

    log_info(logger, "Conectado a Kernel en socket %d", socket_kernel);

    while (1)
    {
        log_info(logger, "Esperando paquete de Kernel en socket %d", socket_kernel);
        t_paquete *paquete = recibir_paquete(logger, socket_kernel);

        // Rompo el while cuando se cae la conexion
        if (paquete->codigo_operacion < 0 || paquete->codigo_operacion > IO_IDENTIFICADOR)
        {
            liberar_conexion(socket_kernel);
            log_info(logger, "Ejecucion de interfaz generica terminada.");
            
            eliminar_paquete(paquete);
            break;
        }

        switch (paquete->codigo_operacion)
        {
        case IO_GEN_SLEEP:
        {
            log_info(logger, "%u : <%u> - Operacion a realizar: IO_GEN_SLEEP", process_getpid(), process_getpid());

            uint32_t unidades_de_trabajo = deserializar_unidades_de_trabajo(paquete);
            log_debug(logger, "Unidades: %d", unidades_de_trabajo);
            usleep(unidades_de_trabajo * entradasalida.tiempoUnidadDeTrabajo);
            log_debug(logger, "Dormido");

            t_paquete *aviso_sleep = crear_paquete(IO_GEN_SLEEP_TERMINADO);
            // Con el paquete vacio no me anda, si le agrego el uint32_t si...
            actualizar_buffer(aviso_sleep, sizeof(uint32_t));
            serializar_uint32_t(32, aviso_sleep);
            enviar_paquete(aviso_sleep, socket_kernel);
            log_debug(logger, "Paquete enviado");
            eliminar_paquete(aviso_sleep);
            break;
        }
        default:
            log_info(logger, "No conozco la operacion recibida (%d)", paquete->codigo_operacion);
            t_paquete *exit = crear_paquete(IO_AVISO_EXIT);

            enviar_paquete(exit, socket_kernel);
            eliminar_paquete(exit);
            eliminar_paquete(paquete);
            break;
        }
        eliminar_paquete(paquete);
    }
}

uint32_t deserializar_unidades_de_trabajo(t_paquete *paquete)
{
    uint32_t dato;
    memcpy(&dato, paquete->buffer->stream, sizeof(uint32_t));
    paquete->buffer->stream += sizeof(uint32_t);
    return dato;
}