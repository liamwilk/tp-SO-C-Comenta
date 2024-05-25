#include <utils/memoria.h>

void switch_case_cpu(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case CPU_MEMORIA_PROXIMA_INSTRUCCION:
	{
		// Simulo el retardo de acceso a memoria en milisegundos
		sleep(argumentos->memoria.retardoRespuesta / 1000);

		t_cpu_memoria_instruccion *instruccion_recibida = deserializar_t_cpu_memoria_instruccion(buffer);

		log_debug(argumentos->logger, "Instruccion recibida de CPU:");
		log_debug(argumentos->logger, "PID: %d", instruccion_recibida->pid);
		log_debug(argumentos->logger, "Program counter: %d", instruccion_recibida->program_counter);

		log_info(argumentos->logger, "Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");

		t_proceso *proceso_encontrado = buscar_proceso(argumentos,instruccion_recibida->pid);

		if (proceso_encontrado == NULL)
		{
			log_warning(argumentos->logger, "No se encontro el proceso con PID <%d>", instruccion_recibida->pid);
			free(instruccion_recibida);
			break;
		}

		log_debug(argumentos->logger, "Proceso encontrado:");
		log_debug(argumentos->logger, "PID: %d", proceso_encontrado->pid);
		log_debug(argumentos->logger, "Ultimo PC enviado: %d", proceso_encontrado->pc);
		log_debug(argumentos->logger, "Proximo PC a enviar: %d", instruccion_recibida->program_counter);

		// Caso borde, no debería pasar nunca
		if (list_is_empty(proceso_encontrado->instrucciones))
		{
			log_warning(argumentos->logger, "No se encontraron instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);
			free(instruccion_recibida);
			break;
		}

		log_debug(argumentos->logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

		int pc_solicitado = instruccion_recibida->program_counter;

		// Esto no debería pasar nunca, porque las instrucciones tienen EXIT al final.
		// En caso que el archivo de instrucciones no tenga EXIT al final, se envía un mensaje a CPU
		if (pc_solicitado == list_size(proceso_encontrado->instrucciones))
		{

			log_warning(argumentos->logger, "Se solicito una instruccion fuera de rango para el proceso con PID <%d>", proceso_encontrado->pid);

			log_debug(argumentos->logger, "Le aviso a CPU que no hay mas instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);

			t_paquete *paquete_instruccion = crear_paquete(MEMORIA_CPU_PROXIMA_INSTRUCCION);
			t_memoria_cpu_instruccion *instruccion_nula = malloc(sizeof(t_memoria_cpu_instruccion));
			instruccion_nula->pid = proceso_encontrado->pid;
			instruccion_nula->cantidad_elementos = 1;
			instruccion_nula->array = string_split("EXIT", " ");

			serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_nula);
			enviar_paquete(paquete_instruccion, argumentos->memoria.sockets.socket_cpu);

			free(instruccion_recibida);
			break;
		}

		// Actualizo la instruccion solicitada en el PC del proceso para saber cual fue la ultima solicitada por CPU
		proceso_encontrado->pc = instruccion_recibida->program_counter;

		t_memoria_cpu_instruccion *instruccion_proxima = list_get(proceso_encontrado->instrucciones, instruccion_recibida->program_counter);

		for (int i = 0; i < instruccion_proxima->cantidad_elementos; i++)
		{
			log_debug(argumentos->logger, "Instruccion en posicion %d: %s", i, instruccion_proxima->array[i]);
		}

		t_paquete *paquete_instruccion = crear_paquete(MEMORIA_CPU_PROXIMA_INSTRUCCION);
		serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_proxima);
		enviar_paquete(paquete_instruccion, argumentos->memoria.sockets.socket_cpu);

		log_info(argumentos->logger, "Instruccion con PID %d enviada a CPU", instruccion_recibida->pid);

		free(instruccion_recibida);
		eliminar_paquete(paquete_instruccion);
		break;
	}
	default:
	{
		log_warning(argumentos->logger, "[CPU] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&argumentos->memoria.sockets.socket_cpu);
		break;
	}
	}
}