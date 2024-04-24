/* Módulo Memoria */
#include "main.h"

int main() {
    
    logger = iniciar_logger("memoria" , LOG_LEVEL_INFO);
    config = iniciar_config(logger);
	memoria = memoria_inicializar(config);
	memoria_log(memoria, logger);

	// Inicio el servidor de Memoria

    socket_server_memoria = iniciar_servidor(logger,memoria.puertoEscucha);
	log_info(logger, "Servidor Memoria listo para recibir al cliente en socket %d",socket_server_memoria);

	/*
	Atiendo las conexiones entrantes de CPU y Kernel, en orden bloqueante para asegurar que son esos los clientes que se conectan.
	A medida que habilito los sockets, empiezo a escuchar en esos sockets, con hilos detached para seguir conectando los demas.
	*/

	pthread_create(&thread_conectar_cpu,NULL,conectar_cpu,NULL);
	pthread_join(thread_conectar_cpu,NULL);

	pthread_create(&thread_atender_cpu,NULL,atender_cpu,NULL);
	pthread_detach(thread_atender_cpu);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);

	pthread_create(&thread_atender_kernel,NULL,atender_kernel,NULL);
	pthread_detach(thread_atender_kernel);
	
	// Atiendo las conexiones de I/O en un hilo join, para que no finalice el main hasta que no de la orden Kernel.

	pthread_create(&thread_atender_io,NULL,conectar_io,NULL);
	pthread_join(thread_atender_io,NULL);

	// Libero todo

	config_destroy(config);
	log_destroy(logger);
	
    return 0;
}

void* conectar_cpu(){
	socket_cpu = esperar_cliente(logger,socket_server_memoria);
	esperar_handshake(socket_cpu);
	log_info(logger,"CPU conectado en socket %d",socket_cpu);
	pthread_exit(0);
}

void* atender_cpu(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de CPU en socket %d",socket_cpu);
		int cod_op = recibir_operacion(socket_cpu);

		switch(cod_op){
			case MENSAJE:
				// placeholder
				break;
			default:
				log_info(logger, "Solicitud de desconexion con CPU. Cerrando socket %d", socket_cpu);
				liberar_conexion(socket_cpu);
				pthread_exit(0);
				break;
		}
	}

	pthread_exit(0);
}

void* conectar_kernel(){
	socket_kernel = esperar_cliente(logger,socket_server_memoria);
	esperar_handshake(socket_kernel);
	log_info(logger,"Kernel conectado en socket %d",socket_kernel);
	pthread_exit(0);
}

void* atender_kernel(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de Kernel en socket %d",socket_kernel);
		int cod_op = recibir_operacion(socket_kernel);

		switch(cod_op){
			case TERMINAR:
				log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion CPU.");
				liberar_conexion(socket_kernel);
				kernel_orden_apagado=0;
				pthread_exit(0);
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	log_warning(logger,"Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion Kernel.");

	pthread_exit(0);
}

void* conectar_io(){
	while(kernel_orden_apagado){
		int socket_cliente = esperar_cliente(logger, socket_server_memoria);

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
		int cod_op = recibir_operacion(socket_cliente);

		switch(cod_op){
			case MENSAJE:
				// placeholder
				break;
			default:
				log_info(logger, "Solicitud de desconexion con I/O. Cerrando socket %d", socket_cliente);
				liberar_conexion(socket_cliente);
				pthread_exit(0);
				break;
		}
	}

	pthread_exit(0);
}