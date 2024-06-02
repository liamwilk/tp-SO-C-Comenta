#ifndef SERIAL_H_
#define SERIAL_H_
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <commons/log.h>
#include <commons/collections/list.h>

typedef enum
{
	FINALIZAR_SISTEMA,
	CPU_MEMORIA_PROXIMA_INSTRUCCION,
	CPU_KERNEL_IO_GEN_SLEEP,
	CPU_KERNEL_PROCESO,
	CPU_MEMORIA_TAM_PAGINA,
	CPU_MEMORIA_NUMERO_FRAME,
	CPU_KERNEL_WAIT,
	CPU_KERNEL_SIGNAL,
	CPU_MEMORIA_RESIZE,
	CPU_KERNEL_RESIZE,
	ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP,
	MEMORIA_KERNEL_NUEVO_PROCESO,
	MEMORIA_CPU_PROXIMA_INSTRUCCION,
	MEMORIA_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO,
	MEMORIA_ENTRADA_SALIDA_IDENTIFICACION,
	MEMORIA_CPU_RESIZE,
	MEMORIA_CPU_TAM_PAGINA,
	KERNEL_CPU_EJECUTAR_PROCESO,
	KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP,
	KERNEL_MEMORIA_NUEVO_PROCESO,
	KERNEL_MEMORIA_FINALIZAR_PROCESO,
	KERNEL_CPU_INTERRUPCION,
	KERNEL_ENTRADA_SALIDA_IDENTIFICACION,
	KERNEL_IO_INTERRUPCION,
	KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO,
	PLACEHOLDER
} t_op_code;

typedef enum
{
	SET,
	SUM,
	SUB,
	JNZ,
	IO_GEN_SLEEP,
	MOV_IN,
	MOV_OUT,
	RESIZE,
	COPY_STRING,
	IO_STDIN_READ,
	IO_STDOUT_WRITE,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ,
	WAIT,
	SIGNAL,
	EXIT
} t_instruccion;

typedef enum
{
	INVALIDO,
	REGISTRO_32,
	REGISTRO_8,
	EAX,
	EBX,
	ECX,
	EDX,
	SI,
	DI,
	AX,
	BX,
	CX,
	DX
} t_registro;

typedef struct
{
	uint32_t size;
	uint32_t offset;
	void *stream;
} t_buffer;

typedef struct
{
	t_op_code codigo_operacion;
	uint32_t size_buffer;
	t_buffer *buffer;
} t_paquete;

typedef struct
{
	uint32_t pid;
	uint32_t cantidad_elementos;
	char **array;
} t_memoria_cpu_instruccion;

typedef struct t_registros_cpu
{
	uint32_t pc, eax, ebx, ecx, edx, si, di;
	uint8_t ax, bx, cx, dx;
} t_registros_cpu;

typedef struct
{
	uint32_t program_counter;
	uint32_t pid;
} t_cpu_memoria_instruccion;

typedef struct
{
	uint32_t pid;
	uint32_t len_motivo;
	char *motivo;
} t_kernel_cpu_interrupcion;

typedef struct
{
	uint32_t pid;
	uint32_t size_interfaz;
	char *interfaz;
	uint32_t tiempo;
	t_registros_cpu registros; // Registros de la CPU para la vuelta de IO_GEN_SLEEP
} t_cpu_kernel_io_gen_sleep;

typedef struct
{
	uint32_t size_path;
	char *path_instrucciones;
	uint32_t program_counter;
	uint32_t pid;
} t_kernel_memoria_proceso;

typedef struct
{
	uint32_t pid;
	uint32_t cantidad_instrucciones;
	bool leido;
} t_memoria_kernel_proceso;

typedef struct
{
	uint32_t pid;
} t_kernel_memoria_finalizar_proceso;

typedef struct
{
	uint32_t pid;
	uint32_t unidad_de_trabajo;
} t_kernel_entrada_salida_unidad_de_trabajo;

typedef struct
{
	uint32_t pid;
	bool terminado;
} t_entrada_salida_kernel_unidad_de_trabajo;

typedef struct
{
	bool terminado;
} t_entrada_salida_kernel_finalizar;

typedef struct
{
	uint32_t pid;
	t_registros_cpu registros;
} t_kernel_cpu_proceso;

typedef struct
{
	uint32_t pid;
	uint32_t ejecutado;
	t_registros_cpu registros;
} t_cpu_kernel_proceso;

typedef struct
{
	uint32_t pid;
	uint32_t numero_pagina;
} t_cpu_memoria_numero_frame;

typedef struct
{
	uint32_t size_identificador;
	char *identificador;
} t_entrada_salida_identificacion;

typedef struct
{
	uint32_t pid;
	t_registros_cpu *registros;
	uint32_t size_nombre_recurso;
	char *nombre_recurso;
} t_cpu_kernel_solicitud_recurso;

typedef struct
{
	uint32_t pid;
	uint32_t len_motivo;
	char *motivo;
} t_kernel_io_interrupcion;

typedef struct
{
	uint32_t pid;
	uint32_t bytes;
} t_cpu_memoria_resize;

typedef struct
{
	uint32_t pid;
	uint32_t bytes;
	uint32_t resultado;
	uint32_t size_motivo;
	char *motivo;
} t_memoria_cpu_resize;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t size_motivo;
	char *motivo;
	t_registros_cpu registros;
} t_cpu_kernel_resize;

/**
 * @fn    *crear_paquete
 * @brief Crea un paquete, y le asigna un buffer.
 * @param codigo_de_operacion t_op_code que va a tener el paquete.
 */
t_paquete *
crear_paquete(t_op_code codigo_de_operacion);

/**
 * @fn    *serializar_paquete
 * @brief Implementacion de la serializacion de un paquete
 * @param paquete Paquete con buffer y su t_op_code
 * @param bytes
 */
void *serializar_paquete(t_paquete *paquete, uint32_t bytes);

/**
 * @fn    enviar_mensaje
 * @brief Envia un mensaje `mensaje` al modulo conectado
 * @param mensaje Paquete con buffer y su t_op_code
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
 * @fn    *recibir_buffer
 * @brief Recibe el buffer entrante
 * @param socket_cliente Socket desde el cual proviene el buffer
 */
t_buffer *recibir_buffer(t_log *logger, int *socket_cliente);

/**
 *
 * @fn    recibir_operacion
 * @brief Invoca la funcion recv y recibe el codigo de operacion desde el socket
 * @param socket_cliente
 */
int recibir_operacion(int socket_cliente);

/**
 * @fn    *recibir_paquete
 * @brief Invoca la funcion recv y recibe todo el paquete.
 * @param logger Logger que se usara para reportar errores
 * @param socket_cliente Socket desde el cual proviene el paquete
 */
t_paquete *recibir_paquete(t_log *logger, int *socket_cliente);

/**
 * @fn    serializar_bool
 * @brief Serializa un bool en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_bool(bool valor, t_paquete *paquete);

/**
 * @brief deserializar_bool
 *
 * Esta función se encarga de deserializar un bool de un flujo de datos.
 *
 * @param flujo Puntero a punteor al flujo de datos.
 * @param destino_del_dato Puntero al destino donde se guardará el dato deserializado.
 */
void deserializar_bool(void **flujo, bool *destino_del_dato);

/**
 * @fn    serializar_uint8_t
 * @brief Serializa un uint8_t en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint8_t(uint8_t valor, t_paquete *paquete);

/**
 * @fn    serializar_uint16_t
 * @brief Serializa un uint16_t en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint16_t(uint16_t valor, t_paquete *paquete);

/**
 * @fn    serializar_uint32_t
 * @brief Serializa un uint32_t en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint32_t(uint32_t valor, t_paquete *paquete);

/**
 * @fn    serializar_uint64_t
 * @brief Serializa un uint64_t en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint64_t(uint64_t valor, t_paquete *paquete);

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
 * @param size El tamaño total del stream. Es decir, el tamaño total de lo que contiene nuestro struct que queremos enviar.
 *
 * Ejemplo:
 * struct t_kernel_memoria_proceso {
 * 	uint32_t size_path;
 * 	char* path_instrucciones;
 * 	uint32_t program_counter;
 * 	uint32_t pid;
 * }
 *
 * En este caso, el size seria sizeof(uint32_t) + strlen(path_instrucciones) + 1 + sizeof(uint32_t) + sizeof(uint32_t)
 */
void actualizar_buffer(t_paquete *paquete, uint32_t size);

/**
 * @brief Revisar un paquete.
 *
 * Esta función se encarga de revisar un paquete y realizar las acciones necesarias
 * según el contenido del paquete. Recibe como parámetros un puntero al paquete a revisar
 * y un puntero al logger donde se registrarán los eventos.
 *
 * @param paquete Puntero al paquete a revisar.
 * @param logger Puntero al logger donde se registrarán los eventos.
 * @param flag Flag que indica si se debe revisar el tamaño del buffer.
 * @param modulo Nombre del módulo que está revisando el paquete.
 */
void revisar_paquete(t_paquete *paquete, t_log *logger, char *modulo);

/**
 * @brief deserializar_uint32_t
 *
 * Esta función se encarga de deserializar un uint32_t de un flujo de datos.
 *
 * @param flujo Puntero a punteor al flujo de datos.
 * @param destino_del_dato Puntero al destino donde se guardará el dato deserializado.
 */
void deserializar_uint32_t(void **flujo, uint32_t *destino_del_dato);

/**
 * @brief deserializar_char
 *
 * Esta función se encarga de deserializar un char de un flujo de datos.
 *
 * @param flujo Puntero a punteor al flujo de datos.
 * @param destino_del_dato Puntero al destino donde se guardará el dato deserializado.
 * @param size_del_dato Tamaño del dato a deserializar.
 */
void deserializar_char(void **flujo, char **destino_del_dato, uint32_t size_del_dato);

/**
 * @brief deserializar_uint64_t
 *
 * Esta función se encarga de deserializar un uint64_t de un flujo de datos.
 *
 * @param flujo Puntero a punteor al flujo de datos.
 * @param destino_del_dato Puntero al destino donde se guardará el dato deserializado.
 */
void deserializar_uint64_t(void **flujo, uint64_t *destino_del_dato);

/**
 * @brief deserializar_uint16_t
 *
 * Esta función se encarga de deserializar un uint16_t de un flujo de datos.
 *
 * @param flujo Puntero a punteor al flujo de datos.
 * @param destino_del_dato Puntero al destino donde se guardará el dato deserializado.
 */
void deserializar_uint16_t(void **flujo, uint16_t *destino_del_dato);

/**
 * @brief deserializar_uint8_t
 *
 * Esta función se encarga de deserializar un uint8_t de un flujo de datos.
 *
 * @param flujo Puntero a punteor al flujo de datos.
 * @param destino_del_dato Puntero al destino donde se guardará el dato deserializado.
 */
void deserializar_uint8_t(void **flujo, uint8_t *destino_del_dato);

/**
 * @fn    serializar_op_code
 * @brief Serializa un t_op_code en el paquete
 * @param valor El valor a serializar
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_op_code(t_op_code valor, t_paquete *paquete);

/**
 * @brief deserializar_op_code
 *
 * Esta función se encarga de deserializar un t_op_code de un flujo de datos.
 *
 * @param flujo Puntero a punteor al flujo de datos.
 * @param destino_del_dato Puntero al destino donde se guardará el dato deserializado.
 */
void deserializar_op_code(void **flujo, t_op_code *destino_del_dato);

/**
 * @fn    serializar_t_kernel_memoria_proceso
 * @brief Serializa un t_kernel_memoria_proceso en un paquete
 * @param buffer El paquete a serializar
 */
void serializar_t_kernel_memoria_proceso(t_paquete **paquete, t_kernel_memoria_proceso *proceso);

/**
 * @fn    serializar_t_memoria_kernel_proceso
 * @brief Serializa un t_memoria_kernel_proceso en un paquete
 * @param buffer El paquete a serializar
 */
void serializar_t_memoria_kernel_proceso(t_paquete **paquete, t_memoria_kernel_proceso *proceso);

/**
 * @fn    serializar_t_registros_cpu
 * @brief Serializa un t_registros_cpu en un paquete
 * @param buffer El paquete a serializar
 */
void serializar_t_registros_cpu(t_paquete **paquete, uint32_t pid, t_registros_cpu *registros);

/**
 * @fn    serializar_t_kernel_memoria_finalizar_proceso
 * @brief Serializa un t_kernel_memoria_finalizar_proceso en un paquete
 * @param buffer El paquete a serializar
 */
void serializar_t_kernel_memoria_finalizar_proceso(t_paquete **paquete, t_kernel_memoria_finalizar_proceso *proceso);

/**
 * @fn    deserializar_t_memoria_kernel_proceso
 * @brief Deserializa un t_memoria_kernel_proceso en un paquete
 * @param buffer El paquete a serializar
 * @return *t_memoria_kernel_proceso
 */
t_memoria_kernel_proceso *deserializar_t_memoria_kernel_proceso(t_buffer *buffer);

/**
 * @fn    serializar_t_kernel_entrada_salida_unidad_de_trabajo
 * @brief Serializa un t_kernel_entrada_salida_unidad_de_trabajo en un paquete
 * @param buffer El paquete a serializar
 * @param unidad El dato a serializar
 */
void serializar_t_kernel_entrada_salida_unidad_de_trabajo(t_paquete **paquete, t_kernel_entrada_salida_unidad_de_trabajo *unidad);

/**
 * @fn    deserializar_t_kernel_entrada_salida_unidad_de_trabajo
 * @brief Deserializa un t_entrada_salida_kernel_unidad_de_trabajo en un paquete
 * @param buffer El paquete a serializar
 * @return *t_kernel_entrada_salida_unidad_de_trabajo
 */
t_kernel_entrada_salida_unidad_de_trabajo *deserializar_t_kernel_entrada_salida_unidad_de_trabajo(t_buffer *buffer);

/**
 * @fn    serializar_t_entrada_salida_kernel_unidad_de_trabajo
 * @brief Serializa un t_entrada_salida_kernel_unidad_de_trabajo en un paquete
 * @param buffer El paquete a serializar
 * @param unidad El dato a serializar
 */
void serializar_t_entrada_salida_kernel_unidad_de_trabajo(t_paquete **paquete, t_entrada_salida_kernel_unidad_de_trabajo *unidad);

/**
 * @fn    deserializar_t_entrada_salida_kernel_unidad_de_trabajo
 * @brief Deserializa un t_entrada_salida_kernel_unidad_de_trabajo en un paquete
 * @param buffer El paquete a serializar
 * @return *t_entrada_salida_kernel_unidad_de_trabajo
 */
t_entrada_salida_kernel_unidad_de_trabajo *deserializar_t_entrada_salida_kernel_unidad_de_trabajo(t_buffer *buffer);

/**
 * @fn    deserializar_t_entrada_salida_kernel_finalizar
 * @brief Deserializa un t_entrada_salida_kernel_finalizar en un paquete
 * @param buffer El paquete a serializar
 * @return *t_entrada_salida_kernel_finalizar
 */
t_entrada_salida_kernel_finalizar *deserializar_t_entrada_salida_kernel_finalizar(t_buffer *buffer);

/**
 * @fn    serializar_t_entrada_salida_kernel_finalizar
 * @brief Serializa un t_entrada_salida_kernel_finalizar en un paquete
 * @param buffer El paquete a serializar
 * @param unidad El dato a serializar
 */
void serializar_t_entrada_salida_kernel_finalizar(t_paquete **paquete, t_entrada_salida_kernel_finalizar *unidad);

/**
 * @fn    deserializar_t_memoria_cpu_instruccion
 * @brief Deserializa un t_memoria_cpu_instruccion en un paquete
 * @param buffer El paquete a serializar
 * @return *t_memoria_cpu_instruccion
 */
t_memoria_cpu_instruccion *deserializar_t_memoria_cpu_instruccion(t_buffer *buffer);

/**
 * @fn    serializar_t_memoria_cpu_instruccion
 * @brief Serializa un t_memoria_cpu_instruccion en un paquete
 * @param paquete El paquete a serializar
 * @param proceso La instruccion a serializar
 */
void serializar_t_memoria_cpu_instruccion(t_paquete **paquete, t_memoria_cpu_instruccion *proceso);

/**
 * @fn    serializar_char_array
 * @brief Serializa un array de char en el paquete
 * @param array El array que se quiere serializar
 * @param cantidad_elementos La cantidad de elementos del array
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_char_array(char **array, uint32_t cantidad_elementos, t_paquete *paquete);

/**
 * @brief deserializar_char_array
 *
 * Esta función se encarga de deserializar un array de char de un flujo de datos.
 * @param array Puntero triple al array de char donde se guardará el dato deserializado.
 * @param cantidad_elementos Puntero donde se guardará la cantidad de elementos del array.
 * @param buffer Buffer de los datos a deserializar.
 */
void deserializar_char_array(char ***array, uint32_t *cantidad_elementos, void **buffer);

/**
 * @fn    serializar_uint8_t_array
 * @brief Serializa un array de uint8_t en el paquete
 * @param array El array que se quiere serializar
 * @param cantidad_elementos La cantidad de elementos del array
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint8_t_array(uint8_t *array, uint32_t cantidad_elementos, t_paquete *paquete);

/**
 * @brief deserializar_uint8_t_array
 *
 * Esta función se encarga de deserializar un array de uint8_t de un flujo de datos.
 * @param array Puntero de puntero al array de uint8_t donde se guardará el dato deserializado.
 * @param cantidad_elementos Puntero donde se guardará la cantidad de elementos del array.
 * @param buffer Buffer de los datos a deserializar.
 */
void deserializar_uint8_t_array(uint8_t **array, uint8_t *cantidad_elementos, void **buffer);

/**
 * @fn    serializar_uint16_t_array
 * @brief Serializa un array de uint16_t en el paquete
 * @param array El array que se quiere serializar
 * @param cantidad_elementos La cantidad de elementos del array
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint16_t_array(uint16_t *array, uint32_t cantidad_elementos, t_paquete *paquete);

/**
 * @brief deserializar_uint16_t_array
 *
 * Esta función se encarga de deserializar un array de uint16_t de un flujo de datos.
 * @param array Puntero de puntero al array de uint16_t donde se guardará el dato deserializado.
 * @param cantidad_elementos Puntero donde se guardará la cantidad de elementos del array.
 * @param buffer Buffer de los datos a deserializar.
 */
void deserializar_uint16_t_array(uint16_t **array, uint16_t *cantidad_elementos, void **buffer);

/**
 * @fn    serializar_uint32_t_array
 * @brief Serializa un array de uint32_t en el paquete
 * @param array El array que se quiere serializar
 * @param cantidad_elementos La cantidad de elementos del array
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint32_t_array(uint32_t *array, uint32_t cantidad_elementos, t_paquete *paquete);

/**
 * @brief deserializar_uint32_t_array
 *
 * Esta función se encarga de deserializar un array de uint32_t de un flujo de datos.
 * @param array Puntero de puntero al array de uint32_t donde se guardará el dato deserializado.
 * @param cantidad_elementos Puntero donde se guardará la cantidad de elementos del array.
 * @param buffer Buffer de los datos a deserializar.
 */
void deserializar_uint32_t_array(uint32_t **array, uint32_t *cantidad_elementos, void **buffer);

/**
 * @fn    serializar_uint64_t_array
 * @brief Serializa un array de uint64_t en el paquete
 * @param array El array que se quiere serializar
 * @param cantidad_elementos La cantidad de elementos del array
 * @param paquete El puntero al paquete donde se serializara
 */
void serializar_uint64_t_array(uint64_t *array, uint32_t cantidad_elementos, t_paquete *paquete);

/**
 * @brief deserializar_uint64_t_array
 *
 * Esta función se encarga de deserializar un array de uint64_t de un flujo de datos.
 * @param array Puntero de puntero al array de uint64_t donde se guardará el dato deserializado.
 * @param cantidad_elementos Puntero donde se guardará la cantidad de elementos del array.
 * @param buffer Buffer de los datos a deserializar.
 */
void deserializar_uint64_t_array(uint64_t **array, uint64_t *cantidad_elementos, void **buffer);

void serializar_t_cpu_memoria_instruccion(t_paquete **paquete, t_cpu_memoria_instruccion *proceso);

t_cpu_kernel_proceso *deserializar_t_cpu_kernel_proceso(t_buffer *buffer);

void serializar_t_cpu_kernel_proceso(t_paquete **paquete, t_cpu_kernel_proceso *proceso);

t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer);

t_cpu_kernel_io_gen_sleep *deserializar_t_cpu_kernel_io_gen_sleep(t_buffer *buffer);

void serializar_t_cpu_kernel_io_gen_sleep(t_paquete **paquete, t_cpu_kernel_io_gen_sleep *unidad);

void serializar_t_kernel_cpu_interrupcion(t_paquete **paquete, t_kernel_cpu_interrupcion *interrupcion);

t_kernel_cpu_interrupcion *deserializar_t_kernel_cpu_interrupcion(t_buffer *buffer);

/**
 * @fn    *deserializar_t_kernel_memoria
 * @brief Deserializa un buffer en un t_kernel_memoria_proceso
 * @param buffer El buffer a deserializar
 */
t_kernel_memoria_proceso *deserializar_t_kernel_memoria(t_buffer *buffer);

/**
 * @fn    *deserializar_t_cpu_memoria_instruccion
 * @brief Deserializa un buffer en un t_cpu_memoria_instruccion
 * @param buffer El buffer a deserializar
 * @return *t_cpu_memoria_instruccion
 */
t_cpu_memoria_instruccion *deserializar_t_cpu_memoria_instruccion(t_buffer *buffer);

/**
 * @fn    serializar_t_memoria_cpu_instruccion
 * @brief Serializa un t_memoria_cpu_instruccion en un paquete
 * @param paquete_instruccion El paquete a serializar
 * @param instruccion La instruccion a serializar
 * @return *t_cpu_memoria_instruccion
 */
void serializar_t_memoria_kernel_proceso(t_paquete **paquete, t_memoria_kernel_proceso *proceso);

/**
 * @brief Deserializa un buffer en una estructura t_kernel_memoria_finalizar_proceso.
 *
 * @param buffer El buffer a deserializar.
 * @return Un puntero a la estructura t_kernel_memoria_finalizar_proceso deserializada.
 */
void serializar_t_kernel_io_interrupcion(t_paquete **paquete, t_kernel_io_interrupcion *interrupcion);

t_kernel_io_interrupcion *deserializar_t_kernel_io_interrupcion(t_buffer *buffer);

t_kernel_memoria_finalizar_proceso *deserializar_t_kernel_memoria_finalizar_proceso(t_buffer *buffer);

/**
 * @brief Remueve el salto de línea de una cadena de caracteres.
 *
 * @param argumento_origen La cadena de caracteres de origen.
 */
void remover_salto_linea(char *argumento_origen);

/**
 * @brief Serializa el tamaño de página de memoria de una CPU en un paquete.
 *
 * @param paquete El paquete en el que se serializará el tamaño de página.
 * @param tam_pagina El tamaño de página de memoria de la CPU.
 */
void serializar_t_memoria_cpu_tam_pagina(t_paquete **paquete, uint32_t tam_pagina);

/**
 * @brief Deserializa un buffer en un puntero a un entero que representa el tamaño de página de memoria de una CPU.
 *
 * @param buffer El buffer a deserializar.
 * @return Un puntero al tamaño de página de memoria deserializado.
 */
uint32_t *deserializar_t_memoria_cpu_tam_pagina(t_buffer *buffer);

/**
 * Serializa un número de marco de memoria CPU en un paquete.
 *
 * Esta función serializa el número de marco de memoria CPU dado en un paquete `paquete`.
 *
 * @param paquete Un puntero doble al paquete que se va a serializar.
 * @param numero_marco El número de marco de memoria CPU que se va a serializar.
 */
void serializar_t_memoria_cpu_numero_marco(t_paquete **paquete, uint32_t numero_marco);

/**
 * Deserializa una estructura t_memoria_cpu_numero_marco a partir de un búfer.
 *
 * @param buffer El búfer que contiene los datos serializados.
 * @return Un puntero a la estructura t_memoria_cpu_numero_marco deserializada.
 */
uint32_t *deserializar_t_memoria_cpu_numero_marco(t_buffer *buffer);

/**
 * @brief Serializa un número de página junto con el PID de un proceso en un paquete.
 *
 * Esta función toma un puntero a un paquete y los valores del PID y número de página de un proceso.
 * Luego, serializa estos valores en el paquete para su posterior envío.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenarán los datos serializados.
 * @param pid El ID del proceso.
 * @param numero_pagina El número de página del proceso.
 */
void serializar_t_cpu_memoria_numero_marco(t_paquete **paquete, uint32_t pid, int numero_pagina);

/**
 * @brief Deserializa un buffer y devuelve una estructura t_cpu_memoria_numero_frame.
 *
 * Esta función toma un buffer y extrae los datos necesarios para crear una estructura t_cpu_memoria_numero_frame.
 * Luego, devuelve la estructura creada.
 *
 * @param buffer El buffer que contiene los datos a deserializar.
 * @return Una estructura t_cpu_memoria_numero_frame con los datos deserializados.
 */
t_cpu_memoria_numero_frame *deserializar_t_cpu_memoria_numero_frame(t_buffer *buffer);

void serializar_t_entrada_salida_identificacion(t_paquete **paquete, t_entrada_salida_identificacion *identificacion);

t_entrada_salida_identificacion *deserializar_t_entrada_salida_identificacion(t_buffer *buffer);

void serializar_t_cpu_kernel_solicitud_recurso(t_paquete **paquete, t_cpu_kernel_solicitud_recurso *contexto);

t_cpu_kernel_solicitud_recurso *deserializar_t_cpu_kernel_solicitud_recurso(t_buffer *buffer);

t_cpu_memoria_resize *deserializar_t_cpu_memoria_resize(t_buffer *buffer);

void serializar_t_cpu_memoria_resize(t_paquete **paquete, t_cpu_memoria_resize *resize);

t_memoria_cpu_resize *deserializar_t_memoria_cpu_resize(t_buffer *buffer);

void serializar_t_memoria_cpu_resize(t_paquete **paquete, t_memoria_cpu_resize *resize);

void serializar_t_cpu_kernel_resize(t_paquete **paquete, t_cpu_kernel_resize *resize);

t_cpu_kernel_resize *deserializar_t_cpu_kernel_resize(t_buffer *buffer);

#endif