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
	DESCONECTAR,
	DAME_PROXIMA_INSTRUCCION,
	RECIBIR_UNA_INSTRUCCION,
	RECIBIR_PATH_INSTRUCCIONES,
	TERMINAR
} op_code;

typedef struct
{
	int size;
	void *stream;
} t_buffer;

typedef struct
{
	char *instruccion;
	char **argumentos;
} t_cpu_memoria_instruccion;

typedef struct t_kernel_memoria
{
	char *pathInstrucciones;
	int program_counter;
	// Agregar a demanda el struct
} t_kernel_memoria;

typedef struct
{
	op_code codigo_operacion; // Header
	int size;
	t_buffer *buffer;
} t_paquete;

t_paquete *crear_paquete(op_code codigo_de_operacion);

/**
 * @fn    serializar_paquete
 * @brief Implementacion de la serializacion de un paquete (strings)
 * @param paquete Paquete con buffer y su op_code
 * @param bytes
 */
void *serializar_paquete(t_paquete *paquete, int bytes);

/**
 * @fn deserializar_paquete
 * @brief Deserializa el paquete
 * @param buffer
 */
void *deserializar_paquete(t_buffer *buffer);

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
void *recibir_buffer(int socket_cliente);

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

t_paquete *recibir_paquete(int socket);

/**
 *
 * @fn    recibir_stream
 * @brief Realiza todo el proceso para recibir el paquete, deserializarlo y devolver un void* stream proveniente del buffer empaquetado.
 * @param socket_cliente
 */
void *recibir_stream(int socket_cliente);

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