/* Módulo CPU */

#include "main.h"

int main()
{
	logger = iniciar_logger("cpu", LOG_LEVEL_DEBUG);
	config = iniciar_config(logger);
	cpu = cpu_inicializar(config);
	cpu_log(cpu, logger);

	socket_server_dispatch = iniciar_servidor(logger, cpu.puertoEscuchaDispatch);
	log_debug(logger, "Servidor Dispatch listo para recibir al cliente en socket %d", socket_server_dispatch);

	socket_server_interrupt = iniciar_servidor(logger, cpu.puertoEscuchaInterrupt);
	log_debug(logger, "Servidor Interrupt listo para recibir al cliente en socket %d", socket_server_interrupt);

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

	/* Ejemplo de envio de instruccion a Memoria
	sleep(15);

	t_paquete *paquete = crear_paquete(PROXIMA_INSTRUCCION);
	t_cpu_memoria_instruccion instruccion;
	
	instruccion.pid = 1;
	instruccion.program_counter = 0;

	actualizar_buffer(paquete, sizeof(uint32_t) + sizeof(uint32_t));
	
	serializar_uint32_t(instruccion.program_counter, paquete);
	
	serializar_uint32_t(instruccion.pid, paquete);
	
	// Envio la instruccion a Memoria
	enviar_paquete(paquete, socket_memoria);

	log_debug(logger,"Paquete enviado a Memoria");

	// Libero la memoria del paquete de instruccion
	eliminar_paquete(paquete);
	*/

	pthread_create(&thread_atender_kernel_interrupt, NULL, atender_kernel_interrupt, NULL);
	pthread_join(thread_atender_kernel_interrupt, NULL);

	log_info(logger, "Kernel solicito el apagado del sistema operativo.");

	log_destroy(logger);
	config_destroy(config);

	return 0;
	
}

void *conectar_memoria()
{
	socket_memoria = crear_conexion(logger, cpu.ipMemoria, cpu.puertoMemoria);
	handshake(logger, socket_memoria, 1, "Memoria");
	log_debug(logger, "Conectado a Memoria en socket %d", socket_memoria);
	pthread_exit(0);
}

void *atender_memoria()
{
	while (kernel_orden_apagado)
	{
		log_debug(logger, "Esperando paquete de Memoria en socket %d", socket_memoria);
		t_paquete *paquete = recibir_paquete(logger, socket_memoria);
		switch (paquete->codigo_operacion)
		{
		case PROXIMA_INSTRUCCION:
			revisar_paquete(paquete,logger,kernel_orden_apagado,"Memoria");
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
		eliminar_paquete(paquete);
	}

	pthread_exit(0);
}

void *conectar_kernel_dispatch()
{
	socket_kernel_dispatch = esperar_cliente(logger, socket_server_dispatch);
	esperar_handshake(socket_kernel_dispatch);
	log_debug(logger, "Kernel conectado por Dispatch en socket %d", socket_kernel_dispatch);
	pthread_exit(0);
}

void *atender_kernel_dispatch()
{
	while (kernel_orden_apagado)
	{
		log_debug(logger, "Esperando paquete de Kernel Dispatch en socket %d", socket_kernel_dispatch);
		t_paquete *paquete = recibir_paquete(logger, socket_kernel_dispatch);
		switch (paquete->codigo_operacion)
		{
		case MENSAJE:
			revisar_paquete(paquete,logger,kernel_orden_apagado,"Dispatch");
			/*
			La logica
			*/
			break;
		case TERMINAR:
			kernel_orden_apagado = 0;
			liberar_conexion(socket_kernel_dispatch);
			liberar_conexion(socket_server_dispatch);
			break;
		case RECIBIR_PCB:
			
			t_pcb *pcb = deserializar_t_pcb(paquete->buffer);
			log_debug(logger, "PCB recibido de Kernel Dispatch");
			log_debug(logger, "PID: %d", pcb->pid);
			log_debug(logger, "Program Counter: %d", pcb->program_counter);
			log_debug(logger, "Quantum: %d", pcb->quantum);
			log_debug(logger, "PC: %d", pcb->registros_cpu->pc);
			log_debug(logger, "EAX: %d", pcb->registros_cpu->eax);
			log_debug(logger, "EBX: %d", pcb->registros_cpu->ebx);
			log_debug(logger, "ECX: %d", pcb->registros_cpu->ecx);
			log_debug(logger, "AX: %d", pcb->registros_cpu->ax);
			log_debug(logger, "BX: %d", pcb->registros_cpu->bx);
			log_debug(logger, "CX: %d", pcb->registros_cpu->cx);
			log_debug(logger, "DX: %d", pcb->registros_cpu->dx);
			
			free(pcb->registros_cpu);
			free(pcb);

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
	log_debug(logger, "Kernel conectado por Interrupt en socket %d", socket_kernel_interrupt);
	pthread_exit(0);
}

void *atender_kernel_interrupt()
{
	while (kernel_orden_apagado)
	{
		log_debug(logger, "Esperando paquete de Kernel Interrupt en socket %d", socket_kernel_interrupt);
		t_paquete *paquete = recibir_paquete(logger, socket_kernel_interrupt);

		

		switch (paquete->codigo_operacion)
		{
		case MENSAJE:
			revisar_paquete(paquete,logger,kernel_orden_apagado,"Interrupt");
			/*
			La logica
			*/
			break;
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

t_memoria_cpu_instruccion *deserializar_t_memoria_cpu_instruccion(t_buffer *buffer)
{
	// Pido memoria para la estructura
	t_memoria_cpu_instruccion *dato = malloc(sizeof(t_memoria_cpu_instruccion));
	void *stream = buffer->stream;

	// Deserializo el tamaño de la instruccion

	deserializar_uint32_t(stream, &(dato->size_instruccion));

	// memcpy(&(dato->size_instruccion), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_char(stream, &(dato->instruccion), dato->size_instruccion);

	// Deserializo la instruccion
	// dato->instruccion = malloc(dato->size_instruccion);
	// memcpy(dato->instruccion, stream, dato->size_instruccion);
	// stream += dato->size_instruccion * sizeof(char);

	deserializar_uint32_t(stream, &(dato->cantidad_argumentos));

	// Deserializo la cantidad de argumentos
	// memcpy(&(dato->cantidad_argumentos), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint32_t(stream, &(dato->size_argumento_1));

	// Deserializo el tamaño del argumento 1
	// memcpy(&(dato->size_argumento_1), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);	

	deserializar_char(stream, &(dato->argumento_1), dato->size_argumento_1);

	// Deserializo el argumento 1
	// dato->argumento_1 = malloc(dato->size_argumento_1);
	// memcpy(dato->argumento_1, stream, dato->size_argumento_1);
	// stream += dato->size_argumento_1 * sizeof(char);

	deserializar_uint32_t(stream, &(dato->size_argumento_2));

	// // Deserializo el tamaño del argumento 2
	// memcpy(&(dato->size_argumento_2), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_char(stream, &(dato->argumento_2), dato->size_argumento_2);

	// // Deserializo el argumento 2
	// dato->argumento_2 = malloc(dato->size_argumento_2);
	// memcpy(dato->argumento_2, stream, dato->size_argumento_2);
	// stream += dato->size_argumento_2 * sizeof(char);

	deserializar_uint32_t(stream, &(dato->size_argumento_3));

	// // Deserializo el tamaño del argumento 3
	// memcpy(&(dato->size_argumento_3), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_char(stream, &(dato->argumento_3), dato->size_argumento_3);

	// Deserializo el argumento 3
	// dato->argumento_3 = malloc(dato->size_argumento_3);
	// memcpy(dato->argumento_3, stream, dato->size_argumento_3);
	// stream += dato->size_argumento_3 * sizeof(char);

	deserializar_uint32_t(stream, &(dato->size_argumento_4));

	// // Deserializo el tamaño del argumento 4
	// memcpy(&(dato->size_argumento_4), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_char(stream, &(dato->argumento_4), dato->size_argumento_4);

	// // Deserializo el argumento 4
	// dato->argumento_4 = malloc(dato->size_argumento_4);
	// memcpy(dato->argumento_4, stream, dato->size_argumento_4);
	// stream += dato->size_argumento_4 * sizeof(char);

	deserializar_uint32_t(stream, &(dato->size_argumento_5));

	// // Deserializo el tamaño del argumento 5
	// memcpy(&(dato->size_argumento_5), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_char(stream, &(dato->argumento_5), dato->size_argumento_5);

	// Deserializo el argumento 5
	// dato->argumento_5 = malloc(dato->size_argumento_5);
	// memcpy(dato->argumento_5, stream, dato->size_argumento_5);
	// stream += dato->size_argumento_5 * sizeof(char);

	return dato;
}

t_pcb *deserializar_t_pcb(t_buffer *buffer)
{	
	// Pido memoria para las estructuras
	t_pcb *pcb = malloc(sizeof(t_pcb));
	pcb->registros_cpu = malloc(sizeof(t_registros_cpu));

	void *stream = buffer->stream;

	deserializar_uint32_t(stream, &(pcb->pid));

	// memcpy(&(pcb->pid), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint32_t(stream, &(pcb->program_counter));

	// memcpy(&(pcb->program_counter), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint32_t(stream, &(pcb->quantum));

	// memcpy(&(pcb->quantum), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint32_t(stream, &(pcb->registros_cpu->pc));

	// memcpy(&(pcb->registros_cpu->pc), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint32_t(stream, &(pcb->registros_cpu->eax));

	// memcpy(&(pcb->registros_cpu->eax), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint32_t(stream, &(pcb->registros_cpu->ebx));

	// memcpy(&(pcb->registros_cpu->ebx), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint32_t(stream, &(pcb->registros_cpu->ecx));

	// memcpy(&(pcb->registros_cpu->ecx), stream, sizeof(uint32_t));
	// stream += sizeof(uint32_t);

	deserializar_uint8_t(stream, &(pcb->registros_cpu->ax));

	// memcpy(&(pcb->registros_cpu->ax), stream, sizeof(uint8_t));
	// stream += sizeof(uint8_t);

	deserializar_uint8_t(stream, &(pcb->registros_cpu->bx));

	// memcpy(&(pcb->registros_cpu->bx), stream, sizeof(uint8_t));
	// stream += sizeof(uint8_t);

	deserializar_uint8_t(stream, &(pcb->registros_cpu->cx));

	// memcpy(&(pcb->registros_cpu->cx), stream, sizeof(uint8_t));
	// stream += sizeof(uint8_t);

	deserializar_uint8_t(stream, &(pcb->registros_cpu->dx));

	// memcpy(&(pcb->registros_cpu->dx), stream, sizeof(uint8_t));
	// stream += sizeof(uint8_t);

	return pcb;
}