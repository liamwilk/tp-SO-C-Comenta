/* Módulo Memoria */
#include "main.h"

int main()
{
	logger = iniciar_logger("memoria", LOG_LEVEL_DEBUG);
	config = iniciar_config(logger);
	memoria = memoria_inicializar(config);
	memoria_log(memoria, logger);
	
	lista_procesos = list_create();
	diccionario_procesos = dictionary_create();

	log_debug(logger, "Estructuras de procesos globales inicializados");

	socket_server_memoria = iniciar_servidor(logger, memoria.puertoEscucha);
	log_debug(logger, "Servidor Memoria listo para recibir al cliente en socket %d", socket_server_memoria);

	pthread_create(&thread_esperar_cpu, NULL, esperar_cpu, NULL);
	pthread_join(thread_esperar_cpu, NULL);

	pthread_create(&thread_atender_cpu, NULL, atender_cpu, NULL);
	pthread_detach(thread_atender_cpu);

	pthread_create(&thread_conectar_kernel, NULL, esperar_kernel, NULL);
	pthread_join(thread_conectar_kernel, NULL);

	pthread_create(&thread_atender_entrada_salida, NULL, esperar_entrada_salida, NULL);
	pthread_detach(thread_atender_entrada_salida);

	pthread_create(&thread_atender_kernel, NULL, atender_kernel, NULL);
	pthread_join(thread_atender_kernel, NULL);

	log_info(logger, "Kernel solicito el apagado del sistema operativo.");

	if(!list_is_empty(lista_procesos))
	{	
		log_debug(logger, "Eliminando procesos almacenados en lista de procesos.");
		// TODO: Liberar memoria de las instrucciones, no solo de la lista, posible memory leak.
		list_destroy_and_destroy_elements(lista_procesos, free);
	}

	if(!dictionary_is_empty(diccionario_procesos))
	{
		log_debug(logger, "Eliminando diccionario de procesos.");
		dictionary_clean_and_destroy_elements(diccionario_procesos,free);
	}

	config_destroy(config);
	log_destroy(logger);

	return 0;
}

void *esperar_cpu()
{
	socket_cpu = esperar_conexion(logger, socket_server_memoria);
	
	if (socket_cpu == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_cpu, MEMORIA_CPU, "CPU");

	if (resultado != CORRECTO)
	{
		liberar_conexion(socket_cpu);
		pthread_exit(0);
	}
	
	log_debug(logger, "CPU conectado en socket %d", socket_cpu);
	pthread_exit(0);
}

void *atender_cpu()
{
	while (kernel_orden_apagado)
	{
		log_debug(logger, "Esperando paquete de CPU en socket %d", socket_cpu);
		t_paquete *paquete = recibir_paquete(logger, socket_cpu);

		switch (paquete->codigo_operacion)
		{
		case PROXIMA_INSTRUCCION:
			{
			revisar_paquete(paquete,logger,kernel_orden_apagado,"CPU");

			// Simulo el retardo de acceso a memoria en milisegundos
			sleep(memoria.retardoRespuesta/1000);

			t_cpu_memoria_instruccion *instruccion_recibida = deserializar_t_cpu_memoria_instruccion(paquete->buffer);

			log_debug(logger, "Deserializado del stream:");
			log_debug(logger, "Instruccion recibida de CPU:");
			log_debug(logger, "PID: %d", instruccion_recibida->pid);
			log_debug(logger, "Program counter: %d", instruccion_recibida->program_counter);

			t_proceso *proceso_encontrado = obtener_proceso(instruccion_recibida->pid);
			
			log_info(logger,"Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");

			if (proceso_encontrado == NULL)
			{
				log_error(logger, "No se encontro el proceso con PID %d", instruccion_recibida->pid);

				// TODO: Informar a CPU que ese proceso no existe en Memoria

				break;
			}

			log_debug(logger, "Proceso encontrado:");
			log_debug(logger, "PID: %d", proceso_encontrado->pid);
			log_debug(logger, "PC: %d", proceso_encontrado->pc);
			log_debug(logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

			if(list_is_empty(proceso_encontrado->instrucciones)){
				log_error(logger, "No se encontraron instrucciones para el proceso con PID %d", proceso_encontrado->pid);
				
				// TODO: Informar a CPU que ese proceso no tiene instrucciones en Memoria. Sería raro igual, no debería pasar jamas pero igualmente voy a controlar este error.

				break;
			}

			t_memoria_cpu_instruccion *instruccion_proxima = list_get(proceso_encontrado->instrucciones, instruccion_recibida->program_counter);

			log_debug(logger, "Instruccion proxima a enviar a CPU: %s", instruccion_proxima->instruccion);
			log_debug(logger, "Cantidad de argumentos: %d", instruccion_proxima->cantidad_argumentos);
			log_debug(logger, "Argumento 1: %s", instruccion_proxima->argumento_1);
			log_debug(logger, "Argumento 2: %s", instruccion_proxima->argumento_2);
			log_debug(logger, "Argumento 3: %s", instruccion_proxima->argumento_3);
			log_debug(logger, "Argumento 4: %s", instruccion_proxima->argumento_4);
			log_debug(logger, "Argumento 5: %s", instruccion_proxima->argumento_5);

			t_paquete *paquete_instruccion = crear_paquete(PROXIMA_INSTRUCCION);
			serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_proxima);
			enviar_paquete(paquete_instruccion, socket_cpu);

			log_info(logger, "Instruccion con PID %d enviada a CPU", instruccion_recibida->pid);

			free(instruccion_recibida);
			eliminar_paquete(paquete_instruccion);

			break;
			}
		default:
		{	
			eliminar_paquete(paquete);
			liberar_conexion(socket_cpu);
			pthread_exit(0);
			break;
		}
		}
		eliminar_paquete(paquete);
	}

	pthread_exit(0);
}

void *esperar_kernel()
{
	socket_kernel = esperar_conexion(logger, socket_server_memoria);
	
	if (socket_kernel == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_kernel, MEMORIA_KERNEL, "Kernel");

	if (resultado != CORRECTO)
	{
		liberar_conexion(socket_kernel);
		pthread_exit(0);
	}

	log_debug(logger, "Kernel conectado en socket %d", socket_kernel);
	pthread_exit(0);
}

void *atender_kernel()
{
	while (kernel_orden_apagado)
	{
		log_debug(logger, "Esperando paquete de Kernel en socket %d", socket_kernel);

		t_paquete *paquete = recibir_paquete(logger, socket_kernel);
		
		switch (paquete->codigo_operacion)
		{
			case KERNEL_MEMORIA_NUEVO_PROCESO:
				{
				revisar_paquete(paquete,logger,kernel_orden_apagado,"Kernel");
				t_kernel_memoria_proceso *dato = deserializar_t_kernel_memoria(paquete->buffer);
				
				log_debug(logger, "Deserializado del stream:");
				log_debug(logger, "Tamaño del path_instrucciones: %d", dato->size_path);
				log_debug(logger, "Path de instrucciones: %s", dato->path_instrucciones);
				log_debug(logger, "Program counter: %d", dato->program_counter);
				log_debug(logger, "PID: %d", dato->pid);
				
				char *path_completo = armar_ruta(memoria.pathInstrucciones, dato->path_instrucciones);

				log_debug(logger, "Path completo de instrucciones: %s", path_completo);
				
				t_proceso *proceso = malloc(sizeof(t_proceso));
				proceso->pc = dato->program_counter;
				proceso->pid = dato->pid;

				// Leo las instrucciones del archivo y las guardo en la lista de instrucciones del proceso
				proceso->instrucciones = memoria_leer_instrucciones(path_completo);

				if (proceso->instrucciones != NULL){
					log_debug(logger, "Se leyeron las instrucciones correctamente");
				}
				else{
					perror("No se pudieron leer las instrucciones");
					
					// Le aviso a Kernel que no pude leer las instrucciones para ese PID
					t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);
					t_memoria_kernel_proceso* respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
					respuesta_proceso->pid = dato->pid;
					respuesta_proceso->cantidad_instruccions = 0;
					respuesta_proceso->leido = false;
					
					free(respuesta_proceso);
					eliminar_paquete(respuesta_paquete);
					break;
				}
				// Guardo el proceso en la lista de procesos
				int resultado_agregar_proceso = list_add(lista_procesos, proceso);

				// Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
				dictionary_put(diccionario_procesos, string_itoa(proceso->pid), (void*)string_itoa(list_size(lista_procesos) - 1));

				if(resultado_agregar_proceso == 0)
					log_debug(logger, "Se agrego el proceso a la lista de procesos global");
				else
					perror("No se pudo agregar el proceso a la lista de procesos");
			

				// Le aviso a Kernel que ya lei las instrucciones para ese PID
				t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);

				t_memoria_kernel_proceso* respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
				respuesta_proceso->pid = dato->pid;
				respuesta_proceso->cantidad_instruccions = list_size(proceso->instrucciones);
				respuesta_proceso->leido = true;

				serializar_t_memoria_kernel_proceso(&respuesta_paquete, respuesta_proceso);
				enviar_paquete(respuesta_paquete, socket_kernel);

				free(path_completo);
				free(dato->path_instrucciones);
				free(dato);

				free(respuesta_proceso);
				eliminar_paquete(respuesta_paquete);
				break;
				}
			case KERNEL_MEMORIA_FINALIZAR_PROCESO:
			{
				revisar_paquete(paquete,logger,kernel_orden_apagado,"Kernel");
				t_kernel_memoria_finalizar_proceso *proceso = deserializar_t_kernel_memoria_finalizar_proceso(paquete->buffer);

				log_debug(logger, "Deserializado del stream:");
				log_debug(logger, "Instruccion recibida para eliminar:");
				log_debug(logger, "PID: %d", proceso->pid);
				
				t_proceso *proceso_encontrado = obtener_proceso(proceso->pid);

				if (proceso_encontrado == NULL)
				{
					// Esto es muy raro, no debería pasar jamas pero igualmente voy a controlar este error.
					// Si Kernel me manda a eliminar un proceso que no existe en Memoria, no hago nada, pero igual
					// esto no debería suceder porque Kernel siempre debería tener la lista de procesos actualizada.
					
					log_error(logger, "No se encontro el proceso con PID %d", proceso->pid);
					break;
				}

				log_info(logger,"Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");
				log_debug(logger, "Proceso encontrado:");
				log_debug(logger, "PID: %d", proceso_encontrado->pid);
				log_debug(logger, "PC: %d", proceso_encontrado->pc);
				log_debug(logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

				// Lo guardo para el log del final
				uint32_t pid = proceso_encontrado->pid;
				
				if(list_is_empty(proceso_encontrado->instrucciones)){
					// Este caso es muy raro, no debería pasar jamas pero igualmente voy a controlar este error.
					// TODO: Revisar este caso, no debería pasar jamas.
					log_error(logger, "No se encontraron instrucciones para el proceso con PID %d", proceso_encontrado->pid);
					break;
				}

				/* TODO: Eliminar cada instruccion del proceso, no solo la lista de instrucciones, posible memory leak.

				Adentro de cada proceso, hay una lista de instrucciones, cada instruccion es un struct t_memoria_cpu_instruccion
				y cada instruccion tiene argumentos que son char*, por lo que hay que liberar la memoria de cada argumento
				y luego liberar la memoria de cada instruccion, y luego liberar la memoria de la lista de instrucciones. 
				Esto ultimo viene nativo con list_destroy_and_destroy_elements(lista_procesos, free);

				*/

				// Esto libera la memoria de la lista de instrucciones
				list_remove_and_destroy_element(lista_procesos, atoi(dictionary_get(diccionario_procesos, string_itoa(proceso_encontrado->pid))), free);

				// Elimino el proceso del diccionario de procesos
				dictionary_remove_and_destroy(diccionario_procesos, string_itoa(proceso_encontrado->pid), free);

				// Elimino el proceso del diccionario de procesos
				// dictionary_remove(diccionario_procesos, string_itoa(proceso_encontrado->pid));

				log_info(logger, "Se elimino el proceso con PID <%d> de la lista de procesos globales", pid);

				// Libero la instruccion recibida
				free(proceso);
				break;
			}
			case TERMINAR:
			{
				kernel_orden_apagado = 0;
				liberar_conexion(socket_kernel);
				liberar_conexion(socket_cpu);
				liberar_conexion(socket_server_memoria);
				break;
			}
			default:
			{
				eliminar_paquete(paquete);
				liberar_conexion(socket_kernel);
				pthread_exit(0);
				break;
			}
			
		}
		eliminar_paquete(paquete);
	}
	pthread_exit(0);
}

void *esperar_entrada_salida()
{
	while (kernel_orden_apagado)
	{
		int socket_cliente = esperar_conexion(logger, socket_server_memoria);

		if (socket_cliente == -1)
		{
			break;
		}

		t_handshake resultado = esperar_handshake(logger, socket_cliente, MEMORIA_ENTRADA_SALIDA, "Entrada/Salida");

		if (resultado != CORRECTO)
		{
			liberar_conexion(socket_cliente);
			break;
		}

		pthread_t hilo;
		int *args = malloc(sizeof(int));
		*args = socket_cliente;
		pthread_create(&hilo, NULL, atender_entrada_salida, args);
		pthread_detach(hilo);
	}

	pthread_exit(0);
}

void *atender_entrada_salida(void *args)
{
	int socket_cliente = *(int *)args;
	log_debug(logger, "I/O conectado en socket %d", socket_cliente);
	free(args);

	do
	{
		log_debug(logger, "Esperando paquete de I/O en socket %d", socket_cliente);
		t_paquete *paquete = recibir_paquete(logger, socket_cliente);
		
		switch (paquete->codigo_operacion)
		{
		case MENSAJE:
			revisar_paquete(paquete,logger,kernel_orden_apagado,"I/O");
			// placeholder
			break;
		default:
			liberar_conexion(socket_cliente);
			pthread_exit(0);
			break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	} while (kernel_orden_apagado);

	pthread_exit(0);
}

char *armar_ruta(char *ruta1, char *ruta2)
{
	char *ruta = malloc(strlen(ruta1) + strlen(ruta2) + 2);
	strcpy(ruta, ruta1);
	strcat(ruta, "/");
	strcat(ruta, ruta2);
	return ruta;
}

t_list *memoria_leer_instrucciones(char *pathInstrucciones)
{
	// Inicializo la lista de instrucciones
	t_list *lista = list_create();

	// Abro el archivo de instrucciones
	FILE *file = fopen(pathInstrucciones, "r");
	if (file == NULL)
	{
		perror("No se pudo abrir el archivo de instrucciones en el path");
		return NULL;
	}

	// Inicializa a NULL para que getline asigne memoria automáticamente
	char *line = NULL;

	// Inicializa a 0 para que getline asigne memoria automáticamente
	size_t len = 0;

	// ssize_t es un tipo de dato que se usa para representar el tamaño de bloques de memoria
	ssize_t read;

	log_debug(logger, "Leyendo archivo de instrucciones...");

	// Getline lee la linea entera, hasta el \n inclusive
	while ((read = getline(&line, &len, file)) != -1)
	{

		log_debug(logger, "Linea leida: %s", line);

		// Creo la estructura de instruccion
		t_memoria_cpu_instruccion *instruccion = malloc(sizeof(t_memoria_cpu_instruccion));
		if (!instruccion)
		{
			// Manejo de error, malloc falló
			fclose(file);
			free(line);
			perror("Fallo al crear la instruccion");
		}

		char *vacio = "vacio";

		// Inicializo los tamaños de los argumentos
		instruccion->size_argumento_1 = strlen(vacio) + 1;
		instruccion->size_argumento_2 = strlen(vacio) + 1;
		instruccion->size_argumento_3 = strlen(vacio) + 1;
		instruccion->size_argumento_4 = strlen(vacio) + 1;
		instruccion->size_argumento_5 = strlen(vacio) + 1;

		// Inicializo los argumentos en "Vacio"
		instruccion->argumento_1 = strdup(vacio);
		instruccion->argumento_2 = strdup(vacio);
		instruccion->argumento_3 = strdup(vacio);
		instruccion->argumento_4 = strdup(vacio);
		instruccion->argumento_5 = strdup(vacio);

		// Separo la linea en palabras
		char **separar_linea = string_split(line, " ");

		// Guardo la instruccion
		instruccion->instruccion = strdup(separar_linea[0]);

		// Guardo el tamaño de la instruccion
		instruccion->size_instruccion = strlen(instruccion->instruccion) + 1;

		// Inicializo el contador de argumentos
		uint32_t arg_count = 1;

		// Guardo los argumentos
		while (separar_linea[arg_count] != NULL)
		{

			if (arg_count == 1)
			{
				free(instruccion->argumento_1);
				instruccion->argumento_1 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_1 = strlen(instruccion->argumento_1) + 1;
			}
			if (arg_count == 2)
			{
				free(instruccion->argumento_2);
				instruccion->argumento_2 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_2 = strlen(instruccion->argumento_2) + 1;
			}
			if (arg_count == 3)
			{
				free(instruccion->argumento_3);
				instruccion->argumento_3 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_3 = strlen(instruccion->argumento_3) + 1;
			}
			if (arg_count == 4)
			{
				free(instruccion->argumento_4);
				instruccion->argumento_4 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_4 = strlen(instruccion->argumento_4) + 1;
			}
			if (arg_count == 5)
			{
				free(instruccion->argumento_5);
				instruccion->argumento_5 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_5 = strlen(instruccion->argumento_5) + 1;
			}

			// Incremento el contador de argumentos
			arg_count++;
		}

		// Guardo la cantidad de argumentos
		instruccion->cantidad_argumentos = arg_count - 1;

		log_debug(logger, "Se agrego la instruccion %s a la lista de instrucciones", instruccion->instruccion);
		log_debug(logger, "Cantidad de argumentos: %d", instruccion->cantidad_argumentos);

		// Muestro los argumentos
		uint32_t i = 0;
		while (i < instruccion->cantidad_argumentos)
		{
			log_debug(logger, "Argumento %d: %s", i + 1, separar_linea[i + 1]);
			i++;
		}

		// Agrego la instruccion a la lista
		list_add(lista, instruccion);

		// Libero la memoria de la línea separada
		int j = 0;
		while (separar_linea[j] != NULL)
		{
			free(separar_linea[j]);
			j++;
		}
		free(separar_linea);
	}

	// Cierro el archivo
	fclose(file);

	// Libero la memoria de la linea
	free(line);

	log_debug(logger, "Tamaño de la lista de instrucciones: %d", list_size(lista));

	return lista;
}

t_proceso *obtener_proceso(uint32_t pid)
{
	t_proceso *proceso = malloc(sizeof(t_proceso));

	int index;
	char* index_char = (char*)dictionary_get(diccionario_procesos, string_itoa(pid));
	
	if (index_char != NULL) {
		index = atoi(index_char); 
	} else {
		log_error(logger, "No se encontro el proceso con PID %d", pid);
		return NULL;
	}
	
	proceso = list_get(lista_procesos, index);

	return proceso;
}