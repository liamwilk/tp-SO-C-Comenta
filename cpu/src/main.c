/* Módulo CPU */

#include "main.h"

int main()
{

	t_log *logger;
	t_config *config;

	logger = iniciar_logger("cpu");
	config = iniciar_config(logger);

	t_cpu cpu=cpu_inicializar(config);

	cpu_log(cpu,logger);

	// Envio mensaje a Memoria

	int socketMemoria = crear_conexion(logger, cpu.ipMemoria, cpu.puertoMemoria,"cpu");
	enviar_mensaje("Hola, soy CPU!", socketMemoria);
	liberar_conexion(socketMemoria);

	// Inicio servidor de CPU

	int serverDispatch_fd = iniciar_servidor(logger, "cpu",NULL,cpu.puertoEscuchaDispatch);
	log_info(logger, "Servidor listo para recibir al cliente en Dispatch.");

	// Falta abrir Interrupt tambien, hay que hacerlo bien de una con hilos
	// int serverInterrupt_fd = iniciar_servidor(logger,puertoEscuchaInterrupt);
	// log_info(logger, "Servidor listo para recibir al cliente en Interrupt.");

	int contador = 0;

	while (contador < 1)
	{
		int cliente_fd = esperar_cliente(logger, "cpu" ,serverDispatch_fd);
		int cod_op = recibir_operacion(cliente_fd);

		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(logger, cliente_fd);
			contador++;
			break;
		default:
			log_warning(logger, "Operacion desconocida.");
			break;
		}
	}

	log_destroy(logger);
	config_destroy(config);

	free(cpu.ipMemoria);
	free(cpu.algoritmoTlb);

	return 0;
}


void comunicarConCliente(t_paqueteCliente *cliente)
{
	cliente->socket = crear_conexion(cliente->logger, cliente->ip, cliente->puerto,"cpu");
	enviar_mensaje("Hola, soy Cpu", cliente->socket);
}
