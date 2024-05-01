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

	pthread_create(&thread_esperar_kernel_dispatch, NULL, esperar_kernel_dispatch, NULL);
	pthread_join(thread_esperar_kernel_dispatch, NULL);

	pthread_create(&thread_atender_kernel_dispatch, NULL, atender_kernel_dispatch, NULL);
	pthread_detach(thread_atender_kernel_dispatch);

	pthread_create(&thread_esperar_kernel_interrupt, NULL, esperar_kernel_interrupt, NULL);
	pthread_join(thread_esperar_kernel_interrupt, NULL);

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

	if (socket_memoria == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = crear_handshake(logger, socket_memoria, MEMORIA_CPU, "Memoria");

	if (resultado != CORRECTO)
	{
		liberar_conexion(socket_memoria);
		pthread_exit(0);
	}

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
			revisar_paquete(paquete, logger, kernel_orden_apagado, "Memoria");
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

void *esperar_kernel_dispatch()
{
	socket_kernel_dispatch = esperar_conexion(logger, socket_server_dispatch);

	if (socket_kernel_dispatch == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_kernel_dispatch, CPU_DISPATCH_KERNEL, "Kernel por Dispatch");

	if (resultado != CORRECTO)
	{
		liberar_conexion(socket_kernel_dispatch);
		pthread_exit(0);
	}

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
			revisar_paquete(paquete, logger, kernel_orden_apagado, "Dispatch");
			/*
			La logica
			*/
			break;
		case TERMINAR:
			kernel_orden_apagado = 0;
			liberar_conexion(socket_kernel_dispatch);
			liberar_conexion(socket_server_dispatch);
			break;
		case RECIBIR_REGISTROS_CPU:

			t_registros_cpu *registros_cpu = deserializar_t_registros_cpu(paquete->buffer);
			log_debug(logger, "Registros cpu recibido de Kernel Dispatch");
			log_debug(logger, "PC: %d", registros_cpu->pc);
			log_debug(logger, "EAX: %d", registros_cpu->eax);
			log_debug(logger, "EBX: %d", registros_cpu->ebx);
			log_debug(logger, "ECX: %d", registros_cpu->ecx);
			log_debug(logger, "AX: %d", registros_cpu->ax);
			log_debug(logger, "BX: %d", registros_cpu->bx);
			log_debug(logger, "CX: %d", registros_cpu->cx);
			log_debug(logger, "DX: %d", registros_cpu->dx);

			// TODO: Continuar con la logica de recibir registros desde kernel
			free(registros_cpu);

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

void *esperar_kernel_interrupt()
{
	socket_kernel_interrupt = esperar_conexion(logger, socket_server_interrupt);

	if (socket_kernel_interrupt == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_kernel_interrupt, CPU_INTERRUPT_KERNEL, "Kernel por Interrupt");

	if (resultado != CORRECTO)
	{
		liberar_conexion(socket_kernel_interrupt);
		pthread_exit(0);
	}

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
			revisar_paquete(paquete, logger, kernel_orden_apagado, "Interrupt");
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
	t_memoria_cpu_instruccion *dato = malloc(sizeof(t_memoria_cpu_instruccion));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(dato->size_instruccion));
	deserializar_char(&stream, &(dato->instruccion), dato->size_instruccion);
	deserializar_uint32_t(&stream, &(dato->cantidad_argumentos));
	deserializar_uint32_t(&stream, &(dato->size_argumento_1));
	deserializar_char(&stream, &(dato->argumento_1), dato->size_argumento_1);
	deserializar_uint32_t(&stream, &(dato->size_argumento_2));
	deserializar_char(&stream, &(dato->argumento_2), dato->size_argumento_2);
	deserializar_uint32_t(&stream, &(dato->size_argumento_3));
	deserializar_char(&stream, &(dato->argumento_3), dato->size_argumento_3);
	deserializar_uint32_t(&stream, &(dato->size_argumento_4));
	deserializar_char(&stream, &(dato->argumento_4), dato->size_argumento_4);
	deserializar_uint32_t(&stream, &(dato->size_argumento_5));
	deserializar_char(&stream, &(dato->argumento_5), dato->size_argumento_5);

	return dato;
}

t_registros_cpu *deserializar_t_registros_cpu(t_buffer *buffer)
{
	t_registros_cpu *registros_cpu = malloc(sizeof(t_registros_cpu));

	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(registros_cpu->pc));
	deserializar_uint32_t(&stream, &(registros_cpu->eax));
	deserializar_uint32_t(&stream, &(registros_cpu->ebx));
	deserializar_uint32_t(&stream, &(registros_cpu->ecx));
	deserializar_uint8_t(&stream, &(registros_cpu->ax));
	deserializar_uint8_t(&stream, &(registros_cpu->bx));
	deserializar_uint8_t(&stream, &(registros_cpu->cx));
	deserializar_uint8_t(&stream, &(registros_cpu->dx));

	return registros_cpu;
}