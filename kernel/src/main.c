/* Módulo Kernel */
#include "main.h"

int main()
{
	logger = iniciar_logger("kernel", LOG_LEVEL_DEBUG);
	config = iniciar_config(logger);
	kernel = kernel_inicializar(config);
	estados = kernel_inicializar_estados(&estados);
	kernel_log(kernel, logger);
	hilos_args args = {logger, &kernel, &estados, &kernel_orden_apagado};

	/*----HILOS----*/
	hilos_memoria_inicializar(&args, thread_conectar_memoria, thread_atender_memoria);
	hilos_cpu_inicializar(&args, thread_conectar_cpu_dispatch, thread_atender_cpu_dispatch, thread_conectar_cpu_interrupt, thread_atender_cpu_interrupt);
	kernel.sockets.server = iniciar_servidor(logger, kernel.puertoEscucha);
	log_debug(logger, "Servidor listo para recibir clientes en socket %d.", kernel.sockets.server);
	hilos_io_inicializar(&args, thread_esperar_entrada_salida);
	hilos_consola_inicializar(&args, thread_atender_consola);
	log_warning(logger, "El Usuario solicito el apagado del sistema operativo.");
	log_destroy(logger);
	config_destroy(config);

	return 0;
}
