#ifndef SERIAL_H_
#define SERIAL_H_
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/collections/list.h>


// al momento de agregar TADS  esto se irá expandiendo, para permitirnos hacer el casting de forma más imediata
typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;


// estructura universal para el intercambio de TADS. Dependiendo del op_code se realiza un distinto casting al buffer para luego convertirlo en otra cosa distinta
typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_paquete* crear_paquete(void);
t_buffer *buffer_create(uint32_t size);

void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void recibir_mensaje(int socket_cliente , char* buffer);
t_paquete* recibir_paquete(int socket_cliente);

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void buffer_add(t_buffer *buffer, void *data, uint32_t size);

void eliminar_paquete(t_paquete* paquete);
void buffer_destroy(t_buffer *buffer);

void buffer_read(t_buffer *buffer, void *data, uint32_t size);












#endif