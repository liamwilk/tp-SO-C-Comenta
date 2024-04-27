#include "generic.h"

void* procesar_entradasalida_gen(t_entradasalida entradasalida, t_log *logger)
{
    // La interfaz generica tiene realmente un unico hilo de ejecucion.
    // No seria necesario crear otro thread para atender la conexion con el kernel.

    int socket_kernel = crear_conexion(logger,entradasalida.ipKernel,entradasalida.puertoKernel);
	handshake(logger,socket_kernel,1,"Kernel");
	log_info(logger,"Conectado a Kernel en socket %d",socket_kernel);

    // TODO: pregunta como hay que matar a los modulos de I/O
    //      es con una orden del kernel?
    //      es despues de un determinado tiempo?
    //      preguntas que le surgen a uno durante la vida misma...

    while(1)
    {
		log_info(logger,"Esperando paquete de Kernel en socket %d",socket_kernel);
		int cod_op = recibir_operacion(socket_kernel);

        switch (cod_op)
        {
        case IO_GEN_SLEEP:
            log_info(logger, "%u : <%u> - Operacion a realizar: IO_GEN_SLEEP", process_getpid(), process_getpid());
            int* tamanio_buffer = (int *)sizeof(int);
            int unidades_de_trabajo = *(int *)recibir_buffer(tamanio_buffer, socket_kernel);
            // TODO: hacer que duerma unidades de trabajo en lugar de segundos.
            sleep(unidades_de_trabajo);
            // free(unidades_de_trabajo); Hace falta liberar esto?
            break;
        
        default:
            log_info(logger, "No conozco la operacion recibida (%d)", cod_op);
            // TODO: mandar EXIT a Kernel.
            break;
        }
    }

    liberar_conexion(socket_kernel);
    log_info(logger, "Interfaz generica terminada.");

    return NULL;
}