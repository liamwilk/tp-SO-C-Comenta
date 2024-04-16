/* Módulo Kernel */

#include "main.h"

int main() {

    logger = iniciar_logger("kernel");
    config = iniciar_config(logger);
    kernel = kernel_inicializar(config);
	kernel_log(kernel, logger);

	// Creo las conexiones a Memoria, CPU Dispatch y CPU Interrupt

	socket_memoria = crear_conexion(logger,kernel.ipMemoria,kernel.puertoMemoria);
	log_info(logger,"Conectado a Memoria en socket %d",socket_memoria);

	socket_cpu_interrupt = crear_conexion(logger,kernel.ipCpu,kernel.puertoCpuInterrupt);
	log_info(logger,"Conectado a CPU por Interrupt en socket %d",socket_cpu_interrupt);

	socket_cpu_dispatch = crear_conexion(logger,kernel.ipCpu,kernel.puertoCpuDispatch);
	log_info(logger,"Conectado a CPU por Dispatch en socket %d",socket_cpu_dispatch);

    // Inicio server Kernel

	socket_server_kernel = iniciar_servidor(logger,kernel.puertoEscucha);
	log_info(logger, "Servidor listo para recibir clientes.");

	// Atendemos las conexiones entrantes a Kernel desde IO

	pthread_create(&io,NULL,atender_io,NULL);
	pthread_join(io,NULL);

	/*
	Aca va lo que hace Kernel, una vez que ya tiene todas las conexiones con todos los modulos establecidas.
	*/

	// Libero

	liberar_conexion(socket_server_kernel);
	liberar_conexion(socket_memoria);
	liberar_conexion(socket_cpu_interrupt);
	liberar_conexion(socket_cpu_dispatch);

	log_destroy(logger);
	config_destroy(config);

    return 0;
}

void* atender_io(){
	socket_io = esperar_cliente(logger,socket_server_kernel);
	log_info(logger,"I/O conectado en socket %d",socket_io);
	pthread_exit(0);
}


