#include <utils/memoria.h>

void switch_case_kernel(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
		case KERNEL_MEMORIA_NUEVO_PROCESO:
		{
			t_kernel_memoria_proceso *dato = deserializar_t_kernel_memoria(buffer);

			log_debug(argumentos->logger, "Tamaño del path_instrucciones: %d", dato->size_path);
			log_debug(argumentos->logger, "Path de instrucciones: %s", dato->path_instrucciones);
			log_debug(argumentos->logger, "Program counter: %d", dato->program_counter);
			log_debug(argumentos->logger, "PID: %d", dato->pid);

			char *path_completo = armar_ruta(argumentos->memoria.pathInstrucciones, dato->path_instrucciones);

			log_debug(argumentos->logger, "Path completo de instrucciones: %s", path_completo);

			t_proceso *proceso = malloc(sizeof(t_proceso));
			proceso->pc = dato->program_counter;
			proceso->pid = dato->pid;

			// Leo las instrucciones del archivo y las guardo en la lista de instrucciones del proceso
			proceso->instrucciones = leer_instrucciones(argumentos,path_completo, proceso->pid);

			// Le aviso a Kernel que no pude leer las instrucciones para ese PID
			if (proceso->instrucciones == NULL)
			{

				t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);

				t_memoria_kernel_proceso *respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
				respuesta_proceso->pid = dato->pid;
				respuesta_proceso->cantidad_instrucciones = 0;
				respuesta_proceso->leido = false;

				serializar_t_memoria_kernel_proceso(&respuesta_paquete, respuesta_proceso);

				enviar_paquete(respuesta_paquete, argumentos->memoria.sockets.socket_kernel);

				log_error(argumentos->logger, "No se pudieron leer las instrucciones.");

				free(dato->path_instrucciones);
				free(dato);
				free(proceso);
				free(path_completo);
				free(respuesta_proceso);

				eliminar_paquete(respuesta_paquete);
				break;
			}

			log_info(argumentos->logger, "Se leyeron las instrucciones correctamente");
			log_info(argumentos->logger, "Process ID: %d", proceso->pid);
			log_info(argumentos->logger, "Cantidad de instrucciones leidas: %d", list_size(proceso->instrucciones));

			for (int i = 0, j = 1; i < list_size(proceso->instrucciones); i++)
			{
				t_memoria_cpu_instruccion *instruccion = list_get(proceso->instrucciones, i);
				log_debug(argumentos->logger, "Instruccion %d en indice %d:", j, i);
				for (int k = 0; k < instruccion->cantidad_elementos; k++)
				{
					log_debug(argumentos->logger, "Elemento %d: %s", k, instruccion->array[k]);
				}
				j++;
			}

			// Guardo el proceso en la lista de procesos
			int index = list_add(argumentos->memoria.lista_procesos, proceso);

			log_info(argumentos->logger, "Proceso <%d> agregado a la lista de procesos global en la posicion %d", proceso->pid, index);

			// Añado el proceso al diccionario de procesos, mapeando el PID a el indice en la lista de procesos
			dictionary_put(argumentos->memoria.diccionario_procesos, string_itoa(proceso->pid), string_itoa(index));

			{ // Reviso que se haya guardado correctamente en el diccionario de procesos y en la lista de procesos
				char *indice = dictionary_get(argumentos->memoria.diccionario_procesos, string_itoa(proceso->pid));
				t_proceso *proceso_encontrado = list_get(argumentos->memoria.lista_procesos, atoi(indice));

				log_debug(argumentos->logger, "Reviso que se haya guardado correctamente en el diccionario de procesos y en la lista de procesos");
				log_debug(argumentos->logger, "Proceso encontrado:");
				log_debug(argumentos->logger, "PID: %d", proceso_encontrado->pid);
				log_debug(argumentos->logger, "PC: %d", proceso_encontrado->pc);
				log_debug(argumentos->logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

				if (proceso->pid == proceso_encontrado->pid && proceso->pc == proceso_encontrado->pc && list_size(proceso->instrucciones) == list_size(proceso_encontrado->instrucciones))
				{
					log_info(argumentos->logger, "Proceso con PID <%d> guardado correctamente en el diccionario de procesos y en la lista de procesos", proceso->pid);
				}
			}

			{ // Le aviso a Kernel que ya lei las instrucciones para ese PID
				t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);

				t_memoria_kernel_proceso *respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
				respuesta_proceso->pid = dato->pid;
				respuesta_proceso->cantidad_instrucciones = list_size(proceso->instrucciones);
				respuesta_proceso->leido = true;

				serializar_t_memoria_kernel_proceso(&respuesta_paquete, respuesta_proceso);
				enviar_paquete(respuesta_paquete, argumentos->memoria.sockets.socket_kernel);

				free(path_completo);
				free(dato->path_instrucciones);
				free(dato);
				free(respuesta_proceso);

				eliminar_paquete(respuesta_paquete);
			}

			break;
		}
		case KERNEL_MEMORIA_FINALIZAR_PROCESO:
		{
			t_kernel_memoria_finalizar_proceso *proceso = deserializar_t_kernel_memoria_finalizar_proceso(buffer);

			log_debug(argumentos->logger, "Instruccion recibida para eliminar:");
			log_debug(argumentos->logger, "PID: %d", proceso->pid);

			log_debug(argumentos->logger, "Se comienza a buscar el proceso solicitado por KERNEL en la lista de procesos globales.");

			t_proceso *proceso_encontrado = buscar_proceso(argumentos,proceso->pid);

			if (proceso_encontrado == NULL)
			{
				log_warning(argumentos->logger, "No se pudo recuperar el proceso con PID <%d> porque no existe en Memoria.", proceso->pid);
				free(proceso);
				break;
			}

			log_debug(argumentos->logger, "Proceso encontrado:");
			log_debug(argumentos->logger, "PID: %d", proceso_encontrado->pid);
			log_debug(argumentos->logger, "PC: %d", proceso_encontrado->pc);
			log_debug(argumentos->logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

			// Lo guardo para el log del final
			uint32_t pid = proceso_encontrado->pid;

			// Caso borde, no debería pasar nunca
			if (list_is_empty(proceso_encontrado->instrucciones))
			{
				log_warning(argumentos->logger, "No se encontraron instrucciones para el proceso con PID <%d>. Es un proceso invalido, lo elimino de la lista.", proceso_encontrado->pid);
				list_destroy(proceso_encontrado->instrucciones);
				free(proceso_encontrado->instrucciones);
				free(proceso_encontrado);
				free(proceso);
				break;
			}

			// Remuevo cada instruccion y luego la lista de instrucciones en si misma
			eliminar_instrucciones(argumentos,proceso_encontrado->instrucciones);

			char *pid_char = string_itoa(proceso_encontrado->pid);

			// Remuevo el proceso del diccionario de procesos
			dictionary_remove_and_destroy(argumentos->memoria.diccionario_procesos, pid_char, free);

			log_info(argumentos->logger, "Se elimino el proceso con PID <%d> de Memoria.", pid);

			free(proceso_encontrado);
			free(pid_char);
			free(proceso);
			break;
		}
		case FINALIZAR_SISTEMA:
		{
			pthread_cancel(argumentos->memoria.threads.thread_atender_cpu);
			pthread_cancel(argumentos->memoria.threads.thread_atender_entrada_salida);

			pthread_cancel(argumentos->memoria.threads.thread_atender_kernel);

			liberar_conexion(&argumentos->memoria.sockets.socket_cpu);
			liberar_conexion(&argumentos->memoria.sockets.socket_server_memoria);
			liberar_conexion(&argumentos->memoria.sockets.socket_kernel);

			break;
		}
		default:
		{
			log_warning(argumentos->logger, "[Kernel] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&argumentos->memoria.sockets.socket_cpu);
			break;
		}
	}
}