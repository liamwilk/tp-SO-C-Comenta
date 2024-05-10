/* Módulo Kernel */
#include "main.h"

int main()
{
	logger = iniciar_logger("kernel", LOG_LEVEL_DEBUG);
	config = iniciar_config(logger);
	kernel = kernel_inicializar(config);
	estados = kernel_inicializar_estados(&estados);
	kernel_log(kernel, logger);
	inicializar_args();
	inicializar_semaforos();

	/*----HILOS----*/

	hilos_memoria_inicializar(&args, thread_conectar_memoria, args.kernel->threads.thread_atender_memoria);

	hilos_cpu_inicializar(&args, thread_conectar_cpu_dispatch, args.kernel->threads.thread_atender_cpu_dispatch, thread_conectar_cpu_interrupt, args.kernel->threads.thread_atender_cpu_interrupt);

	kernel.sockets.server = iniciar_servidor(logger, kernel.puertoEscucha);

	hilos_io_inicializar(&args, args.kernel->threads.thread_atender_entrada_salida);

	hilos_planificador_inicializar(&args, args.kernel->threads.thread_planificador);

	hilos_consola_inicializar(&args, args.kernel->threads.thread_atender_consola);

	log_destroy(logger);
	config_destroy(config);
	sem_destroy(&kernel.iniciar_planificador);

	return 0;
}

void inicializar_args()
{
	args.logger = logger;
	args.kernel = &kernel;
	args.estados = &estados;
	args.kernel_orden_apagado = &kernel_orden_apagado;
	args.kernel->sockets.entrada_salida_generic = 0;
	args.kernel->sockets.entrada_salida_stdin = 0;
	args.kernel->sockets.entrada_salida_stdout = 0;
	args.kernel->sockets.entrada_salida_dialfs = 0;
	args.kernel->sockets.entrada_salida = 0;
}

void inicializar_semaforos()
{
	sem_init(&kernel.iniciar_planificador, 0, 0);
	sem_init(&kernel.detener_planificador, 0, 1);
	sem_init(&kernel.actualizar_planificador, 0, 1);
}