/* Módulo Memoria */
#include "main.h"

int main()
{

	// Inicio el logger y la configuracion
	logger = iniciar_logger("memoria");
	config = iniciar_config(logger);

	// Cargo la configuracion
	t_memoria memoria = memoria_inicializar(config);

	// Logeo la configuracion
	memoria_log(memoria, logger);

	// Levanto el servidor de memoria
	iniciar_servidor(logger, "memoria", NULL, memoria.puertoEscucha);
	log_info(logger, "Memoria está lista para recibir clientes.");

	// Espero la conexion del Kernel
	// (para saber cual es cual cuando haya más,
	// hay que poner semaforos y ordenarlos)

	pthread_create(&kernelThread, NULL, esperar_kernel, NULL);
	pthread_detach(kernelThread);

	// Espero que todos los threads finalicen
	pthread_exit(0);

	// Libero punteros
	liberar_conexion(socketKernel);

	// Destruyo estructuras
	log_destroy(logger);
	config_destroy(config);
}

void *esperar_kernel()
{
	socketKernel = nuevo_socket();
	while (1)
	{
		int cod_op = recibir_operacion(socketKernel);
		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(logger, socketKernel);
			break;
		}
	}
	return NULL;
}

int nuevo_socket()
{
	int nuevoSocket = esperar_cliente(logger, "memoria", serverMemoria);
	uint32_t ok = 0;
	uint32_t error = -1;

	recv(nuevoSocket, &respuesta, sizeof(uint32_t), MSG_WAITALL);
	if (respuesta == 1)
	{
		send(nuevoSocket, &ok, sizeof(uint32_t), 0);
	}
	else
	{
		send(nuevoSocket, &error, sizeof(uint32_t), 0);
		log_error(logger, "Error en el manejo de handhsake.");
	}
	return nuevoSocket;
}