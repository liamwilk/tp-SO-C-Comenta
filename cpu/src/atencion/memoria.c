#include <utils/cpu.h>

void switch_case_memoria(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_CPU_RESIZE:
	{
		t_memoria_cpu_resize *proceso_recibido = deserializar_t_memoria_cpu_resize(buffer);
		
		log_debug(args->logger, "Se recibio respuesta de Memoria sobre el pedido de RESIZE para el proceso <%d>", proceso_recibido->pid);
		log_debug(args->logger, "Frames solicitados: %d", proceso_recibido->frames);
		log_debug(args->logger, "Resultado: %d", proceso_recibido->resultado);
		log_debug(args->logger, "Motivo: %s", proceso_recibido->motivo);

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

