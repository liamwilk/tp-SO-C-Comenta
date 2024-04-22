/* Módulo Kernel */

#include "main.h"

int main() {

    logger_info = iniciar_logger("kernel" , LOG_LEVEL_INFO);
	logger_debug = iniciar_logger("kernel" , LOG_LEVEL_DEBUG);
	logger_warning = iniciar_logger("kernel" , LOG_LEVEL_WARNING);
	logger_error = iniciar_logger("kernel" , LOG_LEVEL_ERROR);
	logger_trace = iniciar_logger("kernel" , LOG_LEVEL_TRACE);

    config = iniciar_config(logger_error);
    kernel = kernel_inicializar(config);
	kernel_log(kernel, logger_info);

	// Creo las conexiones a Memoria, CPU Dispatch y CPU Interrupt

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);

	pthread_create(&thread_conectar_cpu_dispatch,NULL,conectar_cpu_dispatch,NULL);
	pthread_join(thread_conectar_cpu_dispatch,NULL);

	pthread_create(&thread_conectar_cpu_interrupt,NULL,conectar_cpu_interrupt,NULL);
	pthread_join(thread_conectar_cpu_interrupt,NULL);

    // Inicio server Kernel

	socket_server_kernel = iniciar_servidor(logger_info,kernel.puertoEscucha);
	log_info(logger_info, "Servidor listo para recibir clientes.");

	// Atendemos las conexiones entrantes a Kernel desde IO

	pthread_create(&thread_atender_io,NULL,procesar_io,NULL);
	

	/*
	Aca va lo que hace Kernel, una vez que ya tiene todas las conexiones con todos los modulos establecidas.
	*/

	pthread_join(thread_atender_io,NULL);

	// Libero

	liberar_conexion(socket_server_kernel);
	liberar_conexion(socket_memoria);
	liberar_conexion(socket_cpu_interrupt);
	liberar_conexion(socket_cpu_dispatch);

	log_destroy(logger_info);
	log_destroy(logger_debug);	
	log_destroy(logger_warning);		
	log_destroy(logger_error);
	log_destroy(logger_trace);



	config_destroy(config);

    return 0;
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
		int socket_cliente = esperar_cliente(logger_info, socket_server_kernel);

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

void* conectar_memoria(){
	socket_memoria = crear_conexion(logger_info,kernel.ipMemoria,kernel.puertoMemoria);
	handshake(logger_info,  logger_error,socket_memoria,1,"Memoria");
	log_info(logger_info,"Conectado a Memoria en socket %d",socket_memoria);
	pthread_exit(0);
}

void* conectar_cpu_dispatch(){
	socket_cpu_dispatch = crear_conexion(logger_info,kernel.ipCpu,kernel.puertoCpuDispatch);
	handshake(logger_info,  logger_error,socket_cpu_dispatch,1,"CPU Dispatch");
	log_info(logger_info,"Conectado a CPU por Dispatch en socket %d",socket_cpu_dispatch);
	pthread_exit(0);
}

void* conectar_cpu_interrupt(){
	socket_cpu_interrupt = crear_conexion(logger_info,kernel.ipCpu,kernel.puertoCpuInterrupt);
	handshake(logger_info,  logger_error,socket_cpu_interrupt,1,"CPU Interrupt");
	log_info(logger_info,"Conectado a CPU por Interrupt en socket %d",socket_cpu_interrupt);
	pthread_exit(0);
}