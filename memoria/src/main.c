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

	eliminar_procesos(lista_procesos);
	dictionary_clean_and_destroy_elements(diccionario_procesos, free);

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
	while(1)
	{
		pthread_testcancel();

		log_debug(logger, "Esperando paquete de CPU en socket %d", socket_cpu);
		t_paquete *paquete = recibir_paquete(logger, socket_cpu);

		if(paquete == NULL)
		{	
			log_warning(logger, "CPU se desconecto del socket %d.", socket_cpu);
			break;
		}
		
		switch (paquete->codigo_operacion)
		{
		case CPU_MEMORIA_PROXIMA_INSTRUCCION:
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
				log_warning(logger, "No se encontro el proceso con PID <%d>", instruccion_recibida->pid);
				free(instruccion_recibida);
				break;
			}

			log_debug(logger, "Proceso encontrado:");
			log_debug(logger, "PID: %d", proceso_encontrado->pid);
			log_debug(logger, "PC: %d", proceso_encontrado->pc);
			log_debug(logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

			if(list_is_empty(proceso_encontrado->instrucciones)){
				log_warning(logger, "No se encontraron instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);
				free(instruccion_recibida);
				free(proceso_encontrado);
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

			t_paquete *paquete_instruccion = crear_paquete(MEMORIA_CPU_PROXIMA_INSTRUCCION);
			serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_proxima);
			enviar_paquete(paquete_instruccion, socket_cpu);

			log_info(logger, "Instruccion con PID %d enviada a CPU", instruccion_recibida->pid);

			free(instruccion_recibida);
			free(instruccion_proxima);
			eliminar_paquete(paquete_instruccion);
			break;
			}
		default:
		{	
			log_warning(logger, "[CPU] Se recibio un codigo de operacion desconocido. Cierro hilo");
			eliminar_paquete(paquete);
			liberar_conexion(socket_cpu);
			pthread_exit(0);
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
	while(1)
	{
		pthread_testcancel();

		log_debug(logger, "Esperando paquete de Kernel en socket %d", socket_kernel);

		t_paquete *paquete = recibir_paquete(logger, socket_kernel);
		
		if(paquete == NULL)
		{	
			log_warning(logger, "Memoria se desconecto del socket %d.", socket_kernel);
			break;
		}
		
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
					log_error(logger,"No se pudieron leer las instrucciones.");
					
					// Le aviso a Kernel que no pude leer las instrucciones para ese PID
					t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);
					t_memoria_kernel_proceso* respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
					respuesta_proceso->pid = dato->pid;
					respuesta_proceso->cantidad_instruccions = 0;
					respuesta_proceso->leido = false;
					
					free(dato->path_instrucciones);
					free(dato);
					free(proceso);
					free(path_completo);
					free(respuesta_proceso);
					eliminar_paquete(respuesta_paquete);
					break;
				}
				// Guardo el proceso en la lista de procesos
				int index = list_add(lista_procesos, proceso);
				
				log_debug(logger, "Se agrego el proceso a la lista de procesos global");

				// Añado el proceso al diccionario de procesos, mapeando el PID a el indice en la lista de procesos
				dictionary_put(diccionario_procesos, string_itoa(proceso->pid), string_itoa(index));

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
				
				log_info(logger,"Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");

				t_proceso *proceso_encontrado = obtener_proceso(proceso->pid);

				if (proceso_encontrado == NULL)
				{
					log_warning(logger, "No se pudo recuperar el proceso con PID <%d> porque no existe en Memoria.", proceso->pid);
					free(proceso);
					break;
				}
				
				log_debug(logger, "Proceso encontrado:");
				log_debug(logger, "PID: %d", proceso_encontrado->pid);
				log_debug(logger, "PC: %d", proceso_encontrado->pc);
				log_debug(logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

				// Lo guardo para el log del final
				uint32_t pid = proceso_encontrado->pid;

				// Caso borde, no debería pasar nunca
				if(list_is_empty(proceso_encontrado->instrucciones)){
					log_warning(logger, "No se encontraron instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);
					free(proceso);
					break;
				}

				// Remuevo cada instruccion y luego la lista de instrucciones en si misma
				eliminar_instrucciones(proceso_encontrado->instrucciones);

				char* pid_char = string_itoa(proceso_encontrado->pid);

				// Libero el t_proceso de la lista de procesos, ya habiendo liberado las instrucciones
				list_remove_and_destroy_element(lista_procesos, atoi(dictionary_get(diccionario_procesos, pid_char)), free);

				// Elimino el proceso del diccionario de procesos
				dictionary_remove_and_destroy(diccionario_procesos, pid_char, free);

				log_info(logger, "Se elimino el proceso con PID <%d> de Memoria.\n", pid);

				free(pid_char);
				free(proceso);
				break;
			}
			case TERMINAR:
			{
				pthread_cancel(thread_atender_cpu);
				pthread_cancel(thread_atender_entrada_salida);

				if(socket_entrada_salida_stdin != 0){
					pthread_cancel(thread_atender_entrada_salida_stdin);
					liberar_conexion(socket_entrada_salida_stdin);
				}
				
				if(socket_entrada_salida_stdout != 0){
					pthread_cancel(thread_atender_entrada_salida_stdout);
					liberar_conexion(socket_entrada_salida_stdout);
				}
				
				if(socket_entrada_salida_dialfs != 0){
					pthread_cancel(thread_atender_entrada_salida_dialfs);
					liberar_conexion(socket_entrada_salida_dialfs);
				}
			
				pthread_cancel(thread_atender_kernel);
				
				liberar_conexion(socket_cpu);
				liberar_conexion(socket_server_memoria);
				liberar_conexion(socket_kernel);

				break;
			}
			default:
			{
				log_warning(logger, "[Kernel] Se recibio un codigo de operacion desconocido. Cierro hilo");
				eliminar_paquete(paquete);
				liberar_conexion(socket_cpu);
				pthread_exit(0);
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
            log_error(logger, "Error al esperar conexion de modulo de Entrada/Salida");
			break;
		}

		t_handshake modulo = esperar_handshake_entrada_salida(logger, socket_cliente);

        if (modulo == ERROR)
        {
            log_error(logger, "Error al recibir handshake de modulo de Entrada/Salida");
            liberar_conexion(socket_cliente);
            break;
        }

		int *args = malloc(sizeof(int));
		*args = socket_cliente;

		switch (modulo)
        {
        case MEMORIA_ENTRADA_SALIDA_STDIN:
            log_debug(logger, "Se conecto un modulo de entrada/salida STDIN");
            pthread_create(&thread_atender_entrada_salida_stdin, NULL, atender_entrada_salida_stdin, args);
            pthread_detach(thread_atender_entrada_salida_stdin);
            break;
        case MEMORIA_ENTRADA_SALIDA_STDOUT:
            log_debug(logger, "Se conecto un modulo de entrada/salida STDOUT");
            pthread_create(&thread_atender_entrada_salida_stdout, NULL, atender_entrada_salida_stdout, args);
            pthread_detach(thread_atender_entrada_salida_stdout);
            break;
        case MEMORIA_ENTRADA_SALIDA_DIALFS:
            log_debug(logger, "Se conecto un modulo de entrada/salida DIALFS");
            pthread_create(&thread_atender_entrada_salida_dialfs, NULL, atender_entrada_salida_dialfs, args);
            pthread_detach(thread_atender_entrada_salida_dialfs);
            break;
        default:
            log_error(logger, "Se conecto un modulo de entrada/salida desconocido. Desconectando...");
            liberar_conexion(socket_cliente);
            free(args);
            break;
        }
	}

	pthread_exit(0);
}

void *atender_entrada_salida_stdin(void *args)
{
	char *modulo = "I/O STDIN";

	socket_entrada_salida_stdin = *(int *)args;
	log_debug(logger, "%s conectado en socket %d", modulo, socket_entrada_salida_stdin);

	while(1)
	{
		pthread_testcancel();

		log_debug(logger, "Esperando paquete de %s en socket %d",modulo,socket_entrada_salida_stdin);
		t_paquete *paquete = recibir_paquete(logger, socket_entrada_salida_stdin);
		
		if(paquete == NULL)
		{	
			log_warning(logger, "I/O STDIN se desconecto del socket %d.", socket_entrada_salida_stdin);
			break;
		}

		switch (paquete->codigo_operacion)
		{
		case PLACEHOLDER:
			revisar_paquete(paquete,logger,kernel_orden_apagado,"I/O");
			// placeholder
			break;
		default:
			{
				log_warning(logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo",modulo);
				liberar_conexion(socket_entrada_salida_stdin);
				eliminar_paquete(paquete);
				free(args);
				pthread_exit(0);
			}
		}
		eliminar_paquete(paquete);
	}
	
	free(args);
	pthread_exit(0);
}

void *atender_entrada_salida_stdout(void *args)
{
	char *modulo = "I/O STDOUT";

	socket_entrada_salida_stdout = *(int *)args;
	log_debug(logger, "%s conectado en socket %d", modulo, socket_entrada_salida_stdout);

	while(1)
	{	
		pthread_testcancel();
		log_debug(logger, "Esperando paquete de %s en socket %d",modulo,socket_entrada_salida_stdout);
		t_paquete *paquete = recibir_paquete(logger, socket_entrada_salida_stdout);
		
		
		if(paquete == NULL)
		{	
			log_warning(logger, "I/O STDOUT se desconecto del socket %d.", socket_entrada_salida_stdout);
			break;
		}

		switch (paquete->codigo_operacion)
		{
		case PLACEHOLDER:
			revisar_paquete(paquete,logger,kernel_orden_apagado,"I/O");
			// placeholder
			break;
		default:
			{
				log_warning(logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo",modulo);
				liberar_conexion(socket_entrada_salida_stdout);
				eliminar_paquete(paquete);
				free(args);
				pthread_exit(0);
			}
		}
		eliminar_paquete(paquete);
	}
	
	free(args);
	pthread_exit(0);
}

void *atender_entrada_salida_dialfs(void *args)
{
	char *modulo = "I/O DialFS";

	socket_entrada_salida_dialfs = *(int *)args;
	log_debug(logger, "%s conectado en socket %d", modulo, socket_entrada_salida_dialfs);

	while(1)
	{
		pthread_testcancel();

		log_debug(logger, "Esperando paquete de %s en socket %d",modulo,socket_entrada_salida_dialfs);
		t_paquete *paquete = recibir_paquete(logger, socket_entrada_salida_dialfs);
		
		
		if(paquete == NULL)
		{	
			log_warning(logger, "Memoria se desconecto del socket %d.", socket_entrada_salida_dialfs);
			break;
		}
		
		switch (paquete->codigo_operacion)
		{
		case PLACEHOLDER:
			revisar_paquete(paquete,logger,kernel_orden_apagado,"I/O");
			// placeholder
			break;
		default:
			{
				log_warning(logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo",modulo);
				liberar_conexion(socket_entrada_salida_dialfs);
				eliminar_paquete(paquete);
				free(args);
				pthread_exit(0);
			}
		}
		eliminar_paquete(paquete);
	}

	free(args);
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
	char* pid_char = string_itoa(pid);
	char* indice = dictionary_get(diccionario_procesos, pid_char);
	
	if (indice == NULL) {
		log_error(logger, "No se encontro el proceso con PID <%s>", pid_char);
		return NULL;
	}

	free(pid_char);

	return list_get(lista_procesos, atoi(indice));
}

void eliminar_procesos(t_list *lista_procesos) {
	while (!list_is_empty(lista_procesos)) {
		t_proceso *proceso = list_remove(lista_procesos, 0);
		eliminar_instrucciones(proceso->instrucciones);
		free(proceso);
	}
}

void eliminar_instrucciones(t_list *lista_instrucciones) {
		for (int i = 0; i < list_size(lista_instrucciones); i++) {
			t_memoria_cpu_instruccion *instruccion = list_remove(lista_instrucciones, 0);
			
			log_debug(logger, "Instruccion a liberar: %s", instruccion->instruccion);
			log_debug(logger, "Argumento 1 a liberar: %s", instruccion->argumento_1);
			log_debug(logger, "Argumento 2 a liberar: %s", instruccion->argumento_2);
			log_debug(logger, "Argumento 3 a liberar: %s", instruccion->argumento_3);
			log_debug(logger, "Argumento 4 a liberar: %s", instruccion->argumento_4);
			log_debug(logger, "Argumento 5 a liberar: %s", instruccion->argumento_5);
			
			free(instruccion->argumento_1);
			free(instruccion->argumento_2);
			free(instruccion->argumento_3);
			free(instruccion->argumento_4);
			free(instruccion->argumento_5);
			free(instruccion->instruccion);
			free(instruccion);
		}
		list_destroy(lista_instrucciones);
}