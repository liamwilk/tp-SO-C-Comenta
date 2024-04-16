#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<pthread.h>
#include<limits.h>
#include <utils/modulos.h>
#include <utils/handshake.h>

t_kernel kernel;

t_log* logger;
t_config* config;

typedef enum
{
	MENSAJE,
	PAQUETE,

}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_log* iniciar_logger(char*);
t_config* iniciar_config(t_log* logger);
void terminar_programa(int, t_log*, t_config*);
int crear_conexion(t_log*,char*, int);
void* serializar_paquete(t_paquete*, int);
void enviar_mensaje(char*, int);
void crear_buffer(t_paquete*);
void agregar_a_paquete(t_paquete*, void*, int);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
void liberar_conexion(int);
int iniciar_servidor(t_log*, int);
int esperar_cliente(t_log*, int);
int recibir_operacion(int);
void recibir_mensaje(t_log*,int);
void *recibir_buffer(int*,int);


void* atender_io();

pthread_t io;

int socket_io;
int socket_server_kernel;
int socket_memoria;
int socket_cpu_interrupt;
int socket_cpu_dispatch;

#endif /* MAIN_H_ */