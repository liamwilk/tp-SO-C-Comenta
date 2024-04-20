/* Módulo Entrada-Salida */

#include "main.h"

int main() {
    
    logger_info = iniciar_logger("entradasalida" , LOG_LEVEL_INFO);
	logger_debug = iniciar_logger("entradasalida" , LOG_LEVEL_DEBUG);
	logger_warning = iniciar_logger("entradasalida" , LOG_LEVEL_WARNING);
	logger_error = iniciar_logger("entradasalida" , LOG_LEVEL_ERROR);
	logger_trace = iniciar_logger("entradasalida" , LOG_LEVEL_TRACE);

    config = iniciar_config(logger_error);
    entradasalida = entradasalida_inicializar(config);
    entradasalida_log(entradasalida,logger_info);

	// Conectamos con Memoria y Kernel.

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);

	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);

	/*
	Aca iria la logica de lo que hace I/O una vez que ya tiene las conexiones abiertas con Kernel y Memoria.
	*/

    log_destroy(logger_info);
	log_destroy(logger_debug);
	log_destroy(logger_warning);
	log_destroy(logger_error);
	log_destroy(logger_trace);

	config_destroy(config);

	liberar_conexion(socket_memoria);
	liberar_conexion(socket_kernel);

	return 0;
}

void* conectar_memoria(){
	socket_memoria = crear_conexion(logger_info,entradasalida.ipMemoria,entradasalida.puertoMemoria);
	handshake(logger_info, logger_error, socket_memoria,1,"Memoria");
	log_info(logger_info,"Conectado a Memoria en socket %d",socket_memoria);
	pthread_exit(0);
}

void* conectar_kernel(){
	socket_kernel = crear_conexion(logger_info,entradasalida.ipKernel,entradasalida.puertoKernel);
	handshake(logger_info,  logger_error, socket_kernel,1,"Kernel");
	log_info(logger_info,"Conectado a Kernel en socket %d",socket_kernel);
	pthread_exit(0);
}