/* Módulo CPU */

#include "main.h"

int main() {
    
    logger = iniciar_logger("cpu");
    config = iniciar_config(logger);
    cpu = cpu_inicializar(config);
    cpu_log(cpu, logger);

	// Abrimos los sockets de conexion

	socket_memoria_dispatch = crear_conexion(logger,cpu.ipMemoria,cpu.puertoMemoria);
	log_info(logger,"Conectado a Memoria por Dispatch en socket %d",socket_memoria_dispatch);
	
	socket_memoria_interrupt = crear_conexion(logger,cpu.ipMemoria,cpu.puertoMemoria);
	log_info(logger,"Conectado a Memoria por Interrupt en socket %d",socket_memoria_interrupt);

	// Iniciamos los servidores de CPU (Dispatch y Interrupt)

    socket_server_dispatch = iniciar_servidor(logger,cpu.puertoEscuchaDispatch);
	log_info(logger, "Servidor Dispatch listo para recibir al cliente en Dispatch.");

	socket_server_interrupt = iniciar_servidor(logger,cpu.puertoEscuchaInterrupt);
	log_info(logger, "Servidor Interrupt listo para recibir al cliente en Interrupt.");

	// Atendemos las conexiones entrantes a CPU desde Kernel

	pthread_create(&dispatch,NULL,atender_kernel_dispatch,NULL);
	pthread_join(dispatch,NULL);

	pthread_create(&interrupt,NULL,atender_kernel_interrupt,NULL);
	pthread_join(interrupt,NULL);

	/*
	Aca va el resto de lo que falta hacer en CPU despues, en este punto ya tenemos las conexiones bidireccionales con Memoria y Kernel abiertas en sus sockets.
	*/

	// Libero

    log_destroy(logger);
    config_destroy(config);

	liberar_conexion(socket_memoria_dispatch);
	liberar_conexion(socket_memoria_interrupt);
	liberar_conexion(socket_kernel_dispatch);
	liberar_conexion(socket_kernel_interrupt);
	liberar_conexion(socket_server_dispatch);
	liberar_conexion(socket_server_interrupt);

    return 0;
}

void* atender_kernel_dispatch(){
	socket_kernel_dispatch = esperar_cliente(logger,socket_server_dispatch);
	log_info(logger,"Kernel conectado por Dispatch en socket %d",socket_kernel_dispatch);
	pthread_exit(0);
}

void* atender_kernel_interrupt(){
	socket_kernel_interrupt = esperar_cliente(logger,socket_server_interrupt);
	log_info(logger,"Kernel conectado por Interrupt en socket %d",socket_kernel_interrupt);
	pthread_exit(0);
}




