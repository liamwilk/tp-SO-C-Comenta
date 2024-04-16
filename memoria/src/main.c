/* Módulo Memoria */
#include "main.h"

int main() {
    
    logger = iniciar_logger("memoria");
    config = iniciar_config(logger);
	memoria = memoria_inicializar(config);
	memoria_log(memoria, logger);

	// Inicio el servidor de Memoria

    socket_server_memoria = iniciar_servidor(logger,puertoEscucha);
	log_info(logger, "Servidor listo para recibir al cliente");

	// Atiendo las conexiones entrantes de CPU Dispatch, CPU Interrupt, Kernel y I/O, en ese orden.

	pthread_create(&cpu_dispatch,NULL,atender_cpu_dispatch,NULL);
	pthread_join(cpu_dispatch,NULL);

	pthread_create(&cpu_interrupt,NULL,atender_cpu_interrupt,NULL);
	pthread_join(cpu_interrupt,NULL);

	pthread_create(&kernel,NULL,atender_kernel,NULL);
	pthread_join(kernel,NULL);

	pthread_create(&io,NULL,atender_io,NULL);
	pthread_join(io,NULL);


	/*
	Aca va todo lo que hace Memoria, una vez que ya tiene todas las conexiones habilitadas a todos los Modulos.
	*/

	// Libero

	config_destroy(config);
	log_destroy(logger);
	
	liberar_conexion(socket_server_memoria);
	liberar_conexion(socket_cpu_dispatch);
	liberar_conexion(socket_cpu_interrupt);
	liberar_conexion(socket_kernel);
	liberar_conexion(socket_io);
	
    return 0;
}

void* atender_cpu_dispatch(){
	socket_cpu_dispatch = esperar_cliente(logger,socket_server_memoria);
	log_info(logger,"CPU Dispatch conectado en socket %d",socket_cpu_dispatch);
	pthread_exit(0);
}

void* atender_cpu_interrupt(){
	socket_cpu_interrupt = esperar_cliente(logger,socket_server_memoria);
	log_info(logger,"CPU Interrupt conectado en socket %d",socket_cpu_interrupt);
	pthread_exit(0);
}

void* atender_kernel(){
	socket_kernel = esperar_cliente(logger,socket_server_memoria);
	log_info(logger,"Kernel conectado en socket %d",socket_kernel);
	pthread_exit(0);
}

void* atender_io(){
	socket_io = esperar_cliente(logger,socket_server_memoria);
	log_info(logger,"I/O conectado en socket %d",socket_io);
	pthread_exit(0);
}


