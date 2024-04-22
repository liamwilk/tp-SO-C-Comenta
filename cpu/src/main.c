/* Módulo CPU */

#include "main.h"

int main() {
    
    logger_info = iniciar_logger("cpu" , LOG_LEVEL_INFO);
	logger_debug = iniciar_logger("cpu" , LOG_LEVEL_DEBUG);
	logger_warning = iniciar_logger("cpu" , LOG_LEVEL_WARNING);
	logger_error = iniciar_logger("cpu" , LOG_LEVEL_ERROR);
	logger_trace = iniciar_logger("cpu" , LOG_LEVEL_TRACE);

    config = iniciar_config(logger_error);
    cpu = cpu_inicializar(config);
    cpu_log(cpu, logger_info);

	// Abrimos los sockets de conexion

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);

	// Iniciamos los servidores de CPU (Dispatch y Interrupt)

    socket_server_dispatch = iniciar_servidor(logger_info,cpu.puertoEscuchaDispatch);
	log_info(logger_info, "Servidor Dispatch listo para recibir al cliente en Dispatch.");

	socket_server_interrupt = iniciar_servidor(logger_info,cpu.puertoEscuchaInterrupt);
	log_info(logger_info, "Servidor Interrupt listo para recibir al cliente en Interrupt.");

	// Atendemos las conexiones entrantes a CPU desde Kernel

	pthread_create(&thread_atender_kernel_dispatch,NULL,atender_kernel_dispatch,NULL);
	pthread_join(thread_atender_kernel_dispatch,NULL);

	pthread_create(&thread_atender_kernel_interrupt,NULL,atender_kernel_interrupt,NULL);
	pthread_join(thread_atender_kernel_interrupt,NULL);

	/*
	Aca va el resto de lo que falta hacer en CPU despues, en este punto ya tenemos las conexiones bidireccionales con Memoria y Kernel abiertas en sus sockets.
	*/

	// Libero

	log_destroy(logger_info);
	log_destroy(logger_debug);
	log_destroy(logger_warning);
	log_destroy(logger_error);
	log_destroy(logger_trace);




	config_destroy(config);

	liberar_conexion(socket_memoria);
	liberar_conexion(socket_kernel_dispatch);
	liberar_conexion(socket_kernel_interrupt);
	liberar_conexion(socket_server_dispatch);
	liberar_conexion(socket_server_interrupt);

	return 0;
}
void* conectar_memoria(){
	socket_memoria = crear_conexion(logger_info,cpu.ipMemoria,cpu.puertoMemoria);
	handshake(logger_info, logger_error,socket_memoria,1,"Memoria por Dispatch");
	log_info(logger_info,"Conectado a Memoria en socket %d",socket_memoria);
	pthread_exit(0);
}


void* atender_kernel_dispatch(){
	socket_kernel_dispatch = esperar_cliente(logger_info,socket_server_dispatch);
	esperar_handshake(socket_kernel_dispatch);
	log_info(logger_info,"Kernel conectado por Dispatch en socket %d",socket_kernel_dispatch);
	pthread_exit(0);
}

void* atender_kernel_interrupt(){
	socket_kernel_interrupt = esperar_cliente(logger_info,socket_server_interrupt);
	esperar_handshake(socket_kernel_interrupt);
	log_info(logger_info,"Kernel conectado por Interrupt en socket %d",socket_kernel_interrupt);
	pthread_exit(0);
}




