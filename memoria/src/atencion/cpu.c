#include <utils/memoria.h>

void switch_case_cpu(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
		case CPU_MEMORIA_RESIZE:
		{
			/* TODO: Recibo un PID y un nuevo tamaño para la tabla de paginas del proceso (es decir, una cantidad de frames, que traducen a una cantidad de paginas), y debo verificar que el PID exista en Memoria, y luego si se debe hacer un resize positivo o negativo.
			
			Si es positivo hay que verificar si el espacio de marcos solicitados no es mayor a la cantidad de frames totales en Memoria.
			
			Si es negativo, se debe verificar que la cantidad de frames en uso no sea mayor a la cantidad de frames a liberar, en caso contrario, ¿no se puede liberar la cantidad de frames solicitados? ¿elimino datos? ¿cierro el proceso?, además, debo liberar y marcar como libres los frames que antes estaban ocupados por ese proceso.
			
			Si todo es correcto, se debe enviar un mensaje a CPU con el resultado de la operación, y si no, se debe enviar un mensaje de error "Out of memory" o "No se puede liberar la cantidad de frames solicitados", segun sea el caso. */

			t_cpu_memoria_resize *proceso_recibido = deserializar_t_cpu_memoria_resize(buffer);

			log_debug(argumentos->logger, "Se recibio un pedido de resize de tabla de paginas para el proceso con PID <%d>", proceso_recibido->pid);

			t_proceso *proceso = buscar_proceso(argumentos, proceso_recibido->pid);

			// Si el proceso no existe, envio un mensaje a CPU
			if (proceso == NULL)
			{
				log_error(argumentos->logger, "No se encontro el proceso con PID <%d> para efectuar el resize.", proceso_recibido->pid);

				// Aviso a CPU que no se encontro el proceso
				t_paquete *paquete = crear_paquete(MEMORIA_CPU_RESIZE);
				t_memoria_cpu_resize *proceso_enviar = malloc(sizeof(t_memoria_cpu_resize));
				proceso_enviar->pid = proceso_recibido->pid;
				proceso_enviar->frames = 0;
				proceso_enviar->resultado = 0;
				proceso_enviar->motivo = strdup("No se encontro el proceso con PID solicitado");
				proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

				serializar_t_memoria_cpu_resize(&paquete, proceso_enviar);
				enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);

				free(proceso_enviar->motivo);
				free(proceso_enviar);
				eliminar_paquete(paquete);
				break;
			}

			if (proceso_recibido->frames > list_size(proceso->tabla_paginas))
			{
				log_debug(argumentos->logger, "Se solicito un resize positivo de %d frames", proceso_recibido->frames);

				// Reviso si hay suficientes frames disponibles en sistema para el resize solicitado
				if (proceso_recibido->frames > (argumentos->memoria.tamMemoria / argumentos->memoria.tamPagina))
				{
					log_error(argumentos->logger, "No hay suficientes frames disponibles en sistema para el resize solicitado");

					t_paquete *paquete = crear_paquete(MEMORIA_CPU_RESIZE);
					t_memoria_cpu_resize *proceso_enviar = malloc(sizeof(t_memoria_cpu_resize));
					proceso_enviar->pid = proceso_recibido->pid;
					proceso_enviar->frames = 0;
					proceso_enviar->resultado = 1;
					proceso_enviar->motivo = strdup("Out of memory");
					proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;
					
					serializar_t_memoria_cpu_resize(&paquete, proceso_enviar);
					enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);

					free(proceso_enviar->motivo);
					free(proceso_enviar);
					eliminar_paquete(paquete);
					break;
				}

				// Realizo el resize positivo
				
				// TODO: Aca va la logica para hacer el resize positivo

				log_info(argumentos->logger, "Se realizo un resize positivo de %d frames para el proceso con PID <%d>", proceso_recibido->frames, proceso_recibido->pid);
			}
			else
			{
				log_debug(argumentos->logger, "Se solicito un resize negativo de %d frames", proceso_recibido->frames);

				int cantidad_frames_en_uso = 0;

				for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
				{
					t_pagina *pagina = list_get(proceso->tabla_paginas, i);

					if (pagina->en_uso)
					{
						cantidad_frames_en_uso++;
					}
				}

				// Si la cantidad de frames en uso es mayor a la cantidad de frames a liberar, no se pueden liberar los frames solicitados
				if (cantidad_frames_en_uso > proceso_recibido->frames)
				{
					log_error(argumentos->logger, "No se pueden liberar %d frames para el proceso con PID <%d>", proceso_recibido->frames, proceso_recibido->pid);

					t_paquete *paquete = crear_paquete(MEMORIA_CPU_RESIZE);
					t_memoria_cpu_resize *proceso_enviar = malloc(sizeof(t_memoria_cpu_resize));
					proceso_enviar->pid = proceso_recibido->pid;
					proceso_enviar->frames = 0;
					proceso_enviar->resultado = 2;
					proceso_enviar->motivo = strdup("No se pueden liberar la cantidad de frames solicitados porque el proceso los esta utilizando");
					proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

					serializar_t_memoria_cpu_resize(&paquete, proceso_enviar);
					enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);

					free(proceso_enviar->motivo);
					free(proceso_enviar);
					eliminar_paquete(paquete);
					break;
				}

				// Realizo el resize negativo

				// TODO: Aca va la logica para hacer el resize negativo
			}
			
			break;
		}
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
		case CPU_MEMORIA_TAM_PAGINA:
			{
				t_paquete *paquete = crear_paquete(CPU_MEMORIA_TAM_PAGINA);
				uint32_t sizepagina = argumentos->memoria.tamPagina;

				serializar_t_memoria_cpu_tam_pagina(&paquete, sizepagina);

				enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
				eliminar_paquete(paquete);
				break;
			}
		case CPU_MEMORIA_NUMERO_FRAME:
			{
				t_cpu_memoria_numero_frame *proceso_recibido = deserializar_t_cpu_memoria_numero_frame(buffer);

				t_proceso *proceso = NULL;
				proceso = buscar_proceso(argumentos, proceso->pid);

				if(memoria_acceder_tabla_paginas(argumentos, proceso->pid, proceso_recibido->numero_pagina) == -1)
				{
					log_error(argumentos->logger, "No se encontro la página para el proceso con PID <%d> y pagina <%d>", proceso->pid, proceso_recibido->numero_pagina);
					free(proceso);
					break;
				}

				uint32_t numero_marco = memoria_acceder_tabla_paginas(argumentos, proceso->pid, proceso_recibido->numero_pagina);

				t_paquete *paquete = crear_paquete(CPU_MEMORIA_NUMERO_FRAME);
				serializar_t_memoria_cpu_numero_marco(&paquete, numero_marco);

				enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
				free(proceso);
				eliminar_paquete(paquete);
			}
		default:
		{
			log_warning(argumentos->logger, "[CPU] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&argumentos->memoria.sockets.socket_cpu);
			break;
		}
	}
}