/* Módulo CPU */

#include "main.h"

int main() {
    
    logger = iniciar_logger("cpu");
    config = iniciar_config(logger);
    cpu = cpu_inicializar(config);
    cpu_log(cpu, logger);

	// Abrimos los sockets de conexion

	socket_memoria_dispatch = crear_conexion(logger,cpu.ipMemoria,cpu.puertoMemoria);
	log_info(logger,"Conectado a Memoria por Dispatch en socket %d",socket_memoria_dispatch);
	
	socket_memoria_interrupt = crear_conexion(logger,cpu.ipMemoria,cpu.puertoMemoria);
	log_info(logger,"Conectado a Memoria por Interrupt en socket %d",socket_memoria_interrupt);

	// Iniciamos los servidores de CPU (Dispatch y Interrupt)

    socket_server_dispatch = iniciar_servidor(logger,cpu.puertoEscuchaDispatch);
	log_info(logger, "Servidor Dispatch listo para recibir al cliente en Dispatch.");

	socket_server_interrupt = iniciar_servidor(logger,cpu.puertoEscuchaInterrupt);
	log_info(logger, "Servidor Interrupt listo para recibir al cliente en Interrupt.");

	// Atendemos las conexiones entrantes a CPU desde Kernel

	pthread_create(&dispatch,NULL,atender_kernel_dispatch,NULL);
	pthread_join(dispatch,NULL);

	pthread_create(&interrupt,NULL,atender_kernel_interrupt,NULL);
	pthread_join(interrupt,NULL);

	/*
	Aca va el resto de lo que falta hacer en CPU despues, en este punto ya tenemos las conexiones bidireccionales con Memoria y Kernel abiertas en sus sockets.
	*/

	// Libero

    log_destroy(logger);
    config_destroy(config);

	liberar_conexion(socket_memoria_dispatch);
	liberar_conexion(socket_memoria_interrupt);
	liberar_conexion(socket_kernel_dispatch);
	liberar_conexion(socket_kernel_interrupt);
	liberar_conexion(socket_server_dispatch);
	liberar_conexion(socket_server_interrupt);

    return 0;
}

void* atender_kernel_dispatch(){
	socket_kernel_dispatch = esperar_cliente(logger,socket_server_dispatch);
	log_info(logger,"Kernel conectado por Dispatch en socket %d",socket_kernel_dispatch);
	pthread_exit(0);
}

void* atender_kernel_interrupt(){
	socket_kernel_interrupt = esperar_cliente(logger,socket_server_interrupt);
	log_info(logger,"Kernel conectado por Interrupt en socket %d",socket_kernel_interrupt);
	pthread_exit(0);
}

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(t_log* logger, int socket_cliente)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

int esperar_cliente(t_log* logger, int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int iniciar_servidor(t_log* logger, int puerto)
{

	char puerto_str[6];

    // Convierto el puerto a string, para poder usarlo en getaddrinfo
    snprintf(puerto_str, sizeof(puerto_str), "%d", puerto);

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto_str, &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	int socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	// Asociamos el socket a un puerto

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

t_log* iniciar_logger(char* nombreDelModulo){
	
    t_log* nuevo_logger;

	nuevo_logger = log_create("module.log", nombreDelModulo, true, LOG_LEVEL_DEBUG);

	if (nuevo_logger == NULL) {
		log_error(nuevo_logger, "No se pudo crear el logger.");
		perror("No se puedo crear el logger.");
	}

	return nuevo_logger;
}

t_config* iniciar_config(t_log* logger){
	t_config* nuevo_config;

	char *current_dir = getcwd(NULL, 0);

	char ruta_completa[PATH_MAX]; 
    sprintf(ruta_completa, "%s/module.config", current_dir);

	nuevo_config = config_create(ruta_completa);

    if(nuevo_config == NULL){
		log_error(logger,"No se pudo crear la config.");
		perror("No se pudo crear la config.");
	}

	free(current_dir);
	return nuevo_config;
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	if(logger != NULL) {
		log_destroy(logger);
	}

	if(config != NULL) {
		config_destroy(config);
	}

	liberar_conexion(conexion);
}

int crear_conexion(t_log* logger,char *ip, int puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;
    char puerto_str[6];

    // Convierto el puerto a string, para poder usarlo en getaddrinfo
    snprintf(puerto_str, sizeof(puerto_str), "%d", puerto);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto_str, &hints, &server_info);

    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
        log_error(logger,"No se pudo conectar el socket cliente al servidor puerto %d!\n",puerto);
    } else {
		log_info(logger,"Conectado al servidor puerto %d!\n",puerto);
	}
	

    freeaddrinfo(server_info);

    return socket_cliente;
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	if(send(socket_cliente, a_enviar, bytes, 0)==-1){
		perror("No se pudo enviar el mensaje");
	}
	printf("Mensaje enviado!");

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}