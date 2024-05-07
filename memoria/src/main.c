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
	
	config_destroy(config);
	log_destroy(logger);

	return 0;
}

void *esperar_cpu()
{
	socket_cpu = esperar_conexion(logger, socket_server_memoria);
	
	if (socket_cpu == -1)
	{
		log_debug(logger, "Error al esperar conexion de CPU");
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_cpu, MEMORIA_CPU, "CPU");

	if (resultado != CORRECTO)
	{
		liberar_conexion(&socket_cpu);
		pthread_exit(0);
	}
	
	log_debug(logger, "CPU conectado en socket %d", socket_cpu);
	pthread_exit(0);
}

void *atender_cpu()
{
	hilo_ejecutar(logger, socket_cpu, "CPU", switch_case_cpu);
	pthread_exit(0);
}

void *esperar_kernel()
{
	socket_kernel = esperar_conexion(logger, socket_server_memoria);
	
	if (socket_kernel == -1)
	{
		log_error(logger, "Error al esperar conexion de Kernel");
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_kernel, MEMORIA_KERNEL, "Kernel");

	if (resultado != CORRECTO)
	{
		liberar_conexion(&socket_kernel);
		pthread_exit(0);
	}

	log_debug(logger, "Kernel conectado en socket %d", socket_kernel);
	pthread_exit(0);
}

void *atender_kernel()
{
	hilo_ejecutar(logger, socket_kernel, "Kernel", switch_case_kernel);
	pthread_exit(0);
}

void *esperar_entrada_salida()
{
	while (1)
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
            liberar_conexion(&socket_cliente);
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
            liberar_conexion(&socket_cliente);
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

	hilo_ejecutar(logger, socket_entrada_salida_stdin, modulo, switch_case_entrada_salida_stdin);
	
	free(args);
	pthread_exit(0);
}

void *atender_entrada_salida_stdout(void *args)
{
	char *modulo = "I/O STDOUT";

	socket_entrada_salida_stdout = *(int *)args;
	log_debug(logger, "%s conectado en socket %d", modulo, socket_entrada_salida_stdout);

	hilo_ejecutar(logger, socket_entrada_salida_stdout, modulo, switch_case_entrada_salida_stdout);
	
	free(args);
	pthread_exit(0);
}

void *atender_entrada_salida_dialfs(void *args)
{
	char *modulo = "I/O DialFS";

	socket_entrada_salida_dialfs = *(int *)args;
	log_debug(logger, "%s conectado en socket %d", modulo, socket_entrada_salida_dialfs);

	hilo_ejecutar(logger, socket_entrada_salida_dialfs, modulo, switch_case_entrada_salida_dialfs);
	
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

t_list *leer_instrucciones(char *path_instrucciones)
{	
	if(path_instrucciones == NULL){
		log_error(logger, "Path de instrucciones nulo. No se puede leer el archivo");
		return NULL;
	}

	// Abro el archivo de instrucciones
	FILE *file = fopen(path_instrucciones, "r");

	if (file == NULL)
	{	
		log_error(logger, "No se pudo abrir el archivo de instrucciones <%s>", path_instrucciones);
		return NULL;
	}

	// Inicializo la lista de instrucciones
	t_list *lista = list_create();

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

		t_memoria_cpu_instruccion *instruccion = malloc(sizeof(t_memoria_cpu_instruccion));
		
		if (instruccion == NULL)
		{
			log_error(logger, "No se pudo reservar memoria para la instruccion");
			fclose(file);
			free(line);
		}

		// Separo la línea en palabras
		char **separar_linea = string_split(line, " ");

		if(separar_linea[0] == NULL){
			log_error(logger, "Instruccion vacia. Cierro el archivo y libero memoria");
			fclose(file);
			free(line);
			free(instruccion);
			return NULL;
		}

		// Cuento la cantidad de elementos en separar_linea
    	uint32_t cantidad_elementos = 0;

    	while (separar_linea[cantidad_elementos] != NULL) {
        	cantidad_elementos++;
    	}

		// Asigno memoria para el array de punteros en instruccion->array
		instruccion->array = malloc(cantidad_elementos * sizeof(char*));
		if (instruccion->array == NULL) {
			log_error(logger, "No se pudo reservar memoria para instruccion->array");
			fclose(file);
			free(line);
			free(instruccion);
			return NULL;
		}
		instruccion->cantidad_elementos = cantidad_elementos;

		for (int i = 0; i < cantidad_elementos; i++) {
			instruccion->array[i] = strdup(separar_linea[i]);
			log_debug(logger, "Posicion %d: %s",i, instruccion->array[i]);
		}

		// Agrego la instruccion a la lista
		list_add(lista, instruccion);

		// Libero la memoria de la línea separada
		for(int i = 0; i < cantidad_elementos; i++){
			free(separar_linea[i]);
		}
		free(separar_linea);
	}
	// Cierro el archivo
	fclose(file);

	free(line);

	log_debug(logger, "Tamaño de la lista de instrucciones: %d", list_size(lista));

	return lista;
}

t_proceso *obtener_proceso(uint32_t pid)
{	
	char* pid_char = string_itoa((int)pid);
	char* indice = dictionary_get(diccionario_procesos, pid_char);
	
	if (indice == NULL) {
		log_error(logger, "No se encontro el proceso con PID <%s>", pid_char);
		return NULL;
	}

	free(pid_char);

	return list_get(lista_procesos, atoi(indice));
}

void eliminar_procesos(t_list *lista_procesos) {

	for (int i = 0; i < list_size(lista_procesos); i++){
		t_proceso *proceso = list_get(lista_procesos, i);
		if(!list_is_empty(proceso->instrucciones)){
			log_debug(logger, "Eliminando proceso con PID <%d> de Memoria.", proceso->pid);
			eliminar_instrucciones(proceso->instrucciones);
			free(proceso);
		}
	}
	list_destroy(lista_procesos);

	if(!dictionary_is_empty(diccionario_procesos)){
		dictionary_clean_and_destroy_elements(diccionario_procesos, free);
	}
}

void switch_case_kernel(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer){
	
	switch (codigo_operacion)
		{
			case KERNEL_MEMORIA_NUEVO_PROCESO:
			{
				t_kernel_memoria_proceso *dato = deserializar_t_kernel_memoria(buffer);
				
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
				proceso->instrucciones = leer_instrucciones(path_completo);
				
				// Le aviso a Kernel que no pude leer las instrucciones para ese PID
				if (proceso->instrucciones == NULL){
			
					t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);
					
					t_memoria_kernel_proceso* respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
					respuesta_proceso->pid = dato->pid;
					respuesta_proceso->cantidad_instrucciones = 0;
					respuesta_proceso->leido = false;
					
					serializar_t_memoria_kernel_proceso(&respuesta_paquete, respuesta_proceso);

					enviar_paquete(respuesta_paquete, socket_kernel);

					log_error(logger,"No se pudieron leer las instrucciones.");
					
					free(dato->path_instrucciones);
					free(dato);
					free(proceso);
					free(path_completo);
					free(respuesta_proceso);

					eliminar_paquete(respuesta_paquete);
					break;
				}

				log_debug(logger, "Se leyeron las instrucciones correctamente");

				// Guardo el proceso en la lista de procesos
				int index = list_add(lista_procesos, proceso);
				
				log_debug(logger, "Se agrego el proceso a la lista de procesos global");

				char* index_char = string_itoa(index);
				char* pid_char = string_itoa((int)proceso->pid);

				// Añado el proceso al diccionario de procesos, mapeando el PID a el indice en la lista de procesos
				dictionary_put(diccionario_procesos, pid_char, index_char);

				// Le aviso a Kernel que ya lei las instrucciones para ese PID
				t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);

				t_memoria_kernel_proceso* respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
				respuesta_proceso->pid = dato->pid;
				respuesta_proceso->cantidad_instrucciones = list_size(proceso->instrucciones);
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
				t_kernel_memoria_finalizar_proceso *proceso = deserializar_t_kernel_memoria_finalizar_proceso(buffer);

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

				// Elimino el proceso del diccionario de procesos
				dictionary_remove_and_destroy(diccionario_procesos, pid_char, free);

				log_info(logger, "Se elimino el proceso con PID <%d> de Memoria.\n", pid);

				free(proceso_encontrado);
				proceso_encontrado = NULL;

				free(pid_char);
				free(proceso);
				break;
			}
			case FINALIZAR_SISTEMA:
			{
				pthread_cancel(thread_atender_cpu);
				pthread_cancel(thread_atender_entrada_salida);

				if(socket_entrada_salida_stdin != 0){
					pthread_cancel(thread_atender_entrada_salida_stdin);
					liberar_conexion(&socket_entrada_salida_stdin);
				}
				
				if(socket_entrada_salida_stdout != 0){
					pthread_cancel(thread_atender_entrada_salida_stdout);
					liberar_conexion(&socket_entrada_salida_stdout);
				}
				
				if(socket_entrada_salida_dialfs != 0){
					pthread_cancel(thread_atender_entrada_salida_dialfs);
					liberar_conexion(&socket_entrada_salida_dialfs);
				}
			
				pthread_cancel(thread_atender_kernel);
				
				liberar_conexion(&socket_cpu);
				liberar_conexion(&socket_server_memoria);
				liberar_conexion(&socket_kernel);

				break;
			}
			default:
			{
				log_warning(logger, "[Kernel] Se recibio un codigo de operacion desconocido. Cierro hilo");
				liberar_conexion(&socket_cpu);
				break;
			}
			
		}
}

void switch_case_cpu(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer){
	switch (codigo_operacion)
		{
		case CPU_MEMORIA_PROXIMA_INSTRUCCION:
			{
			
			// Simulo el retardo de acceso a memoria en milisegundos
			sleep(memoria.retardoRespuesta/1000);

			t_cpu_memoria_instruccion *instruccion_recibida = deserializar_t_cpu_memoria_instruccion(buffer);

			log_debug(logger, "Deserializado del stream:");
			log_debug(logger, "Instruccion recibida de CPU:");
			log_debug(logger, "PID: %d", instruccion_recibida->pid);
			log_debug(logger, "Program counter: %d", instruccion_recibida->program_counter);

			log_info(logger,"Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");

			t_proceso *proceso_encontrado = obtener_proceso(instruccion_recibida->pid);
			
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

			// Actualizo la instruccion solicitada en el PC del proceso para saber cual fue la ultima solicitada por CPU
			proceso_encontrado->pc = instruccion_recibida->program_counter;

			if(list_is_empty(proceso_encontrado->instrucciones)){
				log_warning(logger, "No se encontraron instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);
				free(instruccion_recibida);
				free(proceso_encontrado);
				break;
			}

			t_memoria_cpu_instruccion *instruccion_proxima = list_get(proceso_encontrado->instrucciones, instruccion_recibida->program_counter);
			
			for(int i=0;i<instruccion_proxima->cantidad_elementos;i++){
				log_debug(logger, "Instruccion en posicion %d: %s", i, instruccion_proxima->array[i]);
			}
			
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
			liberar_conexion(&socket_cpu);
			break;
		}
	}
}

void switch_case_entrada_salida_stdin(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
		{
		case PLACEHOLDER:
			{
			// placeholder
			break;
			}
		default:
			{
			log_warning(logger, "[STDIN] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&socket_entrada_salida_stdin);
			break;
			}
	}
}

void switch_case_entrada_salida_stdout(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
		{
		case PLACEHOLDER:
			{
			// placeholder
			break;
			}
		default:
			{
			log_warning(logger, "[STDOUT] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&socket_entrada_salida_stdin);
			break;
			}
	}
}

void switch_case_entrada_salida_dialfs(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
		{
		case PLACEHOLDER:
			{
			// placeholder
			break;
			}
		default:
			{
			log_warning(logger, "[DialFS] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&socket_entrada_salida_stdin);
			break;
			}
	}
}

void eliminar_instrucciones(t_list *lista_instrucciones) {

	if(!list_is_empty(lista_instrucciones)){
		for(int i = 0; i < list_size(lista_instrucciones); i++){
			t_memoria_cpu_instruccion *instruccion = list_get(lista_instrucciones, i);
			for(int j = 0; j < instruccion->cantidad_elementos; j++){
				log_debug(logger, "Liberando instruccion en posicion %d: %s", j, instruccion->array[j]);
				free(instruccion->array[j]);
			}
			free(instruccion->array);
			free(instruccion);
		}
		list_destroy(lista_instrucciones);
	}
}