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
	instruccion.program_counter = 17;

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
		liberar_conexion(&socket_memoria);
		pthread_exit(0);
	}

	log_debug(logger, "Conectado a Memoria en socket %d", socket_memoria);
	pthread_exit(0);
}

void *atender_memoria()
{
	hilo_ejecutar(logger, socket_memoria, "Memoria", switch_case_memoria);
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
		liberar_conexion(&socket_kernel_dispatch);
		pthread_exit(0);
	}

	log_debug(logger, "Kernel conectado por Dispatch en socket %d", socket_kernel_dispatch);
	liberar_conexion(&socket_server_dispatch);
	pthread_exit(0);
}

void *atender_kernel_dispatch()
{
	hilo_ejecutar(logger, socket_kernel_dispatch, "Kernel Dispatch", switch_case_kernel_dispatch);
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
		liberar_conexion(&socket_kernel_interrupt);
		pthread_exit(0);
	}

	log_debug(logger, "Kernel conectado por Interrupt en socket %d", socket_kernel_interrupt);
	liberar_conexion(&socket_server_interrupt);
	pthread_exit(0);
}

void *atender_kernel_interrupt()
{
	hilo_ejecutar(logger, socket_kernel_interrupt, "Kernel Interrupt", switch_case_kernel_interrupt);
	pthread_exit(0);
}

void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_CPU_PROXIMA_INSTRUCCION:
	{
		// FETCH
		t_instruccion *TIPO_INSTRUCCION = malloc(sizeof(t_instruccion));
		int debeTerminar = cpu_memoria_recibir_instruccion(buffer, logger, &instruccion, TIPO_INSTRUCCION, &proceso);

		if (debeTerminar)
		{
			cpu_kernel_avisar_finalizacion(proceso, socket_kernel_interrupt);
			break;
		}

		// EXECUTE:
		cpu_ejecutar_instruccion(&instruccion, *TIPO_INSTRUCCION, &proceso, logger);
		cpu_memoria_pedir_proxima_instruccion(&proceso, socket_memoria);
		log_debug(logger, "Instruccion pedida a Memoria");
		free(TIPO_INSTRUCCION);
		break;
	}
	case PLACEHOLDER:
	{
		// PLACEHOLDER
	}
	default:
	{
		log_warning(logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&socket_memoria);
		break;
	}
	}
}

void switch_case_kernel_dispatch(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer)
{

	switch (codigo_operacion)
	{
	case PLACEHOLDER:
	{
		/*
		La logica
		*/
		break;
	}
	case KERNEL_CPU_EJECUTAR_PROCESO:
	{
		/*RECIBIR PROCESO*/
		proceso = cpu_kernel_recibir_proceso(buffer, logger);
		/*PEDIR INSTRUCCION*/
		cpu_memoria_pedir_proxima_instruccion(&proceso, socket_memoria);
		log_debug(logger, "Instruccion pedida a Memoria");
		break;
	}
	default:
	{
		log_warning(logger, "[Kernel Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&socket_kernel_dispatch);
		break;
	}
	}
}

void switch_case_kernel_interrupt(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer)
{

	switch (codigo_operacion)
	{
	case PLACEHOLDER:
	{
		/*
		La logica
		*/
		break;
	}
	case FINALIZAR_SISTEMA:
	{
		pthread_cancel(thread_atender_memoria);
		pthread_cancel(thread_atender_kernel_dispatch);
		pthread_cancel(thread_atender_kernel_interrupt);

		liberar_conexion(&socket_memoria);
		liberar_conexion(&socket_kernel_dispatch);
		liberar_conexion(&socket_kernel_interrupt);

		break;
	}
	default:
	{
		log_warning(logger, "[Kernel Interrupt] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&socket_kernel_interrupt);
		break;
	}
	}
}
