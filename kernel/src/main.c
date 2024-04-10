/* Módulo Kernel */

#include "main.h"

int main() {
    
    logger = iniciar_logger("kernel");
    config = iniciar_config(logger);


    Kernel kernel= inicializar_kernel(config);


    log_kernel(kernel);

	// Handshakes

	pthread_create(&memoriatThread, NULL, handshakeMemoria, NULL);
	pthread_detach(memoriatThread);

	pthread_create(&cpuDispatchThread, NULL, handshakeCpuDispatch, NULL);
	pthread_detach(cpuDispatchThread);

	pthread_create(&cpuInterruptThread, NULL, handshakeCpuInterrupt, NULL);
	pthread_detach(cpuInterruptThread);

 	
	// Levanto el servidor de memoria
    serverKernel = iniciar_servidor(logger,kernel.puertoEscucha);
	log_info(logger, "Servidor listo para recibir al cliente");

	/* TODO:
	Aca hay que implementar el controlador de consolas.
	*/

	// Espero que todos los threads finalicen
    pthread_exit(0);

	// Libero todas las conexiones
	liberar_conexion(socketMemoria);
	liberar_conexion(socketCpuDispatch);
	liberar_conexion(socketCpuInterrupt);

	// Libero los punteros
	free(ipMemoria);
	free(ipCpu);
	free(algoritmoPlanificador);
	free(recursos);
	free(instanciasRecursos);

	// Destruyo estructuras
	config_destroy(config);
	log_destroy(logger);
}


/* TODO:
Estas 3 de handshake hay que armarlas en una sola funcion polimorfica para no repetir codigo
*/

void* handshakeMemoria(){
	char* modulo = "Memoria";
	socketMemoria = crear_conexion(logger,ipMemoria,puertoMemoria,modulo);
	handshake(logger, socketMemoria, 1 , modulo);
	enviar_mensaje("Conectado con Kernel", socketMemoria);
	return NULL;
}

void* handshakeCpuDispatch(){
	char* modulo = "CPU Dispatch";
	socketCpuDispatch = crear_conexion(logger,ipCpu,puertoCpuDispatch,modulo);
	handshake(logger,socketCpuDispatch, 1, modulo);
	enviar_mensaje("Conectado con Kernel", socketCpuDispatch);
	return NULL;
}

void* handshakeCpuInterrupt(){
	char* modulo = "CPU Interrupt";
	socketCpuInterrupt = crear_conexion(logger,ipCpu,puertoCpuInterrupt,modulo);
	handshake(logger,socketCpuInterrupt, 1, modulo);
	enviar_mensaje("Conectado con Kernel", socketCpuInterrupt);
	return NULL;
}

uint32_t handshake(t_log* logger,int conexion, uint32_t envio, char *modulo){
	uint32_t result;

	send(conexion, &envio, sizeof(uint32_t), 0);
	recv(conexion, &result, sizeof(uint32_t), MSG_WAITALL);

	if(result == 0) {
		log_info(logger, "[%s] Conexion establecida.", modulo);
	} else {
		log_error(logger, "[%s] Error en la conexión.", modulo);
		return -1;
	}
	return result;
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
	log_info(logger, "Me llego el mensaje: %s", buffer);
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



/* TODO:
Hay que refactorearla para que acepte un array de sockets conexion e itere todos.
*/

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

int crear_conexion(t_log* logger, char *ip, int puerto, char* modulo){
	struct addrinfo hints;
	struct addrinfo *server_info;
	char puerto_str[6];

	snprintf(puerto_str, sizeof(puerto_str), "%d", puerto);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip,puerto_str,&hints,&server_info);

	int socketCliente = socket(server_info->ai_family,server_info->ai_socktype,server_info->ai_protocol);
	int resultado = connect(socketCliente,server_info->ai_addr,server_info->ai_addrlen);

	// Espero a que se conecte
	while(resultado == -1){
		log_info(logger, "[%s@%s:%d] No se pudo conectar. Reintentando...", modulo, ip, *puerto_str);
		sleep(1);
		resultado = connect(socketCliente,server_info->ai_addr,server_info->ai_addrlen);
	}

	log_info(logger, "[%s@%s:%d] Conectado!", modulo, ip, puerto);
	freeaddrinfo(server_info);

	return socketCliente;
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

	send(socket_cliente, a_enviar, bytes, 0);

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