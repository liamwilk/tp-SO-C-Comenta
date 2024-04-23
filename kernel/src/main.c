/* Módulo Kernel */

#include "main.h"

int main() {

    logger = iniciar_logger("kernel" , LOG_LEVEL_INFO);
    config = iniciar_config(logger);
    kernel = kernel_inicializar(config);
	kernel_log(kernel, logger);

	// Creo las conexiones a Memoria, CPU Dispatch y CPU Interrupt. Voy atendiendo las peticiones a medida que las abro.

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);

	pthread_create(&thread_atender_memoria,NULL,atender_memoria,NULL);
	pthread_detach(thread_atender_memoria);

	pthread_create(&thread_conectar_cpu_dispatch,NULL,conectar_cpu_dispatch,NULL);
	pthread_join(thread_conectar_cpu_dispatch,NULL);

	pthread_create(&thread_atender_cpu_dispatch,NULL,atender_cpu_dispatch,NULL);
	pthread_detach(thread_atender_cpu_dispatch);

	pthread_create(&thread_conectar_cpu_interrupt,NULL,conectar_cpu_interrupt,NULL);
	pthread_join(thread_conectar_cpu_interrupt,NULL);

	pthread_create(&thread_atender_cpu_interrupt,NULL,atender_cpu_interrupt,NULL);
	pthread_detach(thread_atender_cpu_interrupt);

    // Inicio server Kernel

	socket_server_kernel = iniciar_servidor(logger,kernel.puertoEscucha);
	log_info(logger, "Servidor listo para recibir clientes.");

	/*
	Atendemos las conexiones entrantes a Kernel desde IO.
	Ahora mismo es Join, pero debería ser detached luego, cuando se implemente la consola interactiva.
	Ese es el hilo que tiene que ser el join que mantiene vivo el main.

	TODO: Implementar consola interactiva, y luego poner ese hilo con join y este con detach.
	*/

	pthread_create(&thread_conectar_io,NULL,conectar_io,NULL);
	pthread_join(thread_conectar_io,NULL);

	// Libero

	log_destroy(logger);
	config_destroy(config);

    return 0;
}

void* conectar_io(){
	while(kernel_orden_apagado){
		int socket_cliente = esperar_cliente(logger, socket_server_kernel);

		if(socket_cliente == -1){
			break;
		}

		pthread_t hilo;
		int* args = malloc(sizeof(int));
		*args = socket_cliente;
		pthread_create(&hilo, NULL, atender_io, args);
		pthread_detach(hilo);
	}

	log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion I/O.");

	pthread_exit(0);
}

void* atender_io(void* args){
	int socket_cliente = *(int *)args;
	esperar_handshake(socket_cliente);
	log_info(logger, "I/O conectado en socket %d", socket_cliente);
	free(args);

	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de I/O en socket %d",socket_cliente);
		t_paquete* paquete = recibir_paquete(socket_cliente);

		if(paquete == NULL){
			log_warning(logger, "Conexión interrumpida con I/O. Cerrando socket %d", socket_cliente);
			liberar_conexion(socket_cliente);
			free(paquete);
			pthread_exit(0);
		}

		switch(paquete->codigo_operacion){
			case DESCONECTAR:
				log_info(logger, "Solicitud de desconexion con I/O. Cerrando socket %d", socket_cliente);
				liberar_conexion(socket_cliente);
				free(paquete);
				pthread_exit(0);
				break;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion I/O.");

	pthread_exit(0);
}

void* conectar_memoria(){
	socket_memoria = crear_conexion(logger,kernel.ipMemoria,kernel.puertoMemoria);
	handshake(logger,socket_memoria,1,"Memoria");
	log_info(logger,"Conectado a Memoria en socket %d",socket_memoria);
	pthread_exit(0);
}

void* atender_memoria(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de Memoria en socket %d",socket_memoria);
		t_paquete* paquete = recibir_paquete(socket_memoria);

		if(paquete == NULL){
			log_warning(logger, "Conexión interrumpida con Memoria. Cerrando socket %d", socket_memoria);
			liberar_conexion(socket_memoria);
			free(paquete);
			pthread_exit(0);
		}

		switch(paquete->codigo_operacion){
			case DESCONECTAR:
				log_info(logger, "Solicitud de desconexion con Memoria. Cerrando socket %d", socket_memoria);
				liberar_conexion(socket_memoria);
				free(paquete);
				pthread_exit(0);
				break;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion Memoria.");

	pthread_exit(0);
}

void* conectar_cpu_dispatch(){
	socket_cpu_dispatch = crear_conexion(logger,kernel.ipCpu,kernel.puertoCpuDispatch);
	handshake(logger,socket_cpu_dispatch,1,"CPU Dispatch");
	log_info(logger,"Conectado a CPU por Dispatch en socket %d",socket_cpu_dispatch);
	pthread_exit(0);
}

void* atender_cpu_dispatch(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de CPU Dispatch en socket %d",socket_cpu_dispatch);
		t_paquete* paquete = recibir_paquete(socket_cpu_dispatch);

		if(paquete == NULL){
			log_warning(logger, "Conexión interrumpida con CPU Dispatch. Cerrando socket %d", socket_cpu_dispatch);
			liberar_conexion(socket_cpu_dispatch);
			free(paquete);
			pthread_exit(0);
		}

		switch(paquete->codigo_operacion){
			case DESCONECTAR:
				log_info(logger, "Solicitud de desconexion con CPU Dispatch. Cerrando socket %d", socket_cpu_dispatch);
				liberar_conexion(socket_cpu_dispatch);
				free(paquete);
				pthread_exit(0);
				break;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion CPU Dispatch.");

	pthread_exit(0);
}

void* conectar_cpu_interrupt(){
	socket_cpu_interrupt = crear_conexion(logger,kernel.ipCpu,kernel.puertoCpuInterrupt);
	handshake(logger,socket_cpu_interrupt,1,"CPU Interrupt");
	log_info(logger,"Conectado a CPU por Interrupt en socket %d",socket_cpu_interrupt);
	pthread_exit(0);
}

void* atender_cpu_interrupt(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de CPU Interrupt en socket %d",socket_cpu_interrupt);
		t_paquete* paquete = recibir_paquete(socket_cpu_dispatch);

		if(paquete == NULL){
			log_warning(logger, "Conexión interrumpida con CPU Interrupt. Cerrando socket %d", socket_cpu_interrupt);
			liberar_conexion(socket_cpu_interrupt);
			free(paquete);
			pthread_exit(0);
		}

		switch(paquete->codigo_operacion){
			case DESCONECTAR:
				log_info(logger, "Solicitud de desconexion con CPU Interrupt. Cerrando socket %d", socket_cpu_interrupt);
				liberar_conexion(socket_cpu_interrupt);
				free(paquete);
				pthread_exit(0);
				break;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion CPU Interrupt.");

	pthread_exit(0);
}