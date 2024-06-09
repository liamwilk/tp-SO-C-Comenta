/* Módulo Kernel */
#include "main.h"

int main(int argc, char *argv[])
{
	logger = iniciar_logger("kernel", LOG_LEVEL_DEBUG);
	inicializar_config(&config, logger, argc, argv);
	kernel = kernel_inicializar(config);
	estados = kernel_inicializar_estados(&estados);
	temporizador.args = &args;
	recursos = dictionary_create();
	inicializar_args();
	inicializar_semaforos();
	inicializar_temporizador(&args, &temporizador);
	pthread_mutex_init(&kernel.lock, NULL);

	kernel_log(&args);

	kernel_recurso_init(recursos, kernel.instanciasRecursos, kernel.recursos);
	kernel_recurso_log(&args);

	// Inicializo las estructuras que almacenan e identifican los sockets de entrada/salida
	kernel.sockets.dictionary_entrada_salida = dictionary_create();
	kernel.sockets.list_entrada_salida = list_create();

	/*----HILOS----*/

	hilos_memoria_inicializar(&args, thread_conectar_memoria, args.kernel->threads.thread_atender_memoria);
	hilos_cpu_inicializar(&args, thread_conectar_cpu_dispatch, args.kernel->threads.thread_atender_cpu_dispatch, thread_conectar_cpu_interrupt, args.kernel->threads.thread_atender_cpu_interrupt);
	kernel.sockets.server = iniciar_servidor(logger, kernel.puertoEscucha);
	hilos_io_inicializar(&args, args.kernel->threads.thread_atender_entrada_salida);
	hilos_planificador_inicializar(&args, args.kernel->threads.thread_planificador);
	hilos_consola_inicializar(&args, args.kernel->threads.thread_atender_consola);

	/*----LIBERO----*/

	printf("\n");
	log_destroy(logger);
	config_destroy(config);
	return 0;
}

void inicializar_args()
{
	args.logger = logger;
	args.kernel = &kernel;
	args.estados = &estados;
	args.kernel_orden_apagado = 1;
	args.kernel->sockets.entrada_salida_generic = 0;
	args.kernel->sockets.entrada_salida_stdin = 0;
	args.kernel->sockets.entrada_salida_stdout = 0;
	args.kernel->sockets.entrada_salida_dialfs = 0;
	args.kernel->sockets.entrada_salida = 0;
	args.kernel->sockets.id_entrada_salida = 1;
	args.recursos = recursos;
}

void inicializar_semaforos()
{
	sem_init(&kernel.planificador_iniciar, 0, 0);
	sem_init(&kernel.sistema_finalizar, 0, 4);
	sem_init(&kernel.log_lock, 0, 1);
}