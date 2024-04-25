/* Módulo Entrada-Salida */

#include "main.h"

int main() {
    
    logger = iniciar_logger("entradasalida" , LOG_LEVEL_INFO);
    config = iniciar_config(logger);
    entradasalida = entradasalida_inicializar(config);
    entradasalida_log(entradasalida,logger);

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);

	log_warning(logger,"Se cierra modulo I/O.");

    log_destroy(logger);
	config_destroy(config);

	liberar_conexion(socket_memoria);
	liberar_conexion(socket_kernel);

	return 0;
}

void* conectar_memoria(){
	socket_memoria = crear_conexion(logger,entradasalida.ipMemoria,entradasalida.puertoMemoria);
	handshake(logger,socket_memoria,1,"Memoria");
	log_info(logger,"Conectado a Memoria en socket %d",socket_memoria);
	pthread_exit(0);
}

void* conectar_kernel(){
	socket_kernel = crear_conexion(logger,entradasalida.ipKernel,entradasalida.puertoKernel);
	handshake(logger,socket_kernel,1,"Kernel");
	log_info(logger,"Conectado a Kernel en socket %d",socket_kernel);
	pthread_exit(0);
}