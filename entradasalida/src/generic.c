#include "generic.h"

void* procesar_entradasalida_gen(t_entradasalida entradasalida, t_log *logger)
{
    // La interfaz generica tiene realmente un unico hilo de ejecucion.
    // No seria necesario crear otro thread para atender la conexion con el kernel.

    int socket_kernel = crear_conexion(logger,entradasalida.ipKernel,entradasalida.puertoKernel);
	handshake(logger,socket_kernel,1,"Kernel");
    
    char* nombre_modulo = "placeholder";
    identificar_modulo(nombre_modulo, socket_kernel);

	log_info(logger,"Conectado a Kernel en socket %d",socket_kernel);

    while(1)
    {
		log_info(logger,"Esperando paquete de Kernel en socket %d",socket_kernel);
		t_paquete* paquete = recibir_paquete(logger, socket_kernel);

        // Rompo el while cuando se cae la conexion
        if(paquete->codigo_operacion == 0){
            liberar_conexion(socket_kernel);
            log_info(logger, "Ejecucion de interfaz generica terminada.");
            break;
        }

        switch (paquete->codigo_operacion)
        {
        case IO_GEN_SLEEP:
        {
            log_info(logger, "%u : <%u> - Operacion a realizar: IO_GEN_SLEEP", process_getpid(), process_getpid());
            
            uint32_t unidades_de_trabajo = deserializar_unidades_de_trabajo(paquete);
            usleep(unidades_de_trabajo * entradasalida.tiempoUnidadDeTrabajo);

            t_paquete *aviso_sleep = crear_paquete(IO_GEN_SLEEP_TERMINADO);
            enviar_paquete(aviso_sleep, socket_kernel);
            eliminar_paquete(aviso_sleep);
            break;
        }

        default:
            log_info(logger, "No conozco la operacion recibida (%d)", paquete->codigo_operacion);
            t_paquete* exit = crear_paquete(IO_AVISO_EXIT);
            enviar_paquete(exit, socket_kernel);
            eliminar_paquete(exit);
            break;
        }
    }

    return NULL;
}

uint32_t deserializar_unidades_de_trabajo(t_paquete *paquete)
{
    uint32_t dato;
    memcpy(&dato, paquete->buffer->stream, sizeof(uint32_t));
    return dato;
}