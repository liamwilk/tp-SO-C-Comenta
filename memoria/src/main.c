/* Módulo Memoria */
#include "main.h"

int main()
{	
	args.logger = iniciar_logger("memoria", LOG_LEVEL_DEBUG);
	args.memoria.config = iniciar_config(args.logger);
	memoria_inicializar(args.memoria.config);
	memoria_log(args.memoria, args.logger);
	
	args.memoria.lista_procesos = list_create();
	args.memoria.diccionario_procesos = dictionary_create();

	args.memoria.diccionario_entrada_salida = dictionary_create();
	args.memoria.lista_entrada_salida = list_create();

	args.memoria.sockets.id_entrada_salida = 1;

	args.memoria.sockets.socket_server_memoria = iniciar_servidor(args.logger, args.memoria.puertoEscucha);
	
	pthread_create(&args.memoria.threads.thread_esperar_cpu, NULL, esperar_cpu, NULL);
	pthread_join(args.memoria.threads.thread_esperar_cpu, NULL);

	pthread_create(&args.memoria.threads.thread_atender_cpu, NULL, atender_cpu, NULL);
	pthread_detach(args.memoria.threads.thread_atender_cpu);

	pthread_create(&args.memoria.threads.thread_conectar_kernel, NULL, esperar_kernel, NULL);
	pthread_join(args.memoria.threads.thread_conectar_kernel, NULL);

	pthread_create(&args.memoria.threads.thread_atender_entrada_salida, NULL, esperar_entrada_salida, NULL);
	pthread_detach(args.memoria.threads.thread_atender_entrada_salida);

	pthread_create(&args.memoria.threads.thread_atender_kernel, NULL, atender_kernel, NULL);
	pthread_join(args.memoria.threads.thread_atender_kernel, NULL);

	eliminar_procesos(args.memoria.lista_procesos);
	
	config_destroy(args.memoria.config);
	log_destroy(args.logger);

	return 0;
}

void inicializar_sockets(){
	args.memoria.sockets.socket_cpu = 0;
	args.memoria.sockets.socket_kernel = 0;
	args.memoria.sockets.socket_server_memoria = 0;
}

void *esperar_cpu()
{
	conexion_recibir(args.logger, args.memoria.sockets.socket_server_memoria, &args.memoria.sockets.socket_cpu, "CPU", MEMORIA_CPU);
	pthread_exit(0);
}

void *atender_cpu()
{
	hilo_ejecutar(args.logger, args.memoria.sockets.socket_cpu, "CPU", switch_case_cpu);
	pthread_exit(0);
}

void *esperar_kernel()
{
	conexion_recibir(args.logger, args.memoria.sockets.socket_server_memoria, &args.memoria.sockets.socket_kernel, "Kernel", MEMORIA_KERNEL);
	pthread_exit(0);
}

void *atender_kernel()
{
	hilo_ejecutar(args.logger, args.memoria.sockets.socket_kernel, "Kernel", switch_case_kernel);
	pthread_exit(0);
}

//////////////////////////////////////////////////////////////////////////////////////////

void *esperar_entrada_salida(void *paquete)
{
	// t_args *io_args = (t_args *)paquete;

	while (1)
	{
		int socket_cliente = conexion_socket_recibir(args.memoria.sockets.socket_server_memoria);

		if (socket_cliente == -1)
		{
            log_error(args.logger, "Error al esperar conexion de modulo de Entrada/Salida");
			break;
		}

		t_handshake modulo = esperar_handshake_entrada_salida(args.logger, socket_cliente);

        if (modulo == ERROR)
        {
            log_error(args.logger, "Error al recibir handshake de modulo de Entrada/Salida");
            liberar_conexion(&socket_cliente);
            break;
        }

		log_debug(args.logger, "Conexion por handshake recibida y estrablecido con modulo de Entrada/Salida en socket %d", socket_cliente);

		pthread_t thread_atender_entrada_salida;
        t_args_hilo *hilo_args = malloc(sizeof(t_args_hilo));
        hilo_args->argumentos = &args;
        hilo_args->entrada_salida = NULL;
        char *interfaz = NULL;

		switch (modulo)
        {
        case MEMORIA_ENTRADA_SALIDA_STDIN:
			interfaz = agregar_entrada_salida(&args, STDIN, socket_cliente);
            hilo_args->entrada_salida = buscar_interfaz(&args, interfaz);
            pthread_create(&thread_atender_entrada_salida, NULL, atender_entrada_salida_stdin, hilo_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        case MEMORIA_ENTRADA_SALIDA_STDOUT:
            interfaz = agregar_entrada_salida(&args, STDOUT, socket_cliente);
            hilo_args->entrada_salida = buscar_interfaz(&args, interfaz);
            pthread_create(&thread_atender_entrada_salida, NULL, atender_entrada_salida_stdout, hilo_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        case MEMORIA_ENTRADA_SALIDA_DIALFS:
            interfaz = agregar_entrada_salida(&args, DIALFS, socket_cliente);
            hilo_args->entrada_salida = buscar_interfaz(&args, interfaz);
            pthread_create(&thread_atender_entrada_salida, NULL, atender_entrada_salida_dialfs, hilo_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        default:
            log_debug(args.logger,"Se conecto un modulo de entrada/salida desconocido. Cerrando...");
            liberar_conexion(&socket_cliente);
            break;
        }
	}

	pthread_exit(0);
}

void *atender_entrada_salida_stdin(void *argumentos)
{
    t_args_hilo *io_args = (t_args_hilo*)argumentos;

    char *modulo = "STDIN";
    char *interfaz = io_args->entrada_salida->interfaz;
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Conectado en socket %d", modulo, interfaz, orden, *socket);

    while (1)
    {
        log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, interfaz, orden, *socket);
        t_paquete *paquete = recibir_paquete(io_args->argumentos->logger, socket);

        if (paquete == NULL)
        {
            log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, interfaz, orden);
            break;
        }
        revisar_paquete(paquete, io_args->argumentos->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            // Placeholder
            break;
        }
        default:
        {
            log_warning(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, interfaz, orden);
            liberar_conexion(socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }

    log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, interfaz, orden);

    remover_interfaz(io_args->argumentos, interfaz);

    free(io_args);
    pthread_exit(0);
}

void *atender_entrada_salida_stdout(void *argumentos)
{
   t_args_hilo *io_args = (t_args_hilo*)argumentos;

    char *modulo = "STDOUT";
    char *interfaz = io_args->entrada_salida->interfaz;
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Conectado en socket %d", modulo, interfaz, orden, *socket);

    while (1)
    {
        log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, interfaz, orden, *socket);
        t_paquete *paquete = recibir_paquete(io_args->argumentos->logger, socket);

        if (paquete == NULL)
        {
            log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, interfaz, orden);
            break;
        }
        revisar_paquete(paquete, io_args->argumentos->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            // Placeholder
            break;
        }
        default:
        {
            log_warning(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, interfaz, orden);
            liberar_conexion(socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }

    log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, interfaz, orden);

    remover_interfaz(io_args->argumentos, interfaz);

    free(io_args);
    pthread_exit(0);
}

void *atender_entrada_salida_dialfs(void *argumentos)
{
   t_args_hilo *io_args = (t_args_hilo*)argumentos;

    char *modulo = "DIALFS";
    char *interfaz = io_args->entrada_salida->interfaz;
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Conectado en socket %d", modulo, interfaz, orden, *socket);

    while (1)
    {
        log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, interfaz, orden, *socket);
        t_paquete *paquete = recibir_paquete(io_args->argumentos->logger, socket);

        if (paquete == NULL)
        {
            log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, interfaz, orden);
            break;
        }
        revisar_paquete(paquete, io_args->argumentos->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            // Placeholder
            break;
        }
        default:
        {
            log_warning(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, interfaz, orden);
            liberar_conexion(socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }

    log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, interfaz, orden);

    remover_interfaz(io_args->argumentos, interfaz);

    free(io_args);
    pthread_exit(0);
}

char *agregar_entrada_salida(t_args *argumentos, t_tipo_entrada_salida type, int socket)
{
    t_entrada_salida *entrada_salida = NULL;
    switch (type)
    {
    case STDIN:
        entrada_salida = agregar_interfaz(argumentos, STDIN, socket);
        break;
    case STDOUT:
        entrada_salida = agregar_interfaz(argumentos, STDOUT, socket);
        break;
    case DIALFS:
        entrada_salida = agregar_interfaz(argumentos, DIALFS, socket);
        break;
    default:
        log_error(argumentos->logger, "Tipo de socket de entrada/salida no reconocido");
        break;
    }

    if (entrada_salida == NULL)
    {
        log_error(argumentos->logger, "Error al agregar el socket de entrada/salida");
        return NULL;
    }

    return entrada_salida->interfaz;
};

t_entrada_salida *agregar_interfaz(t_args *argumentos, t_tipo_entrada_salida tipo, int socket)
{
    // Asigno memoria para el socket de entrada/salida (no debo liberarla porque se guarda dentro de la lista la referencia)
    t_entrada_salida *entrada_salida = malloc(sizeof(t_entrada_salida));

    // Guardo el socket en el cual se conecto el modulo de entrada/salida
    entrada_salida->socket = socket;
    entrada_salida->tipo = tipo;
    entrada_salida->orden = argumentos->memoria.sockets.id_entrada_salida;

    // Calculo el tamaño que necesito para almacenar el identificador de la interfaz
    int size_necesario = snprintf(NULL, 0, "Int%d", argumentos->memoria.sockets.id_entrada_salida) + 1;

    // Reservo memoria para la interfaz (no debo liberarla porque se guarda dentro del diccionario)
    char *interfaz = malloc(size_necesario);

    // Imprimo sobre la variable interfaz el identificador de la interfaz
    sprintf(interfaz, "Int%d", argumentos->memoria.sockets.id_entrada_salida);

    // Duplico la cadena para guardarla en el TAD y poder identificar la IO (esto pide malloc y hay que liberarlo cuando se desconecta la IO)
    entrada_salida->interfaz = strdup(interfaz);

    int *index = malloc(sizeof(int));

    // Agrego el TAD a la lista de entrada/salida y guardo el indice en el que se encuentra
    *index = list_add(args.memoria.lista_entrada_salida, entrada_salida);

    // Guardo en el diccionario la key interfaz y el value indice para ubicarlo en la lista luego
    dictionary_put(args.memoria.diccionario_entrada_salida, interfaz, index);

    log_debug(argumentos->logger, "Se conecto un modulo de entrada/salida en el socket %d con la interfaz %s", socket, interfaz);

   	argumentos->memoria.sockets.id_entrada_salida++;

	return entrada_salida;
}

void remover_interfaz(t_args *argumentos, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(argumentos->memoria.diccionario_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede desconectar
    if (indice == NULL)
    {
        log_debug(argumentos->logger, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_entrada_salida *entrada_salida = list_get(argumentos->memoria.lista_entrada_salida, *indice);

    log_debug(argumentos->logger, "Se remueve interfaz de entrada/salida %s", interfaz);

    // Cierro el socket de la entrada/salida del lado de Kernel
    liberar_conexion(&entrada_salida->socket);

    // Elimino la interfaz del diccionario
    dictionary_remove(argumentos->memoria.diccionario_entrada_salida, interfaz);

    // Libero la memoria de la interfaz
    free(entrada_salida->interfaz);

    // Libero la memoria del TAD
    free(entrada_salida);

    // Libero el indice guardado en el diccionario
    free(indice);

    // No lo elimino de la lista porque no se puede hacer un list_remove sin reorganizar los indices. Lo dejo en la lista pero no se puede acceder a el porque está vacio. Al finalizar el programa, destruyo la estructura de la lista entera.
}

t_entrada_salida *buscar_interfaz(t_args *argumentos, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(argumentos->memoria.diccionario_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede buscar
    if (indice == NULL)
    {
        log_error(argumentos->logger, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return NULL;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_entrada_salida *entrada_salida = list_get(argumentos->memoria.lista_entrada_salida, *indice);

    if (entrada_salida == NULL)
    {
        log_error(argumentos->logger, "No se encontro la interfaz %s en la lista de entrada/salida", interfaz);
    }

    log_debug(argumentos->logger, "Se encontro el modulo de entrada/salida en el socket %d asociado a la interfaz %s", entrada_salida->socket, interfaz);

    return entrada_salida;
}

//////////////////////////////////////////////////////////////////////////////////////////

void eliminar_procesos(t_list *lista_procesos) {

	if(!list_is_empty(lista_procesos)){
		for (int i = 0; i < list_size(lista_procesos); i++){
			t_proceso *proceso = list_get(lista_procesos, i);
			if(!list_is_empty(proceso->instrucciones)){
				log_debug(args.logger, "Proceso con PID <%d> encontrado en la lista de procesos global.", proceso->pid);
				log_debug(args.logger,"Tamaño de la lista de instrucciones: %d", list_size(proceso->instrucciones));
				char* pid = string_itoa(proceso->pid);
				eliminar_instrucciones(proceso->instrucciones);
				log_info(args.logger, "Proceso con PID <%s> que fue cargado pero no finalizado, ahora fue eliminado de Memoria.", pid);
				free(pid);
				free(proceso);
			}
		}	
	}

	// Libero la memoria de la lista de procesos
	list_destroy(lista_procesos);

	if(!dictionary_is_empty(args.memoria.diccionario_procesos)){
		dictionary_clean_and_destroy_elements(args.memoria.diccionario_procesos, free);
	}

	// Libero la memoria del diccionario de procesos
	dictionary_destroy(args.memoria.diccionario_procesos);
}

void eliminar_instrucciones(t_list *lista_instrucciones) {

	if(!list_is_empty(lista_instrucciones)){
		for(int i = 0; i < list_size(lista_instrucciones); i++){
			t_memoria_cpu_instruccion *instruccion = list_get(lista_instrucciones, i);
			for(int j = 0; j < instruccion->cantidad_elementos; j++){
				if(instruccion->array[j] != NULL){
					log_debug(args.logger, "Liberando instruccion en posicion %d: %s", j, instruccion->array[j]);
					free(instruccion->array[j]);
				}
			}
			free(instruccion->array);
			free(instruccion);
		}
		list_destroy(lista_instrucciones);
	}
}

char *armar_ruta(char *ruta1, char *ruta2)
{
	char *ruta = malloc(strlen(ruta1) + strlen(ruta2) + 2);
	strcpy(ruta, ruta1);
	strcat(ruta, "/");
	strcat(ruta, ruta2);
	return ruta;
}

t_list *leer_instrucciones(char *path_instrucciones, uint32_t pid)
{	
	if(path_instrucciones == NULL){
		log_error(args.logger, "Path de instrucciones nulo. No se puede leer el archivo");
		return NULL;
	}

	// Abro el archivo de instrucciones
	FILE *file = fopen(path_instrucciones, "r");

	if (file == NULL)
	{	
		log_error(args.logger, "No se pudo abrir el archivo de instrucciones <%s>", path_instrucciones);
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

	log_debug(args.logger, "Leyendo archivo de instrucciones...");

	// Getline lee la linea entera, hasta el \n inclusive
	while ((read = getline(&line, &len, file)) != -1)
	{
		log_debug(args.logger, "Linea leida: %s", line);

		t_memoria_cpu_instruccion *instruccion = malloc(sizeof(t_memoria_cpu_instruccion));
		
		if (instruccion == NULL)
		{
			log_error(args.logger, "No se pudo reservar memoria para la instruccion");
			fclose(file);
			free(line);
		}

		// Separo la línea en palabras
		char **separar_linea = string_split(line, " ");

		if(separar_linea[0] == NULL){
			log_error(args.logger, "Instruccion vacia. Cierro el archivo y libero memoria");
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
			log_error(args.logger, "No se pudo reservar memoria para instruccion->array");
			fclose(file);
			free(line);
			free(instruccion);
			return NULL;
		}
		
		instruccion->cantidad_elementos = cantidad_elementos;
		instruccion->pid = pid;

		for (int i = 0; i < cantidad_elementos; i++) {
			instruccion->array[i] = strdup(separar_linea[i]);
			log_debug(args.logger, "Posicion %d: %s",i, instruccion->array[i]);
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

	log_debug(args.logger, "Tamaño de la lista de instrucciones: %d", list_size(lista));

	return lista;
}

t_proceso *buscar_proceso(uint32_t pid)
{	
	char* pid_char = string_itoa(pid);
	char* indice = dictionary_get(args.memoria.diccionario_procesos, pid_char);
	
	if (indice == NULL) {
		log_error(args.logger, "No se encontro el proceso con PID <%s>", pid_char);
		return NULL;
	}
	
	free(pid_char);

	return list_get(args.memoria.lista_procesos, atoi(indice));
}

//////////////////////////////////////////////////////////////////////////////////////////

void switch_case_kernel(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer){
	
	switch (codigo_operacion)
		{
			case KERNEL_MEMORIA_NUEVO_PROCESO:
			{
				t_kernel_memoria_proceso *dato = deserializar_t_kernel_memoria(buffer);
				
				log_debug(logger, "Tamaño del path_instrucciones: %d", dato->size_path);
				log_debug(logger, "Path de instrucciones: %s", dato->path_instrucciones);
				log_debug(logger, "Program counter: %d", dato->program_counter);
				log_debug(logger, "PID: %d", dato->pid);
				
				char *path_completo = armar_ruta(args.memoria.pathInstrucciones, dato->path_instrucciones);

				log_debug(logger, "Path completo de instrucciones: %s", path_completo);
				
				t_proceso *proceso = malloc(sizeof(t_proceso));
				proceso->pc = dato->program_counter;
				proceso->pid = dato->pid;

				// Leo las instrucciones del archivo y las guardo en la lista de instrucciones del proceso
				proceso->instrucciones = leer_instrucciones(path_completo,proceso->pid);
				
				// Le aviso a Kernel que no pude leer las instrucciones para ese PID
				if (proceso->instrucciones == NULL){
			
					t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);
					
					t_memoria_kernel_proceso* respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
					respuesta_proceso->pid = dato->pid;
					respuesta_proceso->cantidad_instrucciones = 0;
					respuesta_proceso->leido = false;
					
					serializar_t_memoria_kernel_proceso(&respuesta_paquete, respuesta_proceso);

					enviar_paquete(respuesta_paquete, args.memoria.sockets.socket_kernel);

					log_error(logger,"No se pudieron leer las instrucciones.");
					
					free(dato->path_instrucciones);
					free(dato);
					free(proceso);
					free(path_completo);
					free(respuesta_proceso);

					eliminar_paquete(respuesta_paquete);
					break;
				}

				log_info(logger, "Se leyeron las instrucciones correctamente");
				log_info(logger, "Process ID: %d",proceso->pid);
				log_info(logger, "Cantidad de instrucciones leidas: %d", list_size(proceso->instrucciones));
				
				for(int i=0, j=1; i<list_size(proceso->instrucciones); i++){
					t_memoria_cpu_instruccion *instruccion = list_get(proceso->instrucciones, i);
					log_debug(logger, "Instruccion %d en indice %d:", j,i);
					for(int k=0; k<instruccion->cantidad_elementos; k++){
						log_debug(logger, "Elemento %d: %s", k, instruccion->array[k]);
					}
					j++;
				}

				// Guardo el proceso en la lista de procesos
				int index = list_add(args.memoria.lista_procesos, proceso);
				
				log_info(logger, "Proceso <%d> agregado a la lista de procesos global en la posicion %d", proceso->pid,index);

				// Añado el proceso al diccionario de procesos, mapeando el PID a el indice en la lista de procesos
				dictionary_put(args.memoria.diccionario_procesos, string_itoa(proceso->pid), string_itoa(index));

				{ // Reviso que se haya guardado correctamente en el diccionario de procesos y en la lista de procesos
					char* indice = dictionary_get(args.memoria.diccionario_procesos, string_itoa(proceso->pid));
					t_proceso* proceso_encontrado = list_get(args.memoria.lista_procesos, atoi(indice));

					log_debug(logger,"Reviso que se haya guardado correctamente en el diccionario de procesos y en la lista de procesos");
					log_debug(logger, "Proceso encontrado:");
					log_debug(logger, "PID: %d", proceso_encontrado->pid);
					log_debug(logger, "PC: %d", proceso_encontrado->pc);
					log_debug(logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

					if(proceso->pid == proceso_encontrado->pid && proceso->pc == proceso_encontrado->pc && list_size(proceso->instrucciones) == list_size(proceso_encontrado->instrucciones)){
						log_info(logger, "Proceso con PID <%d> guardado correctamente en el diccionario de procesos y en la lista de procesos", proceso->pid);
					}
				}
				
				{ // Le aviso a Kernel que ya lei las instrucciones para ese PID
					t_paquete *respuesta_paquete = crear_paquete(MEMORIA_KERNEL_NUEVO_PROCESO);

					t_memoria_kernel_proceso* respuesta_proceso = malloc(sizeof(t_memoria_kernel_proceso));
					respuesta_proceso->pid = dato->pid;
					respuesta_proceso->cantidad_instrucciones = list_size(proceso->instrucciones);
					respuesta_proceso->leido = true;

					serializar_t_memoria_kernel_proceso(&respuesta_paquete, respuesta_proceso);
					enviar_paquete(respuesta_paquete, args.memoria.sockets.socket_kernel);

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

				log_debug(logger, "Instruccion recibida para eliminar:");
				log_debug(logger, "PID: %d", proceso->pid);
				
				log_debug(logger,"Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");

				t_proceso *proceso_encontrado = buscar_proceso(proceso->pid);

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
				if(list_is_empty(proceso_encontrado->instrucciones))
				{
					log_warning(logger, "No se encontraron instrucciones para el proceso con PID <%d>. Es un proceso invalido, lo elimino de la lista.", proceso_encontrado->pid);
					list_destroy(proceso_encontrado->instrucciones);
					free(proceso_encontrado->instrucciones);
					free(proceso_encontrado);
					free(proceso);
					break;
				}
				
				// Remuevo cada instruccion y luego la lista de instrucciones en si misma
				eliminar_instrucciones(proceso_encontrado->instrucciones);

				char* pid_char = string_itoa(proceso_encontrado->pid);
				
				// Remuevo el proceso del diccionario de procesos
				dictionary_remove_and_destroy(args.memoria.diccionario_procesos, pid_char, free);

				log_info(logger, "Se elimino el proceso con PID <%d> de Memoria.\n", pid);

				free(proceso_encontrado);
				free(pid_char);
				free(proceso);
				break;
			}
			case FINALIZAR_SISTEMA:
			{
				pthread_cancel(args.memoria.threads.thread_atender_cpu);
				pthread_cancel(args.memoria.threads.thread_atender_entrada_salida);
			
				pthread_cancel(args.memoria.threads.thread_atender_kernel);
				
				liberar_conexion(&args.memoria.sockets.socket_cpu);
				liberar_conexion(&args.memoria.sockets.socket_server_memoria);
				liberar_conexion(&args.memoria.sockets.socket_kernel);

				break;
			}
			default:
			{
				log_warning(logger, "[Kernel] Se recibio un codigo de operacion desconocido. Cierro hilo");
				liberar_conexion(&args.memoria.sockets.socket_cpu);
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
			sleep(args.memoria.retardoRespuesta/1000);

			t_cpu_memoria_instruccion *instruccion_recibida = deserializar_t_cpu_memoria_instruccion(buffer);

			log_debug(logger, "Instruccion recibida de CPU:");
			log_debug(logger, "PID: %d", instruccion_recibida->pid);
			log_debug(logger, "Program counter: %d", instruccion_recibida->program_counter);

			log_info(logger,"Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");

			t_proceso *proceso_encontrado = buscar_proceso(instruccion_recibida->pid);
			
			if (proceso_encontrado == NULL)
			{
				log_warning(logger, "No se encontro el proceso con PID <%d>", instruccion_recibida->pid);
				free(instruccion_recibida);
				break;
			}

			log_debug(logger, "Proceso encontrado:");
			log_debug(logger, "PID: %d", proceso_encontrado->pid);
			log_debug(logger, "Ultimo PC enviado: %d", proceso_encontrado->pc);
			log_debug(logger, "Proximo PC a enviar: %d", instruccion_recibida->program_counter);
			
			// Caso borde, no debería pasar nunca
			if(list_is_empty(proceso_encontrado->instrucciones)){
				log_warning(logger, "No se encontraron instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);
				free(instruccion_recibida);
				break;
			}

			log_debug(logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

			int pc_solicitado = instruccion_recibida->program_counter;

			// Esto no debería pasar nunca, porque las instrucciones tienen EXIT al final.
			// En caso que el archivo de instrucciones no tenga EXIT al final, se envía un mensaje a CPU
			if(pc_solicitado == list_size(proceso_encontrado->instrucciones)){
				
				log_warning(logger, "Se solicito una instruccion fuera de rango para el proceso con PID <%d>", proceso_encontrado->pid);
				
				log_debug(logger,"Le aviso a CPU que no hay mas instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);

				t_paquete *paquete_instruccion = crear_paquete(MEMORIA_CPU_PROXIMA_INSTRUCCION);
				t_memoria_cpu_instruccion *instruccion_nula = malloc(sizeof(t_memoria_cpu_instruccion));
				instruccion_nula->pid = proceso_encontrado->pid;
				instruccion_nula->cantidad_elementos = 1;
				instruccion_nula->array = string_split("EXIT", " ");

				serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_nula);
				enviar_paquete(paquete_instruccion, args.memoria.sockets.socket_cpu);

				free(instruccion_recibida);
				break;
			}

			// Actualizo la instruccion solicitada en el PC del proceso para saber cual fue la ultima solicitada por CPU
			proceso_encontrado->pc = instruccion_recibida->program_counter;

			t_memoria_cpu_instruccion *instruccion_proxima = list_get(proceso_encontrado->instrucciones, instruccion_recibida->program_counter);
			
			for(int i=0;i<instruccion_proxima->cantidad_elementos;i++){
				log_debug(logger, "Instruccion en posicion %d: %s", i, instruccion_proxima->array[i]);
			}
			
			t_paquete *paquete_instruccion = crear_paquete(MEMORIA_CPU_PROXIMA_INSTRUCCION);
			serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_proxima);
			enviar_paquete(paquete_instruccion, args.memoria.sockets.socket_cpu);

			log_info(logger, "Instruccion con PID %d enviada a CPU", instruccion_recibida->pid);

			free(instruccion_recibida);
			eliminar_paquete(paquete_instruccion);
			break;
			}
		default:
		{	
			log_warning(logger, "[CPU] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&args.memoria.sockets.socket_cpu);
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
			liberar_conexion(&args.memoria.sockets.socket_entrada_salida_stdin);
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
			liberar_conexion(&args.memoria.sockets.socket_entrada_salida_stdin);
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
			liberar_conexion(&args.memoria.sockets.socket_entrada_salida_stdin);
			break;
			}
	}
}

void memoria_inicializar()
{
    args.memoria.puertoEscucha = config_get_int_value(args.memoria.config, "PUERTO_ESCUCHA");
    args.memoria.tamMemoria = config_get_int_value(args.memoria.config, "TAM_MEMORIA");
    args.memoria.tamPagina = config_get_int_value(args.memoria.config, "TAM_PAGINA");
    args.memoria.pathInstrucciones = config_get_string_value(args.memoria.config, "PATH_INSTRUCCIONES");
    args.memoria.retardoRespuesta = config_get_int_value(args.memoria.config, "RETARDO_RESPUESTA");
}

void memoria_log()
{
    log_info(args.logger, "PUERTO_ESCUCHA: %d", args.memoria.puertoEscucha);
    log_info(args.logger, "TAM_MEMORIA: %d", args.memoria.tamMemoria);
    log_info(args.logger, "TAM_PAGINA: %d", args.memoria.tamPagina);
    log_info(args.logger, "PATH_INSTRUCCIONES: %s", args.memoria.pathInstrucciones);
    log_info(args.logger, "RETARDO_RESPUESTA: %d", args.memoria.retardoRespuesta);
    printf("\n");
}