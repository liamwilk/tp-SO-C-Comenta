/* Módulo Memoria */
#include "main.h"

int main() {
    
    logger = iniciar_logger("memoria" , LOG_LEVEL_INFO);
    config = iniciar_config(logger);
	memoria = memoria_inicializar(config);
	memoria_log(memoria, logger);

    socket_server_memoria = iniciar_servidor(logger,memoria.puertoEscucha);
	log_info(logger, "Servidor Memoria listo para recibir al cliente en socket %d",socket_server_memoria);

	pthread_create(&thread_conectar_cpu,NULL,conectar_cpu,NULL);
	pthread_join(thread_conectar_cpu,NULL);

	pthread_create(&thread_atender_cpu,NULL,atender_cpu,NULL);
	pthread_detach(thread_atender_cpu);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);

	pthread_create(&thread_atender_io,NULL,conectar_io,NULL);
	pthread_detach(thread_atender_io);

	pthread_create(&thread_atender_kernel,NULL,atender_kernel,NULL);
	pthread_join(thread_atender_kernel,NULL);
	
	log_warning(logger,"Kernel solicito el apagado del sistema operativo.");

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
		void* stream = recibir_stream(socket_cpu);

		switch(cod_op){
			case DAME_PROXIMA_INSTRUCCION:
				// Se envia un paquete a CPU con una sola instruccion
				
				break;
			default:
				liberar_conexion(socket_cpu);
				pthread_exit(0);
				break;
		}
		free(stream);
	}

	pthread_exit(0);
}

void* conectar_kernel(){
	socket_kernel = esperar_cliente(logger,socket_server_memoria);
	esperar_handshake(socket_kernel);
	log_info(logger,"Kernel conectado en socket %d",socket_kernel);
	pthread_exit(0);
}

t_kernel_memoria* deserializar(t_buffer* buffer) {

    t_kernel_memoria* dato = malloc(sizeof(t_kernel_memoria));
    void* stream = buffer->stream;
    
	memcpy(&(dato->size_path), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    dato->path_instrucciones = malloc(dato->size_path);
	memcpy(dato->path_instrucciones, stream, dato->size_path);
    stream += dato->size_path * sizeof(char);
    
	memcpy(&(dato->program_counter), stream, sizeof(uint32_t));

    return dato;
}

void* atender_kernel(){
	while(kernel_orden_apagado){
		
		log_info(logger,"Esperando paquete de Kernel en socket %d",socket_kernel);
		t_paquete* paquete = recibir_paquete(socket_kernel);

		switch(paquete->codigo_operacion){
			case RECIBIR_PATH_INSTRUCCIONES:
				t_kernel_memoria *dato = deserializar(paquete->buffer);
				
				log_info(logger,"Tamaño del path: %d",dato->size_path);
				log_info(logger,"Path de instrucciones: %s",dato->path_instrucciones);
				log_info(logger,"Program counter: %d",dato->program_counter);

				free(dato->path_instrucciones);
				free(dato);

				break;
			case TERMINAR:
				kernel_orden_apagado=0;
				liberar_conexion(socket_kernel);
				liberar_conexion(socket_cpu);
				liberar_conexion(socket_server_memoria);
				break;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
	}

	pthread_exit(0);
}

void* conectar_io(){
	while(kernel_orden_apagado){
		int socket_cliente = esperar_cliente(logger, socket_server_memoria);
		
		if(socket_cliente == -1){
			break;
		}

		esperar_handshake(socket_cliente);
		pthread_t hilo;
		int* args = malloc(sizeof(int));
		*args = socket_cliente;
		pthread_create(&hilo, NULL, atender_io, args);
		pthread_detach(hilo);
	}

	pthread_exit(0);
}

void* atender_io(void* args){
	int socket_cliente = *(int *)args;
	log_info(logger, "I/O conectado en socket %d", socket_cliente);
	free(args);

	do{
		log_info(logger,"Esperando paquete de I/O en socket %d",socket_cliente);
		int cod_op = recibir_operacion(socket_cliente);
		void* stream = recibir_stream(socket_cliente);

		switch(cod_op){
			case MENSAJE:
				// placeholder
				break;
			default:
				liberar_conexion(socket_cliente);
				pthread_exit(0);
				break;
		}
		free(stream);
	}while(kernel_orden_apagado);

	pthread_exit(0);
}