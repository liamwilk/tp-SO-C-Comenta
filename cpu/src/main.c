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

	t_paquete *paquete = crear_paquete(CPU_MEMORIA_PROXIMA_INSTRUCCION);
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
	while(1)
	{
		pthread_testcancel();

		log_debug(logger, "Esperando paquete de Memoria en socket %d", socket_memoria);
		t_paquete *paquete = recibir_paquete(logger, socket_memoria);
		
		if(paquete == NULL)
		{	
			log_warning(logger, "Memoria se desconecto del socket %d.", socket_memoria);
			break;
		}
	
		switch (paquete->codigo_operacion)
		{
		case MEMORIA_CPU_PROXIMA_INSTRUCCION:
		{
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
		}
		default:
			{	
			log_warning(logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
			eliminar_paquete(paquete);
			liberar_conexion(socket_memoria);
			pthread_exit(0);
		}
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
	liberar_conexion(socket_server_dispatch);
	pthread_exit(0);
}

void *atender_kernel_dispatch()
{
	while (1)
	{	
		pthread_testcancel();

		log_debug(logger, "Esperando paquete de Kernel Dispatch en socket %d", socket_kernel_dispatch);
		t_paquete *paquete = recibir_paquete(logger, socket_kernel_dispatch);

		if(paquete == NULL)
		{	
			log_warning(logger, "Kernel Dispatch se desconecto del socket %d.", socket_kernel_dispatch);
			break;
		}
		
		switch (paquete->codigo_operacion)
		{
		case PLACEHOLDER:
			revisar_paquete(paquete, logger, kernel_orden_apagado, "Kernel Dispatch");
			/*
			La logica
			*/
			break;
		case TERMINAR:
			
			revisar_paquete(paquete, logger, kernel_orden_apagado, "Kernel Dispatch");
			
			pthread_cancel(thread_atender_memoria);
			pthread_cancel(thread_atender_kernel_interrupt);
			pthread_cancel(thread_atender_kernel_dispatch);
			
			liberar_conexion(socket_memoria);
			liberar_conexion(socket_kernel_dispatch);
			liberar_conexion(socket_kernel_interrupt);
			

			break;
		case KERNEL_CPU_ENVIAR_REGISTROS:

			t_kernel_cpu_proceso *proceso_cpu = deserializar_t_kernel_cpu_proceso(paquete->buffer);
			log_debug(logger, "Registros cpu recibido de Kernel Dispatch");
			log_debug(logger, "PID: %d", proceso_cpu->pid);
			log_debug(logger, "PC: %d", proceso_cpu->registros.pc);
			log_debug(logger, "EAX: %d", proceso_cpu->registros.eax);
			log_debug(logger, "EBX: %d", proceso_cpu->registros.ebx);
			log_debug(logger, "ECX: %d", proceso_cpu->registros.ecx);
			log_debug(logger, "AX: %d", proceso_cpu->registros.ax);
			log_debug(logger, "BX: %d", proceso_cpu->registros.bx);
			log_debug(logger, "CX: %d", proceso_cpu->registros.cx);
			log_debug(logger, "DX: %d", proceso_cpu->registros.dx);

			// TODO: Continuar con la logica de recibir registros desde kernel
			free(proceso_cpu);
			break;
		default:
			{
			log_warning(logger, "[Kernel Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
			eliminar_paquete(paquete);
			liberar_conexion(socket_kernel_dispatch);
			pthread_exit(0);
			}
		}
		eliminar_paquete(paquete);
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
	liberar_conexion(socket_server_interrupt);
	pthread_exit(0);
}

void *atender_kernel_interrupt()
{	
	while(1)
	{
		pthread_testcancel();
		log_debug(logger, "Esperando paquete de Kernel Interrupt en socket %d", socket_kernel_interrupt);
		t_paquete *paquete = recibir_paquete(logger, socket_kernel_interrupt);
		
		if(paquete == NULL)
		{	
			log_warning(logger, "Kernel Interrupt se desconecto del socket %d.", socket_kernel_interrupt);
			break;
		}
		
		switch (paquete->codigo_operacion)
		{
		case PLACEHOLDER:
			revisar_paquete(paquete, logger, kernel_orden_apagado, "Interrupt");
			/*
			La logica
			*/
			break;
		default:
			{	
			log_warning(logger, "[Kernel Interrupt] Se recibio un codigo de operacion desconocido. Cierro hilo");
			eliminar_paquete(paquete);
			liberar_conexion(socket_kernel_interrupt);
			pthread_exit(0);
		}
		}
		eliminar_paquete(paquete);
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

t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer)
{
	t_kernel_cpu_proceso *proceso = malloc(sizeof(t_kernel_cpu_proceso));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(proceso->pid));
	deserializar_uint32_t(&stream, &(proceso->registros.pc));
	deserializar_uint32_t(&stream, &(proceso->registros.eax));
	deserializar_uint32_t(&stream, &(proceso->registros.ebx));
	deserializar_uint32_t(&stream, &(proceso->registros.ecx));
	deserializar_uint8_t(&stream, &(proceso->registros.ax));
	deserializar_uint8_t(&stream, &(proceso->registros.bx));
	deserializar_uint8_t(&stream, &(proceso->registros.cx));
	deserializar_uint8_t(&stream, &(proceso->registros.dx));
	return proceso;
}