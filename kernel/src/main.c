/* Módulo Kernel */

#include "main.h"
#include "consola.h"

int main()
{

	logger = iniciar_logger("kernel", LOG_LEVEL_INFO);
	config = iniciar_config(logger);
	kernel = kernel_inicializar(config);
	kernel_log(kernel, logger);
	// Creo las conexiones a Memoria, CPU Dispatch y CPU Interrupt. Voy atendiendo las peticiones a medida que las abro.

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

	sockets.server = iniciar_servidor(logger, kernel.puertoEscucha);
	log_info(logger, "Servidor listo para recibir clientes en socket %d.", sockets.server);

	// Conecto interfaces de I/O y levanto consola interactiva (en join para que no finalice el main hasta que el usuario termine)

	pthread_create(&thread_conectar_io, NULL, conectar_io, NULL);
	pthread_detach(thread_conectar_io);

	pthread_create(&thread_atender_consola, NULL, atender_consola, NULL);
	pthread_join(thread_atender_consola, NULL);

	// Libero

	log_destroy(logger);
	config_destroy(config);

	return 0;
}

void *atender_consola()
{
	procesar_consola(logger, &pid, new, &kernel_orden_apagado, &kernel, &sockets);
}

void *conectar_io()
{
	while (kernel_orden_apagado)
	{
		log_debug(logger, "Estoy esperando");
		int socket_cliente = esperar_cliente(logger, sockets.server);

		if (socket_cliente == -1)
		{
			break;
		}

		pthread_t hilo;
		int *args = malloc(sizeof(int));
		*args = socket_cliente;
		pthread_create(&hilo, NULL, atender_io, args);
		pthread_detach(hilo);
	}

	log_warning(logger, "Kernel solicitó el apagado del sistema operativo. Se cierra servidor de atencion I/O.");

	pthread_exit(0);
}

void *atender_io(void *args)
{
	int socket_cliente = *(int *)args;
	esperar_handshake(socket_cliente);
	log_info(logger, "I/O conectado en socket %d", socket_cliente);
	free(args);

	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de I/O en socket %d", socket_cliente);
		int cod_op = recibir_operacion(socket_cliente);

		switch (cod_op)
		{
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

void *conectar_memoria()
{
	sockets.memoria = crear_conexion(logger, kernel.ipMemoria, kernel.puertoMemoria);
	handshake(logger, sockets.memoria, 1, "Memoria");
	log_info(logger, "Conectado a Memoria en socket %d", sockets.memoria);
	pthread_exit(0);
}

void *atender_memoria()
{
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de Memoria en socket %d", sockets.memoria);
		int cod_op = recibir_operacion(sockets.memoria);

		switch (cod_op)
		{
		case MENSAJE:
			// placeholder para despues
			break;
		default:
			log_info(logger, "Solicitud de desconexion con Memoria. Cerrando socket %d", sockets.memoria);
			liberar_conexion(sockets.memoria);
			pthread_exit(0);
			break;
		}
	}

	pthread_exit(0);
}

void *conectar_cpu_dispatch()
{
	sockets.cpu_dispatch = crear_conexion(logger, kernel.ipCpu, kernel.puertoCpuDispatch);
	handshake(logger, sockets.cpu_dispatch, 1, "CPU Dispatch");
	log_info(logger, "Conectado a CPU por Dispatch en socket %d", sockets.cpu_dispatch);
	pthread_exit(0);
}

void *atender_cpu_dispatch()
{
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de CPU Dispatch en socket %d", sockets.cpu_dispatch);
		int cod_op = recibir_operacion(sockets.cpu_dispatch);

		switch (cod_op)
		{
		case MENSAJE:
			// Placeholder
			break;
		default:
			log_info(logger, "Solicitud de desconexion con CPU Dispatch. Cerrando socket %d", sockets.memoria);
			liberar_conexion(sockets.memoria);
			pthread_exit(0);
			break;
		}
	}

	pthread_exit(0);
}

void *conectar_cpu_interrupt()
{
	sockets.cpu_interrupt = crear_conexion(logger, kernel.ipCpu, kernel.puertoCpuInterrupt);
	handshake(logger, sockets.cpu_interrupt, 1, "CPU Interrupt");
	log_info(logger, "Conectado a CPU por Interrupt en socket %d", sockets.cpu_interrupt);
	pthread_exit(0);
}

void *atender_cpu_interrupt()
{
	while (kernel_orden_apagado)
	{
		log_info(logger, "Esperando paquete de CPU Interrupt en socket %d", sockets.cpu_interrupt);
		int cod_op = recibir_operacion(sockets.cpu_interrupt);

		switch (cod_op)
		{
		case MENSAJE:
			// Placeholder
			break;
		default:
			log_info(logger, "Solicitud de desconexion con CPU Interrupt. Cerrando socket %d", sockets.cpu_interrupt);
			liberar_conexion(sockets.cpu_interrupt);
			pthread_exit(0);
			break;
		}
	}

	pthread_exit(0);
}