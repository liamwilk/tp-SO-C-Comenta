/* Módulo Memoria */
#include "main.h"

int main() {
    
    logger = iniciar_logger("memoria" , LOG_LEVEL_DEBUG);
    config = iniciar_config(logger);
	memoria = memoria_inicializar(config);
	memoria_log(memoria, logger);

    socket_server_memoria = iniciar_servidor(logger,memoria.puertoEscucha);
	log_info(logger, "Servidor Memoria listo para recibir al cliente en socket %d",socket_server_memoria);

	pthread_create(&thread_conectar_cpu,NULL,conectar_cpu,NULL);
	pthread_join(thread_conectar_cpu,NULL);

	pthread_create(&thread_atender_cpu,NULL,atender_cpu,NULL);
	pthread_detach(thread_atender_cpu);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);

	pthread_create(&thread_atender_io,NULL,conectar_io,NULL);
	pthread_detach(thread_atender_io);

	pthread_create(&thread_atender_kernel,NULL,atender_kernel,NULL);
	pthread_join(thread_atender_kernel,NULL);
	
	log_warning(logger,"Kernel solicito el apagado del sistema operativo.");

	config_destroy(config);
	log_destroy(logger);
	
    return 0;
}

void* conectar_cpu(){
	socket_cpu = esperar_cliente(logger,socket_server_memoria);
	esperar_handshake(socket_cpu);
	log_info(logger,"CPU conectado en socket %d",socket_cpu);
	pthread_exit(0);
}

void* atender_cpu(){
	while(kernel_orden_apagado){
		log_info(logger,"Esperando paquete de CPU en socket %d",socket_cpu);
		t_paquete* paquete = recibir_paquete(logger,socket_cpu);

		switch(paquete->codigo_operacion){
			case DAME_PROXIMA_INSTRUCCION:
				
				// Creo el paquete
				t_paquete* paquete_instruccion = crear_paquete(RECIBIR_UNA_INSTRUCCION);
				
				// Desencolo la instruccion
				t_memoria_cpu_instruccion* instruccion = desencolar_instruccion(instrucciones_cpu);

				// typedef struct
				// {
				// 	uint32_t size_instruccion;	  // Tamaño de la instruccion
				// 	char *instruccion;			  // Instruccion
				// 	uint32_t cantidad_argumentos; // Cantidad de argumentos
				// 	uint32_t size_argumento_1;	  // Tamaño del argumento
				// 	char *argumento_1;			  // Size del argumento
				// 	uint32_t size_argumento_2;	  // Tamaño del argumento
				// 	char *argumento_2;			  // Size del argumento
				// 	uint32_t size_argumento_3;	  // Tamaño del argumento
				// 	char *argumento_3;			  // Size del argumento
				// 	uint32_t size_argumento_4;	  // Tamaño del argumento
				// 	char *argumento_4;			  // Size del argumento
				// 	uint32_t size_argumento_5;	  // Tamaño del argumento
				// 	char *argumento_5;			  // Size del argumento
				// } t_memoria_cpu_instruccion;

				actualizar_buffer(paquete_instruccion,sizeof(uint32_t)*7+instruccion->size_instruccion+instruccion->size_argumento_1+instruccion->size_argumento_2+instruccion->size_argumento_3+instruccion->size_argumento_4+instruccion->size_argumento_5);

				serializar_uint32_t(instruccion->size_instruccion, paquete_instruccion);

				serializar_char(instruccion->instruccion, paquete_instruccion);
				
				serializar_uint32_t(instruccion->cantidad_argumentos, paquete_instruccion);
				
				serializar_uint32_t(instruccion->size_argumento_1, paquete_instruccion);
				
				serializar_char(instruccion->argumento_1, paquete_instruccion);
				
				serializar_uint32_t(instruccion->size_argumento_2, paquete_instruccion);
				
				serializar_char(instruccion->argumento_2, paquete_instruccion);
				
				serializar_uint32_t(instruccion->size_argumento_3, paquete_instruccion);
				
				serializar_char(instruccion->argumento_3, paquete_instruccion);
				
				serializar_uint32_t(instruccion->size_argumento_4, paquete_instruccion);
				
				serializar_char(instruccion->argumento_4, paquete_instruccion);
				
				serializar_uint32_t(instruccion->size_argumento_5, paquete_instruccion);

				serializar_char(instruccion->argumento_5, paquete_instruccion);

				enviar_paquete(paquete_instruccion,socket_cpu);

				// Eliminar punteros char de la instruccion primero
				free(instruccion->instruccion);
				free(instruccion->argumento_1);
				free(instruccion->argumento_2);
				free(instruccion->argumento_3);
				free(instruccion->argumento_4);
				free(instruccion->argumento_5);
				// Luego elimino la instruccion
				eliminar_paquete(paquete_instruccion);

				break;
			default:
				liberar_conexion(socket_cpu);
				pthread_exit(0);
				break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	pthread_exit(0);
}

void* conectar_kernel(){
	socket_kernel = esperar_cliente(logger,socket_server_memoria);
	esperar_handshake(socket_kernel);
	log_info(logger,"Kernel conectado en socket %d",socket_kernel);
	pthread_exit(0);
}

void* atender_kernel(){
	while(kernel_orden_apagado){
		
		log_info(logger,"Esperando paquete de Kernel en socket %d",socket_kernel);
		t_paquete* paquete = recibir_paquete(logger,socket_kernel);

		switch(paquete->codigo_operacion){
			case RECIBIR_PATH_INSTRUCCIONES:
				t_kernel_memoria *dato = deserializar_t_kernel_memoria(paquete->buffer);
				
				log_warning(logger,"Recibido paquete de Kernel:");
				log_debug(logger,"Codigo de operacion: %d",paquete->codigo_operacion);
				log_debug(logger,"Size del buffer en paquete: %d",paquete->size_buffer);
				log_debug(logger,"Offset en buffer: %d",paquete->buffer->offset);
				log_debug(logger,"Size en buffer: %d",paquete->buffer->size);

				log_warning(logger,"Deserializado del stream:");
				log_debug(logger,"Tamaño del path_instrucciones: %d",dato->size_path);
				log_debug(logger,"Path de instrucciones: %s",dato->path_instrucciones);
				log_debug(logger,"Program counter: %d",dato->program_counter);
				

				char* path_completo = armar_ruta(memoria.pathInstrucciones, dato->path_instrucciones);
				log_debug(logger,"Path completo de instrucciones: %s",path_completo);

				int resultado = encolar_instrucciones(&instrucciones_cpu, path_completo);

				if (resultado==0){
					log_debug(logger,"Se leyeron las instrucciones correctamente");
				} else {
					log_error(logger,"No se pudieron leer las instrucciones");
				}
				
				free(dato->path_instrucciones);
				free(dato);

				break;
			case TERMINAR:
				kernel_orden_apagado=0;
				liberar_conexion(socket_kernel);
				liberar_conexion(socket_cpu);
				liberar_conexion(socket_server_memoria);
				break;
			default:
				log_info(logger, "Operacion desconocida");
				break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
	}

	pthread_exit(0);
}

void* conectar_io(){
	while(kernel_orden_apagado){
		int socket_cliente = esperar_cliente(logger, socket_server_memoria);
		
		if(socket_cliente == -1){
			break;
		}

		esperar_handshake(socket_cliente);
		pthread_t hilo;
		int* args = malloc(sizeof(int));
		*args = socket_cliente;
		pthread_create(&hilo, NULL, atender_io, args);
		pthread_detach(hilo);
	}

	pthread_exit(0);
}

void* atender_io(void* args){
	int socket_cliente = *(int *)args;
	log_info(logger, "I/O conectado en socket %d", socket_cliente);
	free(args);

	do{
		log_info(logger,"Esperando paquete de I/O en socket %d",socket_cliente);
		t_paquete* paquete = recibir_paquete(logger,socket_cliente);

		switch(paquete->codigo_operacion){
			case MENSAJE:
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
	}while(kernel_orden_apagado);

	pthread_exit(0);
}

char* armar_ruta(char* ruta1, char* ruta2){
	char* ruta = malloc(strlen(ruta1) + strlen(ruta2) + 2);
	strcpy(ruta, ruta1);
	strcat(ruta, "/");
	strcat(ruta, ruta2);
	return ruta;
}

int encolar_instrucciones(t_queue** cola,char* pathInstrucciones){
    
	// Inicializo la cola de instrucciones
	*cola = queue_create();

	// Abro el archivo de instrucciones
    FILE* file = fopen(pathInstrucciones, "r");
    if(file == NULL){
        log_error(logger, "No se pudo abrir el archivo de instrucciones en el path %s", pathInstrucciones);
        return 1;
    }

	// Inicializa a NULL para que getline asigne memoria automáticamente
	char *line = NULL;  
    
	// Inicializa a 0 para que getline asigne memoria automáticamente
	size_t len = 0;     
	
	// ssize_t es un tipo de dato que se usa para representar el tamaño de bloques de memoria
    ssize_t read;

	log_debug(logger,"Leyendo archivo de instrucciones...\n");

	// Getline lee la linea entera, hasta el \n inclusive
    while ((read = getline(&line, &len, file)) != -1) {
    
        log_debug(logger,"Linea leida: %s", line);

		// Creo la estructura de instruccion
		t_memoria_cpu_instruccion* instruccion = malloc(sizeof(t_memoria_cpu_instruccion));
		if (!instruccion) {
    	// Manejo de error, malloc falló
			fclose(file);
			free(line);
			return -1;
		}
		
		char* vacio = "vacio";

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
		while(separar_linea[arg_count] != NULL){
			
			if(arg_count == 1){
				free(instruccion->argumento_1);
				instruccion->argumento_1 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_1 = strlen(instruccion->argumento_1) + 1;
			}
			if(arg_count == 2){
				free(instruccion->argumento_2);
				instruccion->argumento_2 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_2 = strlen(instruccion->argumento_2) + 1;
			}
			if(arg_count == 3){
				free(instruccion->argumento_3);
				instruccion->argumento_3 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_3 = strlen(instruccion->argumento_3) + 1;
			}
			if(arg_count == 4){
				free(instruccion->argumento_4);
				instruccion->argumento_4 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_4 = strlen(instruccion->argumento_4) + 1;
			}
			if(arg_count == 5){
				free(instruccion->argumento_5);
				instruccion->argumento_5 = strdup(separar_linea[arg_count]);
				instruccion->size_argumento_5 = strlen(instruccion->argumento_5) + 1;
			}
			
			// Incremento el contador de argumentos
			arg_count++;
		}

		// Guardo la cantidad de argumentos
		instruccion->cantidad_argumentos = arg_count-1;

		log_debug(logger,"Se agrego la instruccion %s a la cola de instrucciones", instruccion->instruccion);
		log_debug(logger,"Cantidad de argumentos: %d", instruccion->cantidad_argumentos);
		
		// Muestro los argumentos
		uint32_t i = 0;
		while (i<instruccion->cantidad_argumentos){
			log_debug(logger,"Argumento %d: %s",i+1,separar_linea[i+1]);
			i++;
		}
		
		// Agrego la instruccion a la cola
		queue_push(*cola, instruccion);
		
		// Libero la memoria de la línea separada
		int j = 0;
		while (separar_linea[j] != NULL) {
			free(separar_linea[j]);
			j++;
		}
		free(separar_linea);
    }

	// Cierro el archivo
    fclose(file);

	// Libero la memoria de la linea
	free(line);

	log_debug(logger,"Tamaño de la cola de instrucciones: %d\n", queue_size(*cola));

    return 0;
}

t_memoria_cpu_instruccion* desencolar_instruccion(t_queue* cola){
	
	t_memoria_cpu_instruccion* instruccion = malloc(sizeof(t_memoria_cpu_instruccion));
	
	if(queue_size(cola) > 0){
		instruccion = queue_pop(cola);
	}else{
		log_error(logger,"No hay instrucciones en la cola");
	}

	return instruccion;
}

t_kernel_memoria* deserializar_t_kernel_memoria(t_buffer* buffer) {

    t_kernel_memoria* dato = malloc(sizeof(t_kernel_memoria));
    void* stream = buffer->stream;
    
	memcpy(&(dato->size_path), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    dato->path_instrucciones = malloc(dato->size_path);
	memcpy(dato->path_instrucciones, stream, dato->size_path);
    stream += dato->size_path * sizeof(char);
    
	memcpy(&(dato->program_counter), stream, sizeof(uint32_t));

    return dato;
}
