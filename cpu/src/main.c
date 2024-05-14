/* Módulo CPU */

#include "main.h"

int main()
{
	logger = iniciar_logger("cpu", LOG_LEVEL_DEBUG);
	config = iniciar_config(logger);
	cpu = cpu_inicializar(config);
	cpu_log(cpu, logger);

	cpu.socket_server_dispatch = iniciar_servidor(logger, cpu.puertoEscuchaDispatch);
	cpu.socket_server_interrupt = iniciar_servidor(logger, cpu.puertoEscuchaInterrupt);

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
	conexion_crear(logger, cpu.ipMemoria, cpu.puertoMemoria, &cpu.socket_memoria, "Memoria", MEMORIA_CPU);
	pthread_exit(0);
}

void *atender_memoria()
{
	hilo_ejecutar(logger, cpu.socket_memoria, "Memoria", switch_case_memoria);
	pthread_exit(0);
}

void *esperar_kernel_dispatch()
{
	conexion_recibir(logger, cpu.socket_server_dispatch, &cpu.socket_kernel_dispatch, "Kernel por Dispatch", CPU_DISPATCH_KERNEL);
	pthread_exit(0);
}

void *atender_kernel_dispatch()
{
	hilo_ejecutar(logger, cpu.socket_kernel_dispatch, "Kernel Dispatch", switch_case_kernel_dispatch);
	pthread_exit(0);
}

void *esperar_kernel_interrupt()
{
	conexion_recibir(logger, cpu.socket_server_interrupt, &cpu.socket_kernel_interrupt, "Kernel por Interrupt", CPU_INTERRUPT_KERNEL);
	pthread_exit(0);
}

void *atender_kernel_interrupt()
{
	hilo_ejecutar(logger, cpu.socket_kernel_interrupt, "Kernel Interrupt", switch_case_kernel_interrupt);
	pthread_exit(0);
}

void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_CPU_PROXIMA_INSTRUCCION:
	{
		// FETCH
		t_instruccion *tipo_instruccion = malloc(sizeof(t_instruccion));

		int debeTerminar = cpu_memoria_recibir_instruccion(buffer, logger, &instruccion, tipo_instruccion, &proceso);

		if (debeTerminar)
		{
			// TODO: este socket deberia ser el de dispatch
			cpu_kernel_avisar_finalizacion(proceso, cpu.socket_kernel_dispatch);
			break;
		}

		// EXECUTE:
		cpu_ejecutar_instruccion(cpu, &instruccion, *tipo_instruccion, &proceso, logger);

		// TODO: Esto podria necesitar un mutex
		if(flag_interrupt)
		{
			cpu_procesar_interrupt(logger, cpu, proceso);
			flag_interrupt = 0;
			break;
		}

		cpu_memoria_pedir_proxima_instruccion(&proceso, cpu.socket_memoria);
		log_debug(logger, "Instruccion pedida a Memoria");
		free(tipo_instruccion);
		break;
	}
	case PLACEHOLDER:
	{
		// PLACEHOLDER
	}
	default:
	{
		log_warning(logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&cpu.socket_memoria);
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
		cpu_memoria_pedir_proxima_instruccion(&proceso, cpu.socket_memoria);
		log_debug(logger, "Instruccion pedida a Memoria");
		break;
	}
	default:
	{
		log_warning(logger, "[Kernel Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&cpu.socket_kernel_dispatch);
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

		liberar_conexion(&cpu.socket_memoria);
		liberar_conexion(&cpu.socket_kernel_dispatch);
		liberar_conexion(&cpu.socket_kernel_interrupt);

		break;
	}
	case KERNEL_CPU_INTERRUPCION:
	{
		// TODO_ esto podria necesitar un mutex
		flag_interrupt = cpu_recibir_interrupcion(logger, buffer, proceso);
		log_warning(logger, "Valor del flag ahora en: %d", flag_interrupt);
		break;
	}

	default:
	{
		log_warning(logger, "[Kernel Interrupt] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&cpu.socket_kernel_interrupt);
		break;
	}
	}
}
