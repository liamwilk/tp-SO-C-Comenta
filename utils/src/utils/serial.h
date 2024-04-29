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
typedef enum
{
	MENSAJE,
	PAQUETE,
	DESCONECTAR,
	DAME_PROXIMA_INSTRUCCION,
	RECIBIR_UNA_INSTRUCCION,
	RECIBIR_PATH_INSTRUCCIONES,
	TERMINAR,
	IO_GEN_SLEEP,
	IO_GEN_SLEEP_TERMINADO,
	IO_AVISO_EXIT,
	IO_IDENTIFICADOR
} op_code;

typedef struct
{
	uint32_t size;	 // Tamaño del payload
	uint32_t offset; // Desplazamiento dentro del payload
	void *stream;	 // Payload
} t_buffer;

typedef struct
{
	op_code codigo_operacion; // Header
	uint32_t size_buffer;	  // Tamaño del buffer
	t_buffer *buffer;		  // Payload (puede ser un mensaje, un paquete, etc)
} t_paquete;

// typedef struct
// {
// 	uint32_t size_instruccion;	  // Tamaño de la instruccion
// 	char *instruccion;			  // Instruccion
// 	uint32_t cantidad_argumentos; // Cantidad de argumentos
// 	uint32_t size_argumentos;	  // Tamaño de los argumentos
// 	char **argumentos;			  // Array de argumentos
// } t_memoria_cpu_instruccion;

typedef struct
{
	uint32_t size_instruccion;	  // Tamaño de la instruccion
	char *instruccion;			  // Instruccion
	uint32_t cantidad_argumentos; // Cantidad de argumentos
	uint32_t size_argumento_1;	  // Tamaño del argumento
	char *argumento_1;			  // Size del argumento
	uint32_t size_argumento_2;	  // Tamaño del argumento
	char *argumento_2;			  // Size del argumento
	uint32_t size_argumento_3;	  // Tamaño del argumento
	char *argumento_3;			  // Size del argumento
	uint32_t size_argumento_4;	  // Tamaño del argumento
	char *argumento_4;			  // Size del argumento
	uint32_t size_argumento_5;	  // Tamaño del argumento
	char *argumento_5;			  // Size del argumento
} t_memoria_cpu_instruccion;

typedef struct t_kernel_memoria
{
	uint32_t size_path;		  // Tamaño del path
	char *path_instrucciones; // Path de las instrucciones
	uint32_t program_counter; // Program counter
} t_kernel_memoria;

/**
 * @fn    *crear_paquete
 * @brief Crea un paquete, y le asigna un buffer.
 * @param codigo_de_operacion op_code que va a tener el paquete.
 */
t_paquete *crear_paquete(op_code codigo_de_operacion);

/**
 * @fn    serializar_paquete
 * @brief Implementacion de la serializacion de un paquete
 * @param paquete Paquete con buffer y su op_code
 * @param bytes
 */
void *serializar_paquete(t_paquete *paquete, uint32_t bytes);

/**
 * @fn    enviar_mensaje
 * @brief Envia un mensaje `mensaje` al modulo conectado
 * @param mensaje Paquete con buffer y su op_code
 * @param socket_cliente
 */
void enviar_mensaje(char *mensaje, int socket_cliente);

/**
 * @fn    crear_buffer
 * @brief Crea un buffer y lo agrega al paquete
 * @param paquete Puntero al paquete donde se va a crear el buffer
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
 * @brief Recibe el buffer entrante
 * @param socket_cliente Socket desde el cual proviene el buffer
 */
t_buffer *recibir_buffer(int socket_cliente);

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

/**
 * @fn    recibir_paquete
 * @brief Invoca la funcion recv y recibe todo el paquete.
 * @param logger Logger que se usara para reportar errores
 * @param socket_cliente Socket desde el cual proviene el paquete
 */
t_paquete *recibir_paquete(t_log *logger, int socket_cliente);

/**
 * @fn    serializar_uint32_t
 * @brief Serializa un uint32_t en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint32_t(uint32_t valor, t_paquete *paquete);

/**
 * @fn    serializar_char
 * @brief Serializa un char en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_char(char *valor, t_paquete *paquete);

/**
 * @fn    actualizar_buffer
 * @brief En base a los valores cargados en el paquete, actualiza el buffer
 * @param paquete El paquete que se va a actualizar
 * @param size El tamaño total del buffer
 */
void actualizar_buffer(t_paquete *paquete, uint32_t size);

#endif