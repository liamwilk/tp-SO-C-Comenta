/* Módulo CPU */

#include "main.h"

int main()
{

	logger = iniciar_logger("cpu", LOG_LEVEL_DEBUG);
	config = iniciar_config(logger);
	cpu = cpu_inicializar(config);
	cpu_log(cpu, logger);

	socket_server_dispatch = iniciar_servidor(logger, cpu.puertoEscuchaDispatch);
	log_info(logger, "Servidor Dispatch listo para recibir al cliente en socket %d", socket_server_dispatch);

	socket_server_interrupt = iniciar_servidor(logger, cpu.puertoEscuchaInterrupt);
	log_info(logger, "Servidor Interrupt listo para recibir al cliente en socket %d", socket_server_interrupt);

	pthread_create(&thread_conectar_memoria, NULL, conectar_memoria, NULL);
	pthread_join(thread_conectar_memoria, NULL);

	pthread_create(&thread_atender_memoria, NULL, atender_memoria, NULL);
	pthread_detach(thread_atender_memoria);

	pthread_create(&thread_conectar_kernel_dispatch, NULL, conectar_kernel_dispatch, NULL);
	pthread_join(thread_conectar_kernel_dispatch, NULL);

	pthread_create(&thread_atender_kernel_dispatch, NULL, atender_kernel_dispatch, NULL);
	pthread_detach(thread_atender_kernel_dispatch);

	pthread_create(&thread_conectar_kernel_interrupt, NULL, conectar_kernel_interrupt, NULL);
	pthread_join(thread_conectar_kernel_interrupt, NULL);
	
	/*
	Hasta que pongamos semaforos, necesito este sleep para darle tiempo a Memoria que reciba el path de instrucciones de
	Kernel y las suba a la cola de instrucciones. Si lo saco, llega la instruccion DAME_PROXIMA_INSTRUCCION antes de que
	memoria haya subido las instrucciones a la cola, y da error. 
	TODO: Acá hay que sincronizar con semáforos.
	*/

	sleep(5);

	t_paquete* paquete = crear_paquete(DAME_PROXIMA_INSTRUCCION);
	char *path = "vacio";
	agregar_a_paquete(paquete, path, strlen(path) + 1);

	// Envio la instruccion a CPU
	enviar_paquete(paquete, socket_memoria);
	
	// Libero la memoria del paquete de instruccion
	eliminar_paquete(paquete);

	pthread_create(&thread_atender_kernel_interrupt, NULL, atender_kernel_interrupt, NULL);
	pthread_join(thread_atender_kernel_interrupt, NULL);

	log_warning(logger, "Kernel solicito el apagado del sistema operativo.");

	log_destroy(logger);
	config_destroy(config);

	return 0;
}

void *conectar_memoria()
{
	socket_memoria = crear_conexion(logger, cpu.ipMemoria, cpu.puertoMemoria);
	handshake(logger, socket_memoria, 1, "Memoria");
	log_info(logger, "Conectado a Memoria en socket %d", socket_memoria);
	pthread_exit(0);
}

void *atender_memoria()
{
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de Memoria en socket %d", socket_memoria);
		t_paquete* paquete = recibir_paquete(logger,socket_memoria);

		switch(paquete->codigo_operacion){
		case RECIBIR_UNA_INSTRUCCION:
			t_memoria_cpu_instruccion *dato = deserializar_t_memoria_cpu_instruccion(paquete->buffer);

			log_debug(logger, "Instruccion recibida de Memoria: %s", dato->instruccion);
			log_debug(logger, "Cantidad de argumentos: %d", dato->cantidad_argumentos);
			log_debug(logger, "Argumento 1: %s", dato->argumento_1);
			log_debug(logger, "Argumento 2: %s", dato->argumento_2);
			log_debug(logger, "Argumento 3: %s", dato->argumento_3);
			log_debug(logger, "Argumento 4: %s", dato->argumento_4);
			log_debug(logger, "Argumento 5: %s", dato->argumento_5);

			free(dato->instruccion);
			free(dato->argumento_1);
			free(dato->argumento_2);
			free(dato->argumento_3);
			free(dato->argumento_4);
			free(dato->argumento_5);
			free(dato);

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

void *conectar_kernel_dispatch()
{
	socket_kernel_dispatch = esperar_cliente(logger, socket_server_dispatch);
	esperar_handshake(socket_kernel_dispatch);
	log_info(logger, "Kernel conectado por Dispatch en socket %d", socket_kernel_dispatch);
	pthread_exit(0);
}

void *atender_kernel_dispatch()
{
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de Kernel Dispatch en socket %d", socket_kernel_dispatch);
		t_paquete* paquete = recibir_paquete(logger,socket_kernel_dispatch);

		switch(paquete->codigo_operacion){
		case TERMINAR:
			kernel_orden_apagado = 0;
			liberar_conexion(socket_kernel_dispatch);
			liberar_conexion(socket_server_dispatch);
			break;
		default:
			liberar_conexion(socket_kernel_dispatch);
			liberar_conexion(socket_server_dispatch);
			pthread_exit(0);
			break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	pthread_exit(0);
}

void *conectar_kernel_interrupt()
{
	socket_kernel_interrupt = esperar_cliente(logger, socket_server_interrupt);
	esperar_handshake(socket_kernel_interrupt);
	log_info(logger, "Kernel conectado por Interrupt en socket %d", socket_kernel_interrupt);
	pthread_exit(0);
}

void *atender_kernel_interrupt()
{
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de Kernel Interrupt en socket %d", socket_kernel_interrupt);
		t_paquete* paquete = recibir_paquete(logger,socket_kernel_interrupt);

		switch(paquete->codigo_operacion){
		case MENSAJE:
			// placeholder
		default:
			liberar_conexion(socket_kernel_interrupt);
			liberar_conexion(socket_server_interrupt);
			pthread_exit(0);
			break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	pthread_exit(0);
}

t_memoria_cpu_instruccion* deserializar_t_memoria_cpu_instruccion(t_buffer* buffer) {

    t_memoria_cpu_instruccion* dato = malloc(sizeof(t_memoria_cpu_instruccion));
    void* stream = buffer->stream;
    
	// Deserializo el tamaño de la instruccion
	memcpy(&(dato->size_instruccion), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

	// Deserializo la instruccion
    dato->instruccion = malloc(dato->size_instruccion);
	memcpy(dato->instruccion, stream, dato->size_instruccion);
    stream += dato->size_instruccion * sizeof(char);
    
	// Deserializo la cantidad de argumentos
	memcpy(&(dato->cantidad_argumentos), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

	// Deserializo el tamaño del argumento 1
	memcpy(&(dato->size_argumento_1), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

	// Deserializo el argumento 1
    dato->argumento_1 = malloc(dato->size_argumento_1);
	memcpy(dato->argumento_1, stream, dato->size_argumento_1);
    stream += dato->size_argumento_1 * sizeof(char);

	// Deserializo el tamaño del argumento 2
	memcpy(&(dato->size_argumento_2), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	// Deserializo el argumento 2
	dato->argumento_2 = malloc(dato->size_argumento_2);
	memcpy(dato->argumento_2, stream, dato->size_argumento_2);
	stream += dato->size_argumento_2 * sizeof(char);

	// Deserializo el tamaño del argumento 3
	memcpy(&(dato->size_argumento_3), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	// Deserializo el argumento 3
	dato->argumento_3 = malloc(dato->size_argumento_3);
	memcpy(dato->argumento_3, stream, dato->size_argumento_3);
	stream += dato->size_argumento_3 * sizeof(char);

	// Deserializo el tamaño del argumento 4
	memcpy(&(dato->size_argumento_4), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	// Deserializo el argumento 4
	dato->argumento_4 = malloc(dato->size_argumento_4);
	memcpy(dato->argumento_4, stream, dato->size_argumento_4);
	stream += dato->size_argumento_4 * sizeof(char);

	// Deserializo el tamaño del argumento 5
	memcpy(&(dato->size_argumento_5), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	// Deserializo el argumento 5
	dato->argumento_5 = malloc(dato->size_argumento_5);
	memcpy(dato->argumento_5, stream, dato->size_argumento_5);
	stream += dato->size_argumento_5 * sizeof(char);

    return dato;
}