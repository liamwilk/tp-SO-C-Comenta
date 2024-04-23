/* Módulo CPU */

#include "main.h"

int main() {
    
    logger = iniciar_logger("cpu" , LOG_LEVEL_INFO);
    config = iniciar_config(logger);
    cpu = cpu_inicializar(config);
    cpu_log(cpu, logger);

	// Iniciamos los servidores de CPU (Dispatch y Interrupt)

    socket_server_dispatch = iniciar_servidor(logger,cpu.puertoEscuchaDispatch);
	log_info(logger, "Servidor Dispatch listo para recibir al cliente en socket %d",socket_server_dispatch);

	socket_server_interrupt = iniciar_servidor(logger,cpu.puertoEscuchaInterrupt);
	log_info(logger, "Servidor Interrupt listo para recibir al cliente en socket %d",socket_server_interrupt);

	// Abro la conexion a Memoria y comienzo a atenderla.

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);

	pthread_create(&thread_atender_memoria,NULL,atender_memoria,NULL);
	pthread_detach(thread_atender_memoria);

	// Atendemos las conexiones entrantes a CPU desde Kernel

	pthread_create(&thread_conectar_kernel_dispatch,NULL,conectar_kernel_dispatch,NULL);
	pthread_join(thread_conectar_kernel_dispatch,NULL);

	pthread_create(&thread_atender_kernel_dispatch,NULL,atender_kernel_dispatch,NULL);
	pthread_detach(thread_atender_kernel_dispatch);

	pthread_create(&thread_conectar_kernel_interrupt,NULL,conectar_kernel_interrupt,NULL);
	pthread_join(thread_conectar_kernel_interrupt,NULL);

	/*
	Dejo el ultimo en join para que el main no termine y el CPU no muera.
	TODO: Revisar esto, para ver que hay que hacer en cpu. Deberia ser temporal este fix.
	Cuando el Kernel se desconecta, el CPU se cierra automaticamente porque se desconecta del socket interrupt.
	*/

	pthread_create(&thread_atender_kernel_interrupt,NULL,atender_kernel_interrupt,NULL);
	pthread_join(thread_atender_kernel_interrupt,NULL);

	// Libero

	log_destroy(logger);
	config_destroy(config);

	return 0;
}
void* conectar_memoria(){
	socket_memoria = crear_conexion(logger,cpu.ipMemoria,cpu.puertoMemoria);
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
			case TERMINAR:
				log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion Memoria.");
				liberar_conexion(socket_memoria);
				free(paquete);
				kernel_orden_apagado=0;
				pthread_exit(0);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	pthread_exit(0);
}

void* conectar_kernel_dispatch(){
	socket_kernel_dispatch = esperar_cliente(logger,socket_server_dispatch);
	esperar_handshake(socket_kernel_dispatch);
	log_info(logger,"Kernel conectado por Dispatch en socket %d",socket_kernel_dispatch);
	pthread_exit(0);
}

void* atender_kernel_dispatch(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de Kernel Dispatch en socket %d",socket_kernel_dispatch);
		t_paquete* paquete = recibir_paquete(socket_kernel_dispatch);

		if(paquete == NULL){
			log_warning(logger, "Conexión interrumpida con Kernel Dispatch. Cerrando socket %d", socket_kernel_dispatch);
			liberar_conexion(socket_kernel_dispatch);
			free(paquete);
			pthread_exit(0);
		}

		switch(paquete->codigo_operacion){
			case DESCONECTAR:
				log_info(logger, "Solicitud de desconexion con Kernel. Cerrando socket %d", socket_kernel_dispatch);
				liberar_conexion(socket_kernel_dispatch);
				free(paquete);
				pthread_exit(0);
				break;
			case TERMINAR:
				log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion Kernel Dispatch.");
				liberar_conexion(socket_kernel_dispatch);
				free(paquete);
				kernel_orden_apagado=0;
				pthread_exit(0);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	pthread_exit(0);
}

void* conectar_kernel_interrupt(){
	socket_kernel_interrupt = esperar_cliente(logger,socket_server_interrupt);
	esperar_handshake(socket_kernel_interrupt);
	log_info(logger,"Kernel conectado por Interrupt en socket %d",socket_kernel_interrupt);
	pthread_exit(0);
}

void* atender_kernel_interrupt(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de Kernel Interrupt en socket %d",socket_kernel_interrupt);
		t_paquete* paquete = recibir_paquete(socket_kernel_interrupt);

		if(paquete == NULL){
			log_warning(logger, "Conexión interrumpida con Kernel Interrupt. Cerrando socket %d", socket_kernel_interrupt);
			liberar_conexion(socket_kernel_interrupt);
			free(paquete);
			pthread_exit(0);
		}

		switch(paquete->codigo_operacion){
			case DESCONECTAR:
				log_info(logger, "Solicitud de desconexion con Kernel Interrupt. Cerrando socket %d", socket_kernel_interrupt);
				liberar_conexion(socket_kernel_interrupt);
				free(paquete);
				pthread_exit(0);
				break;
			case TERMINAR:
				log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion Kernel Interrupt.");
				liberar_conexion(socket_kernel_interrupt);
				free(paquete);
				kernel_orden_apagado=0;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	pthread_exit(0);
}




