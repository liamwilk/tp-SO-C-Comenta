/* Módulo Memoria */
#include "main.h"

int main() {
    
    logger_info = iniciar_logger("memoria" , LOG_LEVEL_INFO);
	logger_debug = iniciar_logger("memoria" , LOG_LEVEL_DEBUG);
	logger_warning = iniciar_logger("memoria" , LOG_LEVEL_WARNING);
	logger_error = iniciar_logger("memoria" , LOG_LEVEL_ERROR);
	logger_trace = iniciar_logger("memoria" , LOG_LEVEL_TRACE);

    config = iniciar_config(logger_error);
	memoria = memoria_inicializar(config);
	memoria_log(memoria, logger_info);

	// Inicio el servidor de Memoria

    socket_server_memoria = iniciar_servidor(logger_info,memoria.puertoEscucha);
	log_info(logger_info, "Servidor listo para recibir al cliente");

	// Atiendo las conexiones entrantes de CPU Dispatch, CPU Interrupt, Kernel y I/O, en ese orden.

	pthread_create(&thread_atender_cpu_dispatch,NULL,atender_cpu,NULL);
	pthread_join(thread_atender_cpu_dispatch,NULL);

	pthread_create(&thread_atender_kernel,NULL,atender_kernel,NULL);
	pthread_join(thread_atender_kernel,NULL);

	pthread_create(&thread_atender_io,NULL,procesar_io,NULL);

	/*
	Aca va todo lo que hace Memoria, una vez que ya tiene todas las conexiones habilitadas a todos los Modulos.
	*/
	
	// Espero al hilo de I/O
	pthread_join(thread_atender_io,NULL);

	// Libero

	config_destroy(config);
	log_destroy(logger_info);
	log_destroy(logger_debug);
	log_destroy(logger_warning);
	log_destroy(logger_error);
	log_destroy(logger_trace);
	
	liberar_conexion(socket_server_memoria);
	liberar_conexion(socket_cpu);
	liberar_conexion(socket_kernel);
	
    return 0;
}

void* atender_cpu(){
	socket_cpu = esperar_cliente(logger_info,socket_server_memoria);
	esperar_handshake(socket_cpu);
	log_info(logger_info,"CPU conectado en socket %d",socket_cpu);
	pthread_exit(0);
}

void* atender_kernel(){
	socket_kernel = esperar_cliente(logger_info,socket_server_memoria);
	esperar_handshake(socket_kernel);
	log_info(logger_info,"Kernel conectado en socket %d",socket_kernel);
	pthread_exit(0);
}

void* atender_io(void* args){
	int socket_cliente = *(int *)args;
	esperar_handshake(socket_cliente);
	log_info(logger_info, "I/O conectado en: %d", socket_cliente);
	// liberar_conexion(socket_cliente);
	return NULL;
}

void* procesar_io(){
	while(1){
		int socket_cliente = esperar_cliente(logger_info, socket_server_memoria);

		if(socket_cliente == -1){
			break;
		}

		pthread_t hilo;
		int* args = malloc(sizeof(int));
		*args = socket_cliente;
		pthread_create(&hilo, NULL, atender_io, args);
		pthread_detach(hilo);
	}

	return NULL;
}