#ifndef SERIAL_H_
#define SERIAL_H_
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>

/*--------Serializacion y paquetes--------*/

typedef enum
{
	MENSAJE,
	PAQUETE,
	DESCONECTAR
} op_code;

typedef struct
{
	int size;
	void *stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	uint64_t size;
	t_buffer *buffer;
} t_paquete;

t_paquete *crear_paquete(void);

/**
 * @fn    serializar_paquete
 * @brief Implementacion de la serializacion de un paquete (strings)
 * @param paquete Paquete con buffer y su op_code
 * @param bytes
 */
void *serializar_paquete(t_paquete *paquete, int bytes);

/**
 * @fn    enviar_mensaje
 * @brief Envia un mensaje `mensaje` al modulo conectado
 * @param mensaje Paquete con buffer y su op_code
 * @param socket_cliente
 */
void enviar_mensaje(char *mensaje, int socket_cliente);

/**
 *
 * @fn    crear_buffer
 * @brief Crea un buffer y lo agrega al paquete
 */
void crear_buffer(t_paquete *paquete);

/**
 *
 * @fn    agregar_a_paquete
 * @brief Agrega al paquete un `valor` de tipo generico
 * @param paquete Paquete de datos
 * @param valor Agrega el tipo generico
 * @param tamanio
 */
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);

/**
 *
 * @fn    enviar_paquete
 * @brief Dado un socket envia el paquete al modulo destino
 * @param paquete Paquete de datos
 * @param socket_cliente
 */
void enviar_paquete(t_paquete *paquete, int socket_cliente);

/**
 *
 * @fn    eliminar_paquete
 * @brief Elimina el paquete para no generar memory leak
 * @param paquete Paquete de datos
 */
void eliminar_paquete(t_paquete *paquete);

/**
 *
 * @fn    recibir_buffer
 * @brief Elimina el paquete para no generar memory leak
 * @param paquete Paquete de datos
 */
void *recibir_buffer(int *size, int socket_cliente);

/**
 *
 * @fn    recibir_mensaje
 * @brief Invoca la funcion recv y recibe los datos desde el socket
 * @param socket_cliente
 */
void recibir_mensaje(t_log *logger, int socket_cliente);

/**
 *
 * @fn    recibir_operacion
 * @brief Invoca la funcion recv y recibe el codigo de operacion desde el socket
 * @param socket_cliente
 */
int recibir_operacion(int socket_cliente);

/*--------BUFFERS--------*/

/**
 *
 * @fn    buffer_create
 * @brief Crea un buffer vacío de tamaño size y offset 0
 * @param size
 */
t_buffer *buffer_create(uint32_t size);

/**
 *
 * @fn    buffer_add
 * @brief Agrega un stream al buffer en la posición actual y avanza el offset
 * @param buffer
 * @param data
 * @param size
 */
void buffer_add(t_buffer *buffer, void *data, uint32_t size);

/**
 *
 * @fn    buffer_destroy
 * @brief Libera la memoria asociada al buffer
 * @param buffer
 */
void buffer_destroy(t_buffer *buffer);

/**
 *
 * @fn    buffer_destroy
 * @brief Guarda size bytes del principio del buffer en la dirección data y avanza el offset
 * @param buffer
 * @param data
 * @param size
 */
void buffer_read(t_buffer *buffer, void *data, uint32_t size);

t_paquete *recibir_paquete(int socket);

// con el debido casting de tipos de datos, ninguna de todas estas funciones sería necesaria

//
//// Agrega un uint32_t al buffer
// void buffer_add_uint32(t_buffer *buffer, uint32_t data);
//
//// Lee un uint32_t del buffer y avanza el offset
// uint32_t buffer_read_uint32(t_buffer *buffer);
//
//// Agrega un uint8_t al buffer
// void buffer_add_uint8(t_buffer *buffer, uint8_t data);
//
//// Lee un uint8_t del buffer y avanza el offset
// uint8_t buffer_read_uint8(t_buffer *buffer);
//
//// Agrega string al buffer con un uint32_t adelante indicando su longitud
// void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);
//
//// Lee un string y su longitud del buffer y avanza el offset
// char *buffer_read_string(t_buffer *buffer, uint32_t *length);

#endif