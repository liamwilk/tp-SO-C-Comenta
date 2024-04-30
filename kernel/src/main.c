/* Módulo Kernel */

#include "main.h"

int main()
{
	logger = iniciar_logger("kernel", LOG_LEVEL_INFO);
	config = iniciar_config(logger);
	kernel = kernel_inicializar(config);
	kernel_log(kernel, logger);

	pthread_create(&thread_conectar_memoria, NULL, conectar_memoria, NULL);
	pthread_join(thread_conectar_memoria, NULL);

	pthread_create(&thread_atender_memoria, NULL, atender_memoria, NULL);
	pthread_detach(thread_atender_memoria);

	////////////////////////////////////////////////////////

	// Se crea un paquete de prueba para recibir en Memoria
	
		// typedef struct
		// {
		// 	op_code codigo_operacion; // Header
		// 	uint32_t size_buffer;	  // Tamaño del buffer
		// 	t_buffer *buffer;		  // Payload (puede ser un mensaje, un paquete, etc)
		// } t_paquete;

	t_paquete *paquete = crear_paquete(RECIBIR_PATH_INSTRUCCIONES);

	// Creo el paquete que necesito mandarle a Memoria. Ojo con punteros, reserven memoria bien.
	
		// typedef struct t_kernel_memoria
		// {
		// uint32_t size_path;		  // Tamaño del path
		// char *path_instrucciones; // Path de las instrucciones
		// uint32_t program_counter; // Program counter
		// } t_kernel_memoria;

	// Creo el t_kernel_memoria que necesito mandarle a Memoria

	t_kernel_memoria dato;
	
	// Inicializo los campos del t_kernel_memoria

	char* mensaje = "instrucciones.txt";
	// dato.path_instrucciones = malloc(strlen(mensaje) + 1);
	// strcpy(dato.path_instrucciones, mensaje);
	dato.path_instrucciones = strdup(mensaje);
	dato.size_path = strlen(mensaje) + 1;
	dato.program_counter = 123456789;

	// Actualizo el size del buffer y reservo la memoria necesaria

	actualizar_buffer(paquete, dato.size_path + sizeof(uint32_t) + sizeof(uint32_t));

		// paquete->buffer->size = dato.size_path + sizeof(uint32_t) + sizeof(uint32_t);
		// paquete->size_buffer = dato.size_path + sizeof(uint32_t) + sizeof(uint32_t);
		// paquete->buffer->stream = malloc(paquete->buffer->size);

		// typedef struct
		// {
		// uint32_t size;	 // Tamaño del payload
		// uint32_t offset; // Desplazamiento dentro del payload
		// void *stream;	 // Payload
		// } t_buffer;

	// Empiezo a serializar el t_kernel_memoria en el buffer del paquete

	serializar_uint32_t(dato.size_path, paquete);
		// memcpy(paquete->buffer->stream + paquete->buffer->offset, &dato.size_path, sizeof(uint32_t));
		// paquete->buffer->offset += sizeof(uint32_t);

	serializar_char(dato.path_instrucciones, paquete);
		// memcpy(paquete->buffer->stream + paquete->buffer->offset, dato.path_instrucciones, dato.size_path);
		// paquete->buffer->offset += dato.size_path;

	serializar_uint32_t(dato.program_counter, paquete);
		// memcpy(paquete->buffer->stream + paquete->buffer->offset, &dato.program_counter, sizeof(uint32_t));
		// paquete->buffer->offset += sizeof(uint32_t);

	// Envio el paquete a Memoria

	enviar_paquete(paquete, socket_memoria);
	
	// Libero memoria

	// Es el puntero a char que esta en mi t_kernel_memoria
	
	free(dato.path_instrucciones);

	// Es el paquete que cree al principio

	eliminar_paquete(paquete);

	////////////////////////////////////////////////////////

	pthread_create(&thread_conectar_cpu_dispatch, NULL, conectar_cpu_dispatch, NULL);
	pthread_join(thread_conectar_cpu_dispatch, NULL);

	pthread_create(&thread_atender_cpu_dispatch, NULL, atender_cpu_dispatch, NULL);
	pthread_detach(thread_atender_cpu_dispatch);

	pthread_create(&thread_conectar_cpu_interrupt, NULL, conectar_cpu_interrupt, NULL);
	pthread_join(thread_conectar_cpu_interrupt, NULL);

	pthread_create(&thread_atender_cpu_interrupt, NULL, atender_cpu_interrupt, NULL);
	pthread_detach(thread_atender_cpu_interrupt);

	socket_server_kernel = iniciar_servidor(logger, kernel.puertoEscucha);
	log_info(logger, "Servidor listo para recibir clientes en socket %d.", socket_server_kernel);

	pthread_create(&thread_conectar_io, NULL, conectar_io, NULL);
	pthread_detach(thread_conectar_io);

	pthread_create(&thread_atender_consola,NULL,atender_consola,NULL);
	pthread_join(thread_atender_consola,NULL);
	
	log_warning(logger,"El Usuario solicito el apagado del sistema operativo.");

	log_destroy(logger);
	config_destroy(config);

	return 0;
}

void *atender_consola()
{
	char *linea;

	t_paquete* finalizar = crear_paquete(TERMINAR);
	char *path = "N/A";
	agregar_a_paquete(finalizar, path, strlen(path) + 1);

	while (kernel_orden_apagado)
	{

		linea = readline("\n> ");
		if (linea)
		{
			add_history(linea);
			char **separar_linea = string_split(linea, " ");
			funciones funcion = obtener_funcion(separar_linea[0]);

			switch (funcion)
			{
			case EJECUTAR_SCRIPT:
				log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
				break;
			case INICIAR_PROCESO:
				log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
				break;
			case FINALIZAR_PROCESO:
				log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
				break;
			case DETENER_PLANIFICACION:
				log_info(logger, "Se ejecuto script %s", separar_linea[0]);
				break;
			case INICIAR_PLANIFICACION:
				log_info(logger, "Se ejecuto script %s", separar_linea[0]);
				break;
			case MULTIPROGRAMACION:
				log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
				break;
			case PROCESO_ESTADO:
				log_info(logger, "Se ejecuto script %s", separar_linea[0]);
				break;
			case FINALIZAR:
				kernel_orden_apagado = 0;
				enviar_paquete(finalizar,socket_cpu_dispatch);
				enviar_paquete(finalizar,socket_memoria);
				liberar_conexion(socket_cpu_dispatch);
				liberar_conexion(socket_cpu_interrupt);
				liberar_conexion(socket_memoria);
				liberar_conexion(socket_server_kernel);
				break;

			default:
				log_info(logger, "Comando no reconocido");
				break;
			}
			// Libero la memoria de la línea separada
			int index = 0;
			while (separar_linea[index] != NULL) {
				free(separar_linea[index]);
				index++;
			}
			free(separar_linea);
			free(linea);
		}
		else
		{
			kernel_orden_apagado = 0;
			break;
		}
	}
	eliminar_paquete(finalizar);
	pthread_exit(0);
}

void *conectar_io()
{
	while (kernel_orden_apagado)
	{
		int socket_cliente = esperar_cliente(logger, socket_server_kernel);

		if (socket_cliente == -1)
		{
			break;
		}
		esperar_handshake(socket_cliente);

		pthread_t hilo;
		int *args = malloc(sizeof(int));
		*args = socket_cliente;
		pthread_create(&hilo, NULL, atender_io, args);
		pthread_detach(hilo);
	}

	pthread_exit(0);
}

void *atender_io(void *args)
{
	int socket_cliente = *(int *)args;
	log_info(logger, "I/O conectado en socket %d", socket_cliente);
	free(args);
	do{
		log_info(logger,"Esperando paquete de I/O en socket %d",socket_cliente);
		t_paquete* paquete = recibir_paquete(logger,socket_cliente);

		switch(paquete->codigo_operacion){
			case IO_IDENTIFICADOR:
				// Placeholder
				log_info(logger,"I/O se identifico");

				break;
			default:
				liberar_conexion(socket_cliente);
				pthread_exit(0);
				break;
		}

		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);

	}while(kernel_orden_apagado);

	pthread_exit(0);
}

void *conectar_memoria()
{
	socket_memoria = crear_conexion(logger, kernel.ipMemoria, kernel.puertoMemoria);
	handshake(logger, socket_memoria, 1, "Memoria");
	log_info(logger, "Conectado a Memoria en socket %d", socket_memoria);
	pthread_exit(0);
}

void* atender_memoria(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de Memoria en socket %d",socket_memoria);
		t_paquete* paquete = recibir_paquete(logger,socket_memoria);

		switch(paquete->codigo_operacion){
			case MENSAJE:
				// placeholder para despues
				break;
			default:
				liberar_conexion(socket_memoria);
				pthread_exit(0);
				break;
		}

		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	pthread_exit(0);
}

void *conectar_cpu_dispatch()
{
	socket_cpu_dispatch = crear_conexion(logger, kernel.ipCpu, kernel.puertoCpuDispatch);
	handshake(logger, socket_cpu_dispatch, 1, "CPU Dispatch");
	log_info(logger, "Conectado a CPU por Dispatch en socket %d", socket_cpu_dispatch);
	pthread_exit(0);
}

void* atender_cpu_dispatch(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de CPU Dispatch en socket %d",socket_cpu_dispatch);
		t_paquete* paquete = recibir_paquete(logger,socket_cpu_dispatch);

		switch(paquete->codigo_operacion){
			case MENSAJE:
				// Placeholder
				break;
			default:
				liberar_conexion(socket_cpu_dispatch);
				pthread_exit(0);
				break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	pthread_exit(0);
}

void *conectar_cpu_interrupt()
{
	socket_cpu_interrupt = crear_conexion(logger, kernel.ipCpu, kernel.puertoCpuInterrupt);
	handshake(logger, socket_cpu_interrupt, 1, "CPU Interrupt");
	log_info(logger, "Conectado a CPU por Interrupt en socket %d", socket_cpu_interrupt);
	pthread_exit(0);
}

void* atender_cpu_interrupt(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de CPU Interrupt en socket %d",socket_cpu_interrupt);
		t_paquete* paquete = recibir_paquete(logger,socket_cpu_interrupt);

		switch(paquete->codigo_operacion){
			case MENSAJE:
				// Placeholder
				break;
			default:
				liberar_conexion(socket_cpu_interrupt);
				pthread_exit(0);
				break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	pthread_exit(0);
}

funciones obtener_funcion(char *funcion)
{
	for (int i = 0; i < NUM_FUNCIONES; i++)
	{
		if (strcmp(FuncionesStrings[i], funcion) == 0)
		{
			return i;
		}
	}
	// Devolver un valor por defecto o manejar el error como prefieras
	return NUM_FUNCIONES;
}
