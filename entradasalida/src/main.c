/* Módulo Entrada-Salida */

#include "main.h"

int main()
{

	t_log *logger;
	t_config *config;

	logger = iniciar_logger("kernel");
	config = iniciar_config(logger);

	char *ipMemoria = config_get_string_value(config, "IP_MEMORIA");
	int puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
	char *ipKernel = config_get_string_value(config, "IP_KERNEL");
	int puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
	char *tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
	int tiempoUnidadDeTrabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
	char *pathBaseDialFs = config_get_string_value(config, "PATH_BASE_DIALFS");
	int blockSize = config_get_int_value(config, "BLOCK_SIZE");
	int blockCount = config_get_int_value(config, "BLOCK_COUNT");

	log_info(logger, "IP_MEMORIA: %s", ipMemoria);
	log_info(logger, "PUERTO_MEMORIA: %d", puertoMemoria);
	log_info(logger, "IP_KERNEL: %s", ipKernel);
	log_info(logger, "PUERTO_KERNEL: %d", puertoKernel);
	log_info(logger, "TIPO_INTERFAZ: %s", tipoInterfaz);
	log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", tiempoUnidadDeTrabajo);
	log_info(logger, "PATH_BASE_DIALFS: %s", pathBaseDialFs);
	log_info(logger, "BLOCK_SIZE: %d", blockSize);
	log_info(logger, "BLOCK_COUNT: %d", blockCount);

	int socketMemoria = crear_conexion(logger, ipMemoria, puertoMemoria);
	enviar_mensaje("Hola, soy I/O", socketMemoria);
	liberar_conexion(socketMemoria);

	int socketKernel = crear_conexion(logger, ipKernel, puertoKernel);
	enviar_mensaje("Hola, soy I/O!", socketKernel);
	liberar_conexion(socketKernel);

	log_destroy(logger);

	free(ipMemoria);
	free(ipKernel);
	free(tipoInterfaz);
	free(pathBaseDialFs);

	return 0;
}

t_paquete *crear_paquete(void)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void comunicarConCliente(t_paqueteCliente *cliente)
{

	cliente->socket = crear_conexion(cliente->logger, cliente->ip, cliente->puerto);
	enviar_mensaje("Hola, soy Kernel", cliente->socket);
}

int esperar_cliente(t_log *logger, int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int iniciar_servidor(t_log *logger, int puerto)
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

t_log *iniciar_logger(char *nombreDelModulo)
{

	t_log *nuevo_logger;

	nuevo_logger = log_create("module.log", nombreDelModulo, true, LOG_LEVEL_DEBUG);

	if (nuevo_logger == NULL)
	{
		log_error(nuevo_logger, "No se pudo crear el logger.");
		perror("No se puedo crear el logger.");
	}

	return nuevo_logger;
}

t_config *iniciar_config(t_log *logger)
{
	t_config *nuevo_config;

	char *current_dir = getcwd(NULL, 0);

	char ruta_completa[PATH_MAX];
	sprintf(ruta_completa, "%s/module.config", current_dir);
	printf("%s", ruta_completa);

	nuevo_config = config_create(ruta_completa);

	if (nuevo_config == NULL)
	{
		log_error(logger, "No se pudo crear la config.");
		perror("No se pudo crear la config.");
	}

	free(current_dir);
	return nuevo_config;
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
	if (logger != NULL)
	{
		log_destroy(logger);
	}

	if (config != NULL)
	{
		config_destroy(config);
	}

	liberar_conexion(conexion);
}

int crear_conexion(t_log *logger, char *ip, int puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;
	char puerto_str[6];

	// Convierto el puerto a string, para poder usarlo en getaddrinfo
	snprintf(puerto_str, sizeof(puerto_str), "%d", puerto);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto_str, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		log_error(logger, "No se pudo conectar el socket cliente al servidor puerto %d!", puerto);
		perror("No se pudo conectar el socket cliente al servidor!");
		exit(1);
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void enviar_mensaje(char *mensaje, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}