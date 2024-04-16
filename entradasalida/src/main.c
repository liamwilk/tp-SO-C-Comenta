/* Módulo Entrada-Salida */

#include "main.h"

int main() {
    
    logger = iniciar_logger("entradasalida");
    config = iniciar_config(logger);
    entradasalida = entradasalida_inicializar(config);
    entradasalida_log(entradasalida,logger);


	// Conectamos con Memoria y Kernel.

    socket_memoria = crear_conexion(logger,entradasalida.ipMemoria,entradasalida.puertoMemoria);
	log_info(logger,"Conectado a Memoria en socket %d",socket_memoria);

    socket_kernel = crear_conexion(logger,entradasalida.ipKernel,entradasalida.puertoKernel);
	log_info(logger,"Conectado a Kernel en socket %d",socket_kernel);

	/*
	Aca iria la logica de lo que hace I/O una vez que ya tiene las conexiones abiertas con Kernel y Memoria.
	*/

    log_destroy(logger);
	config_destroy(config);

	liberar_conexion(socket_memoria);
	liberar_conexion(socket_kernel);

	return 0;
}
