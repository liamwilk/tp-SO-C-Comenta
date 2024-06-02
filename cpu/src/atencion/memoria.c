#include <utils/cpu.h>

void switch_case_memoria(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_CPU_RESIZE:
	{
		t_memoria_cpu_resize *proceso_recibido = deserializar_t_memoria_cpu_resize(buffer);

		log_debug(args->logger, "Se recibio respuesta de Memoria sobre el pedido de RESIZE para el proceso <%d>", proceso_recibido->pid);

		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "No se pudo redimensionar el proceso <%d>", proceso_recibido->pid);
			log_debug(args->logger, "Motivo: <%s>", proceso_recibido->motivo);

			/* "En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación." */

			t_paquete *paquete = crear_paquete(CPU_KERNEL_RESIZE);

			t_cpu_kernel_resize *proceso_fallido = malloc(sizeof(t_cpu_kernel_resize));

			proceso_fallido->pid = proceso_recibido->pid;
			proceso_fallido->motivo = strdup(proceso_recibido->motivo);
			proceso_fallido->size_motivo = strlen(proceso_recibido->motivo) + 1;
			proceso_fallido->resultado = proceso_recibido->resultado;
			proceso_fallido->registros = args->registros;

			serializar_t_cpu_kernel_resize(&paquete, proceso_fallido);

			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);
			free(proceso_fallido);
		}
		else
		{
			log_debug(args->logger, "Se redimensiono el proceso con PID <%d> a <%d> bytes.", proceso_recibido->pid, proceso_recibido->bytes);
		}

		instruccion_solicitar(args);

		free(proceso_recibido->motivo);
		free(proceso_recibido);
		break;
	}
	case MEMORIA_CPU_PROXIMA_INSTRUCCION:
	{
		instruccion_ciclo(args, buffer);
		break;
	}
	case MEMORIA_CPU_TAM_PAGINA:
	{
		uint32_t *tamPag = deserializar_t_memoria_cpu_tam_pagina(buffer);
		args->tam_pagina = *tamPag;
		log_debug(args->logger, "Tamaño de pagina recibido de Memoria: %d", args->tam_pagina);
		free(tamPag);
		break;
	}
	default:
	{
		log_warning(args->logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&args->config_leida.socket_memoria);
		break;
	}
	}
}