/* Módulo Entrada-Salida */

#include "main.h"

int main()
{

	t_log *logger;
	t_config *config;

	logger = iniciar_logger("Entrada-Salida");
	config = iniciar_config(logger);

	// Se pasan los datos de config a local
	char *ipMemoria = config_get_string_value(config, "IP_MEMORIA");
	int puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
	char *ipKernel = config_get_string_value(config, "IP_KERNEL");
	int puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
	char *tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
	int tiempoUnidadDeTrabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
	char *pathBaseDialFs = config_get_string_value(config, "PATH_BASE_DIALFS");
	int blockSize = config_get_int_value(config, "BLOCK_SIZE");
	int blockCount = config_get_int_value(config, "BLOCK_COUNT");

	// Se loggean los valores de config
	log_info(logger, "IP_MEMORIA: %s", ipMemoria);
	log_info(logger, "PUERTO_MEMORIA: %d", puertoMemoria);
	log_info(logger, "IP_KERNEL: %s", ipKernel);
	log_info(logger, "PUERTO_KERNEL: %d", puertoKernel);
	log_info(logger, "TIPO_INTERFAZ: %s", tipoInterfaz);
	log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", tiempoUnidadDeTrabajo);
	log_info(logger, "PATH_BASE_DIALFS: %s", pathBaseDialFs);
	log_info(logger, "BLOCK_SIZE: %d", blockSize);
	log_info(logger, "BLOCK_COUNT: %d", blockCount);

	// Threads para los Handshakes
	pthread_create(&memoriatThread, NULL, handshake_memoria, NULL);
	pthread_detach(memoriatThread);

	pthread_create(&kernelThread, NULL, handshake_kernel, NULL);
	pthread_detach(kernelThread);

	pthread_exit(0); // para que finalice


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

void *handshake_memoria()
{
	char *modulo = "Memoria";
	socketMemoria = crear_conexion(logger, ipMemoria, puertoMemoria, modulo);
	handshake(logger, socketMemoria, 1, modulo);
	enviar_mensaje("Conectado con Memoria", socketMemoria);
	return NULL;
}

void *handshake_kernel()
{
	char *modulo = "Kernel";
	socketKernel = crear_conexion(logger, ipKernel, puertoKernel, modulo);
	handshake(logger, socketKernel, 1, modulo);
	enviar_mensaje("Conectado con Kernel", socketKernel);
	return NULL;
}

uint32_t handshake(t_log *logger, int conexion, uint32_t envio, char *modulo)
{
	uint32_t result;

	send(conexion, &envio, sizeof(uint32_t), 0);
	recv(conexion, &result, sizeof(uint32_t), MSG_WAITALL);

	if (result == 0)
	{
		log_info(logger, "[%s] Conexion establecida.", modulo);
	}
	else
	{
		log_error(logger, "[%s] Error en la conexión.", modulo);
		return -1;
	}
	return result;
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