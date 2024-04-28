/* Módulo Kernel */

#include "main.h"
#include "consola.h"

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

	pthread_create(&thread_conectar_cpu_dispatch, NULL, conectar_cpu_dispatch, NULL);
	pthread_join(thread_conectar_cpu_dispatch, NULL);

	pthread_create(&thread_atender_cpu_dispatch, NULL, atender_cpu_dispatch, NULL);
	pthread_detach(thread_atender_cpu_dispatch);

	pthread_create(&thread_conectar_cpu_interrupt, NULL, conectar_cpu_interrupt, NULL);
	pthread_join(thread_conectar_cpu_interrupt, NULL);

	pthread_create(&thread_atender_cpu_interrupt, NULL, atender_cpu_interrupt, NULL);
	pthread_detach(thread_atender_cpu_interrupt);

	// Inicio server Kernel

	kernel.sockets.server = iniciar_servidor(logger, kernel.puertoEscucha);
	log_info(logger, "Servidor listo para recibir clientes en socket %d.", kernel.sockets.server);
	estados = kernel_inicializar_estados(&estados);

	pthread_create(&thread_conectar_io, NULL, conectar_io, NULL);
	pthread_detach(thread_conectar_io);

	pthread_create(&thread_atender_consola, NULL, atender_consola, NULL);
	pthread_join(thread_atender_consola, NULL);

	log_warning(logger, "El Usuario solicito el apagado del sistema operativo.");

	log_destroy(logger);
	config_destroy(config);

	return 0;
}

void *atender_consola()
{
	consola_iniciar(logger, &kernel, &estados, &kernel_orden_apagado);
	return NULL;
};

void *conectar_io()
{
	while (kernel_orden_apagado)
	{
		int socket_cliente = esperar_cliente(logger, kernel.sockets.server);

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
	do
	{
		log_info(logger, "Esperando paquete de I/O en socket %d", socket_cliente);
		int cod_op = recibir_operacion(socket_cliente);

		switch (cod_op)
		{
		case MENSAJE:
			// placeholder
			break;
		default:
			liberar_conexion(socket_cliente);
			pthread_exit(0);
			break;
		}
	} while (kernel_orden_apagado);

	pthread_exit(0);
}

void *conectar_memoria()
{
	int socket = crear_conexion(logger, kernel.ipMemoria, kernel.puertoMemoria);
	kernel_sockets_agregar(&kernel, MEMORIA, socket);
	handshake(logger, socket, 1, "Memoria");
	log_info(logger, "Conectado a Memoria en socket %d", socket);
	pthread_exit(0);
}

void *atender_memoria()
{
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de Memoria en socket %d", kernel.sockets.memoria);
		int cod_op = recibir_operacion(kernel.sockets.memoria);

		switch (cod_op)
		{
		case MENSAJE:
			// placeholder para despues
			break;
		default:
			liberar_conexion(kernel.sockets.memoria);
			pthread_exit(0);
			break;
		}
	}

	pthread_exit(0);
}

void *conectar_cpu_dispatch()
{
	int socket = crear_conexion(logger, kernel.ipCpu, kernel.puertoCpuDispatch);
	kernel_sockets_agregar(&kernel, CPU_DISPATCH, socket);
	handshake(logger, socket, 1, "CPU Dispatch");
	log_info(logger, "Conectado a CPU por Dispatch en socket %d", socket);
	pthread_exit(0);
}

void *atender_cpu_dispatch()
{
	int socket = kernel.sockets.cpu_dispatch;
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de CPU Dispatch en socket %d", socket);
		int cod_op = recibir_operacion(socket);

		switch (cod_op)
		{
		case MENSAJE:
			// Placeholder
			break;
		default:
			liberar_conexion(socket);
			pthread_exit(0);
			break;
		}
	}

	pthread_exit(0);
}

void *conectar_cpu_interrupt()
{
	int socket = crear_conexion(logger, kernel.ipCpu, kernel.puertoCpuInterrupt);
	kernel_sockets_agregar(&kernel, CPU_INTERRUPT, socket);
	handshake(logger, socket, 1, "CPU Interrupt");
	log_info(logger, "Conectado a CPU por Interrupt en socket %d", socket);
	pthread_exit(0);
}

void *atender_cpu_interrupt()
{
	int socket = kernel.sockets.cpu_interrupt;
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de CPU Interrupt en socket %d", socket);
		int cod_op = recibir_operacion(socket);

		switch (cod_op)
		{
		case MENSAJE:
			// Placeholder
			break;
		default:
			liberar_conexion(socket);
			pthread_exit(0);
			break;
		}
	}

	pthread_exit(0);
}