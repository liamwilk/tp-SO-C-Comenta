/* Módulo Kernel */

#include "main.h"

int main()
{
	/*Inicializacion*/
	logger = iniciar_logger("kernel");
	config = iniciar_config(logger);

	t_kernel kernel = kernel_inicializar(config);

	kernel_log(kernel, logger);
	
	/*Handshakes*/
	t_handshake handshakeMemoria = kernel_handshake_memoria(kernel,hilo_handshake,logger);
	t_handshake handshakeCpuDispatch= kernel_handshake_cpu_dispatch(kernel,hilo_handshake,logger);
	t_handshake handshakeCpuInterrupt = kernel_handshake_cpu_interrupt(kernel,hilo_handshake,logger);


	iniciar_servidor(logger, "kernel",NULL,kernel.puertoEscucha);

	log_info(logger, "Servidor listo para recibir al cliente");

	/// TODO: Aca hay que implementar el controlador de consolas.
	

	hilo_exit();

	// Libero todas las conexiones
	liberar_conexion(handshakeMemoria.socketDestino);
	liberar_conexion(handshakeCpuDispatch.socketDestino);
	liberar_conexion(handshakeCpuInterrupt.socketDestino);


	// Destruyo estructuras
	config_destroy(config);
	log_destroy(logger);
}



