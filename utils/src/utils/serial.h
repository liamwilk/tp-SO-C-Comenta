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
	CPU_MEMORIA_MOV_OUT,
	CPU_MEMORIA_MOV_IN,
	MEMORIA_CPU_IO_MOV_OUT,
	MEMORIA_CPU_NUMERO_FRAME,
	MEMORIA_CPU_IO_MOV_IN,
	MEMORIA_CPU_IO_MOV_IN_2,
	MEMORIA_CPU_IO_STDOUT_WRITE,
	CPU_KERNEL_IO_STDOUT_WRITE,
	CPU_MEMORIA_IO_STDIN_READ,
	MEMORIA_CPU_IO_STDIN_READ,
	MEMORIA_ENTRADA_SALIDA_IO_STDIN_READ,
	CPU_KERNEL_IO_STDIN_READ,
	KERNEL_ENTRADA_SALIDA_IO_STDIN_READ,
	ENTRADA_SALIDA_KERNEL_IO_STDIN_READ,
	KERNEL_ENTRADA_SALIDA_IO_STDOUT_WRITE,
	ENTRADA_SALIDA_MEMORIA_IO_STDOUT_WRITE,
	ENTRADA_SALIDA_KERNEL_IO_STDOUT_WRITE,
	MEMORIA_ENTRADA_SALIDA_IO_STDOUT_WRITE,
	ENTRADA_SALIDA_MEMORIA_IO_STDIN_READ,
	KERNEL_CPU_IO_STDIN_READ,
	KERNEL_CPU_IO_STDOUT_WRITE,
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
	ENTRADA_SALIDA_KERNEL_IDENTIFICACION,
	KERNEL_IO_INTERRUPCION,
	KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO,
	CPU_KERNEL_IO_FS_CREATE,
	CPU_KERNEL_IO_FS_TRUNCATE,
	CPU_KERNEL_IO_FS_WRITE,
	CPU_KERNEL_IO_FS_DELETE,
	KERNEL_ENTRADA_SALIDA_IO_FS_CREATE,
	KERNEL_ENTRADA_SALIDA_IO_FS_TRUNCATE,
	KERNEL_ENTRADA_SALIDA_IO_FS_WRITE,
	KERNEL_ENTRADA_SALIDA_IO_FS_READ,
	KERNEL_ENTRADA_SALIDA_IO_FS_DELETE,
	KERNEL_CPU_IO_FS_CREATE,
	ENTRADA_SALIDA_KERNEL_IO_FS_CREATE,
	ENTRADA_SALIDA_KERNEL_IO_FS_TRUNCATE,
	ENTRADA_SALIDA_KERNEL_IO_FS_DELETE,
	ENTRADA_SALIDA_KERNEL_IO_FS_WRITE,
	MEMORIA_CPU_COPY_STRING,
	CPU_MEMORIA_COPY_STRING_2,
	MEMORIA_CPU_COPY_STRING_2,
	KERNEL_MEMORIA_IO_FS_READ,
	MEMORIA_KERNEL_IO_FS_READ,
	MEMORIA_CPU_IO_FS_WRITE,
	CPU_MEMORIA_IO_FS_WRITE,
	CPU_KERNEL_IO_FS_READ,
	CPU_KERNEL_COPY_STRING,
	PLACEHOLDER,
	KERNEL_CPU_IO_FS_DELETE,
	ENTRADA_SALIDA_KERNEL_IO_FS_READ
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
	COPY_STRING_2,
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

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t marco;
	uint32_t numero_pagina;
	uint32_t direccion_fisica;
	uint32_t desplazamiento;
	uint32_t size_interfaz;
	t_registros_cpu registros;
	char *interfaz;
} t_io_stdout_write;

typedef struct
{
	uint32_t pid;
	uint32_t direccion_fisica;
	uint32_t tamanio;
} t_io_memoria_stdout;

typedef struct
{
	uint32_t pid;
	uint32_t direccion_fisica;
	uint32_t tamanio;
	uint32_t resultado;
	uint32_t size_dato;
	char *dato;
} t_memoria_io_stdout;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
} t_entrada_salida_kernel_io_stdout_write;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t size_motivo;
	char *motivo;
} t_kernel_cpu_io_stdout_write;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t marco_inicial;
	uint32_t marco_final;
	uint32_t numero_pagina;
	uint32_t direccion_fisica;
	uint32_t desplazamiento;
	uint32_t size_interfaz;
	t_registros_cpu registros;
	uint32_t cantidad_marcos;
	uint32_t size_marcos;
	char *marcos;
	char *interfaz;
} t_io_stdin_read;
typedef struct
{
	uint32_t pid;
	uint32_t tamanio_registro_datos;
	uint32_t registro_direccion;
	uint32_t registro_datos;
	uint32_t numero_pagina;
	uint32_t numero_marco;
	uint32_t resultado;
	uint32_t direccion_fisica;
	uint32_t dato_32;
	uint8_t dato_8;
} t_mov_in;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t marco_inicial;
	uint32_t marco_final;
	uint32_t numero_pagina;
	uint32_t direccion_fisica;
	uint32_t desplazamiento;
	uint32_t size_interfaz;
	t_registros_cpu registros;
	char *interfaz;
	char *marcos;
	uint32_t cantidad_marcos;
	uint32_t size_marcos;
} t_kernel_io_stdin_read;

typedef struct
{
	uint32_t pid;
	uint32_t direccion_fisica;
	uint32_t size_input;
	uint32_t registro_tamanio;
	char *input;
	char *marcos;
	uint32_t cantidad_marcos;
	uint32_t size_marcos;
} t_io_memoria_stdin;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
} t_memoria_entrada_salida_io_stdin_read;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
} t_entrada_salida_kernel_io_stdin_read;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t size_motivo;
	char *motivo;
} t_kernel_cpu_io_stdin_read;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t size_motivo;
	char *motivo;
} t_kernel_cpu_io_fs_create;

typedef struct
{
	uint32_t pid;
	uint32_t registro_direccion;
	uint32_t registro_datos;
	uint32_t numero_pagina;
	uint32_t numero_marco;
	uint32_t direccion_fisica;
	uint32_t resultado;
	uint32_t tamanio_registro_datos;
	uint32_t dato_32;
	uint8_t dato_8;
} t_mov_out;

typedef struct
{
	uint32_t pid;
	uint32_t direccion_si;
	uint32_t direccion_di;
	uint32_t direccion_fisica_si;
	uint32_t direccion_fisica_di;
	uint32_t num_pagina_si;
	uint32_t num_pagina_di;
	uint32_t marco_si;
	uint32_t marco_di;
	uint32_t cant_bytes;
	uint32_t size_frase;
	uint32_t resultado;
	char *frase;
} t_copy_string;

typedef struct
{
	uint32_t pid;
	uint32_t numero_pagina;
} t_cpu_memoria_numero_marco;

typedef struct
{
	uint32_t pid;
	uint32_t numero_pagina;
	uint32_t numero_marco;
	uint32_t resultado;
} t_memoria_cpu_numero_marco;

typedef struct
{
	uint32_t pid;
	uint32_t marco;
	uint32_t pagina;
	int ultimo_acceso;
} t_tlb;
typedef struct
{
	uint32_t pid;
	char *interfaz;
	char *nombre_archivo;
	uint32_t size_interfaz;
	uint32_t size_nombre_archivo;
	uint32_t resultado;
	t_registros_cpu registros;
} t_entrada_salida_fs_create;

typedef struct
{
	uint32_t pid;
	char *interfaz;
	char *nombre_archivo;
	uint32_t size_interfaz;
	uint32_t size_nombre_archivo;
	uint32_t tamanio_a_truncar;
	uint32_t resultado;
} t_kernel_entrada_salida_fs_truncate;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	char *escribir;
	uint32_t pid;
	uint32_t size_escribir;
	uint32_t size_interfaz;
	uint32_t size_nombre_archivo;
	uint32_t puntero_archivo;
	uint32_t resultado;
} t_kernel_entrada_salida_fs_write;

typedef struct
{
	uint32_t pid;
	char *interfaz;
	char *nombre_archivo;
	uint32_t size_interfaz;
	uint32_t size_nombre_archivo;
	uint32_t tamanio_a_truncar;
	t_registros_cpu registros;
	uint32_t resultado;
} t_cpu_kernel_fs_truncate;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	char *escribir;
	uint32_t pid;
	uint32_t size_escribir;
	uint32_t size_interfaz;
	uint32_t size_nombre_archivo;
	uint32_t puntero_archivo;
	uint32_t resultado;
	t_registros_cpu registros;
} t_cpu_kernel_fs_write;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	uint32_t pid;
	uint32_t size_nombre_archivo;
	uint32_t size_interfaz;
	uint32_t puntero_archivo;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t resultado;
	uint32_t direccion_fisica;
	uint32_t marco;
	uint32_t numero_pagina;
	uint32_t desplazamiento;
	t_registros_cpu registros;
} t_cpu_memoria_fs_write;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	char *dato;
	uint32_t pid;
	uint32_t size_nombre_archivo;
	uint32_t size_interfaz;
	uint32_t size_dato;
	uint32_t puntero_archivo;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t resultado;
	uint32_t direccion_fisica;
	uint32_t marco;
	uint32_t numero_pagina;
	uint32_t desplazamiento;
	t_registros_cpu registros;
} t_memoria_cpu_fs_write;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	uint32_t pid;
	uint32_t size_nombre_archivo;
	uint32_t size_interfaz;
	uint32_t puntero_archivo;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t resultado;
	uint32_t direccion_fisica;
	uint32_t marco;
	uint32_t numero_pagina;
	uint32_t desplazamiento;
	t_registros_cpu registros;
} t_cpu_kernel_fs_read;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	uint32_t pid;
	uint32_t size_nombre_archivo;
	uint32_t size_interfaz;
	uint32_t puntero_archivo;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t resultado;
	uint32_t direccion_fisica;
	uint32_t marco;
	uint32_t numero_pagina;
	uint32_t desplazamiento;
	t_registros_cpu registros;
} t_kernel_entrada_salida_fs_read;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	uint32_t pid;
	uint32_t size_nombre_archivo;
	uint32_t size_interfaz;
	uint32_t puntero_archivo;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t resultado;
	uint32_t direccion_fisica;
	uint32_t marco;
	uint32_t numero_pagina;
	uint32_t desplazamiento;
	char *dato;
	uint32_t size_dato;
} t_entrada_salida_fs_read_kernel;

typedef struct
{
	char *interfaz;
	char *nombre_archivo;
	uint32_t pid;
	uint32_t size_nombre_archivo;
	uint32_t size_interfaz;
	uint32_t puntero_archivo;
	uint32_t registro_direccion;
	uint32_t registro_tamanio;
	uint32_t resultado;
	uint32_t direccion_fisica;
	uint32_t marco;
	uint32_t numero_pagina;
	uint32_t desplazamiento;
	char *dato;
	uint32_t size_dato;
} t_kernel_memoria_fs_read;

typedef struct
{
	uint32_t pid;
	uint32_t resultado;
	uint32_t size_motivo;
	char *motivo;
} t_memoria_kernel_fs_read;

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

void serializar_t_cpu_memoria_numero_marco(t_paquete **paquete, t_cpu_memoria_numero_marco *enviar);

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

/**
 * @brief Serializa una estructura t_entrada_salida_identificacion en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param identificacion Puntero a la estructura t_entrada_salida_identificacion a serializar.
 */
void serializar_t_entrada_salida_identificacion(t_paquete **paquete, t_entrada_salida_identificacion *identificacion);

/**
 * @brief Deserializa una estructura t_entrada_salida_identificacion desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_entrada_salida_identificacion deserializada.
 */
t_entrada_salida_identificacion *deserializar_t_entrada_salida_identificacion(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_cpu_kernel_solicitud_recurso en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param contexto Puntero a la estructura t_cpu_kernel_solicitud_recurso a serializar.
 */
void serializar_t_cpu_kernel_solicitud_recurso(t_paquete **paquete, t_cpu_kernel_solicitud_recurso *contexto);

/**
 * @brief Deserializa una estructura t_cpu_kernel_solicitud_recurso desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_cpu_kernel_solicitud_recurso deserializada.
 */
t_cpu_kernel_solicitud_recurso *deserializar_t_cpu_kernel_solicitud_recurso(t_buffer *buffer);

/**
 * @brief Deserializa una estructura t_cpu_memoria_resize desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_cpu_memoria_resize deserializada.
 */
t_cpu_memoria_resize *deserializar_t_cpu_memoria_resize(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_cpu_memoria_resize en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param resize Puntero a la estructura t_cpu_memoria_resize a serializar.
 */
void serializar_t_cpu_memoria_resize(t_paquete **paquete, t_cpu_memoria_resize *resize);

/**
 * @brief Deserializa una estructura t_memoria_cpu_resize desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_memoria_cpu_resize deserializada.
 */
t_memoria_cpu_resize *deserializar_t_memoria_cpu_resize(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_memoria_cpu_resize en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param resize Puntero a la estructura t_memoria_cpu_resize a serializar.
 */
void serializar_t_memoria_cpu_resize(t_paquete **paquete, t_memoria_cpu_resize *resize);

/**
 * @brief Serializa una estructura t_cpu_kernel_resize en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param resize Puntero a la estructura t_cpu_kernel_resize a serializar.
 */
void serializar_t_cpu_kernel_resize(t_paquete **paquete, t_cpu_kernel_resize *resize);

/**
 * @brief Deserializa una estructura t_cpu_kernel_resize desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_cpu_kernel_resize deserializada.
 */
t_cpu_kernel_resize *deserializar_t_cpu_kernel_resize(t_buffer *buffer);

/**
 * @brief Deserializa una estructura t_io_stdout_write desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_io_stdout_write deserializada.
 */
t_io_stdout_write *deserializar_t_io_stdout_write(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_io_stdout_write en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param write Puntero a la estructura t_io_stdout_write a serializar.
 */
void serializar_t_io_stdout_write(t_paquete **paquete, t_io_stdout_write *write);

/**
 * @brief Serializa una estructura t_io_memoria_stdout en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param write Puntero a la estructura t_io_memoria_stdout a serializar.
 */
void serializar_t_io_memoria_stdout(t_paquete **paquete, t_io_memoria_stdout *write);

/**
 * @brief Deserializa una estructura t_io_memoria_stdout desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_io_memoria_stdout deserializada.
 */
t_io_memoria_stdout *deserializar_t_io_memoria_stdout(t_buffer *buffer);

/**
 * @brief Deserializa una estructura t_memoria_io_stdout desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_memoria_io_stdout deserializada.
 */
t_memoria_io_stdout *deserializar_t_memoria_io_stdout(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_memoria_io_stdout en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param write Puntero a la estructura t_memoria_io_stdout a serializar.
 */
void serializar_t_memoria_io_stdout(t_paquete **paquete, t_memoria_io_stdout *write);

/**
 * @brief Deserializa una estructura t_entrada_salida_kernel_io_stdout_write desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_entrada_salida_kernel_io_stdout_write deserializada.
 */
t_entrada_salida_kernel_io_stdout_write *deserializar_t_entrada_salida_kernel_io_stdout_write(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_entrada_salida_kernel_io_stdout_write en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param write Puntero a la estructura t_entrada_salida_kernel_io_stdout_write a serializar.
 */
void serializar_t_entrada_salida_kernel_io_stdout_write(t_paquete **paquete, t_entrada_salida_kernel_io_stdout_write *write);

/**
 * @brief Serializa una estructura t_kernel_cpu_io_stdout_write en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param write Puntero a la estructura t_kernel_cpu_io_stdout_write a serializar.
 */
void serializar_t_kernel_cpu_io_stdout_write(t_paquete **paquete, t_kernel_cpu_io_stdout_write *write);

/**
 * @brief Deserializa una estructura t_kernel_cpu_io_stdout_write desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_kernel_cpu_io_stdout_write deserializada.
 */
t_kernel_cpu_io_stdout_write *deserializar_t_kernel_cpu_io_stdout_write(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_io_stdin_read en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_io_stdin_read a serializar.
 */
void serializar_t_io_stdin_read(t_paquete **paquete, t_io_stdin_read *read);

/**
 * @brief Deserializa una estructura t_io_stdin_read desde un buffer.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_io_stdin_read deserializada.
 */
t_io_stdin_read *deserializar_t_io_stdin_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_kernel_cpu_io_stdin_read en un paquete.
 *
 * Esta función toma una estructura t_kernel_cpu_io_stdin_read y la serializa en un paquete
 * para su posterior envío a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_kernel_cpu_io_stdin_read que se desea serializar.
 */
void serializar_t_kernel_cpu_io_stdin_read(t_paquete **paquete, t_kernel_cpu_io_stdin_read *read);

/**
 * @brief Deserializa una estructura t_kernel_cpu_io_stdin_read desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_kernel_cpu_io_stdin_read serializada
 * y la deserializa en una estructura t_kernel_cpu_io_stdin_read.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_kernel_cpu_io_stdin_read deserializada.
 */
t_kernel_cpu_io_stdin_read *deserializar_t_kernel_cpu_io_stdin_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_kernel_io_stdin_read en un paquete.
 *
 * Esta función toma una estructura t_kernel_io_stdin_read y la serializa en un paquete
 * para su posterior envío a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_kernel_io_stdin_read que se desea serializar.
 */
void serializar_t_kernel_io_stdin_read(t_paquete **paquete, t_kernel_io_stdin_read *read);

/**
 * @brief Deserializa una estructura t_kernel_io_stdin_read desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_kernel_io_stdin_read serializada
 * y la deserializa en una estructura t_kernel_io_stdin_read.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_kernel_io_stdin_read deserializada.
 */
t_kernel_io_stdin_read *deserializar_t_kernel_io_stdin_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_io_memoria_stdin en un paquete.
 *
 * Esta función toma una estructura t_io_memoria_stdin y la serializa en un paquete
 * para su posterior envío a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_io_memoria_stdin que se desea serializar.
 */
void serializar_t_io_memoria_stdin(t_paquete **paquete, t_io_memoria_stdin *read);

/**
 * @brief Deserializa una estructura t_io_memoria_stdin desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_io_memoria_stdin serializada
 * y la deserializa, devolviendo un puntero a la estructura deserializada.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_io_memoria_stdin deserializada.
 */
t_io_memoria_stdin *deserializar_t_io_memoria_stdin(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_entrada_salida_kernel_io_stdin_read en un paquete.
 *
 * Esta función toma una estructura t_entrada_salida_kernel_io_stdin_read y la serializa en un paquete.
 * El paquete resultante se asigna a la dirección de memoria apuntada por el puntero paquete.
 *
 * @param paquete Puntero a un puntero de tipo t_paquete donde se almacenará el paquete serializado.
 * @param read Puntero a una estructura t_entrada_salida_kernel_io_stdin_read que se desea serializar.
 */
void serializar_t_entrada_salida_kernel_io_stdin_read(t_paquete **paquete, t_entrada_salida_kernel_io_stdin_read *read);

/**
 * @brief Deserializa un buffer en una estructura t_entrada_salida_kernel_io_stdin_read.
 *
 * Esta función toma un buffer y lo deserializa en una estructura t_entrada_salida_kernel_io_stdin_read.
 * La estructura resultante se devuelve como resultado de la función.
 *
 * @param buffer Puntero a un buffer que se desea deserializar.
 * @return Puntero a una estructura t_entrada_salida_kernel_io_stdin_read deserializada.
 */
t_entrada_salida_kernel_io_stdin_read *deserializar_t_entrada_salida_kernel_io_stdin_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_memoria_entrada_salida_io_stdin_read en un paquete.
 *
 * Esta función toma una estructura t_memoria_entrada_salida_io_stdin_read y la serializa en un paquete
 * para su posterior envío a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_memoria_entrada_salida_io_stdin_read que se desea serializar.
 */
void serializar_t_memoria_entrada_salida_io_stdin_read(t_paquete **paquete, t_memoria_entrada_salida_io_stdin_read *read);

/**
 * @brief Deserializa una estructura t_memoria_entrada_salida_io_stdin_read desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_memoria_entrada_salida_io_stdin_read serializada
 * y la deserializa en una estructura t_memoria_entrada_salida_io_stdin_read.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_memoria_entrada_salida_io_stdin_read deserializada.
 */
t_memoria_entrada_salida_io_stdin_read *deserializar_t_memoria_entrada_salida_io_stdin_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_memoria_cpu_numero_marco en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param enviar Puntero a la estructura t_memoria_cpu_numero_marco a serializar.
 */
void serializar_t_memoria_cpu_numero_marco(t_paquete **paquete, t_memoria_cpu_numero_marco *enviar);

/**
 * @brief Deserializa una estructura t_memoria_cpu_numero_marco desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_memoria_cpu_numero_marco deserializada.
 */
t_memoria_cpu_numero_marco *deserializar_t_memoria_cpu_numero_marco(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_mov_in en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param mov Puntero a la estructura t_mov_in a serializar.
 */
void serializar_t_mov_in(t_paquete **paquete, t_mov_in *mov);

/**
 * @brief Deserializa una estructura t_mov_in desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_mov_in deserializada.
 */
t_mov_in *deserializar_t_mov_in(t_buffer *buffer);

/**
 * @brief Deserializa una estructura t_mov_out desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_mov_out deserializada.
 */
t_mov_out *deserializar_t_mov_out(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_mov_out en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param mov Puntero a la estructura t_mov_out a serializar.
 */
void serializar_t_mov_out(t_paquete **paquete, t_mov_out *mov);

/**
 * @brief Serializa una estructura t_copy_string en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param copy Puntero a la estructura t_copy_string a serializar.
 */
void serializar_t_copy_string(t_paquete **paquete, t_copy_string *copy);

/**
 * @brief Deserializa una estructura t_copy_string desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_copy_string deserializada.
 */
t_copy_string *deserializar_t_copy_string(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_entrada_salida_fs_create en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param create Puntero a la estructura t_entrada_salida_fs_create a serializar.
 */
void serializar_t_entrada_salida_fs_create(t_paquete **paquete, t_entrada_salida_fs_create *create);

/**
 * @brief Deserializa una estructura t_entrada_salida_fs_create desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_entrada_salida_fs_create deserializada.
 */
t_entrada_salida_fs_create *deserializar_t_entrada_salida_fs_create(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_kernel_cpu_io_fs_create en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param read Puntero a la estructura t_kernel_cpu_io_fs_create a serializar.
 */
void serializar_t_kernel_cpu_io_fs_create(t_paquete **paquete, t_kernel_cpu_io_fs_create *read);

/**
 * @brief Deserializa una estructura t_kernel_cpu_io_fs_create desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_kernel_cpu_io_fs_create deserializada.
 */
t_kernel_cpu_io_fs_create *deserializar_t_kernel_cpu_io_fs_create(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_cpu_kernel_fs_truncate en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param truncate Puntero a la estructura t_cpu_kernel_fs_truncate a serializar.
 */
void serializar_t_cpu_kernel_fs_truncate(t_paquete **paquete, t_cpu_kernel_fs_truncate *truncate);

/**
 * @brief Deserializa una estructura t_cpu_kernel_fs_truncate desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_cpu_kernel_fs_truncate deserializada.
 */
t_cpu_kernel_fs_truncate *deserializar_t_cpu_kernel_fs_truncate(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_kernel_entrada_salida_fs_truncate en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param truncate Puntero a la estructura t_kernel_entrada_salida_fs_truncate a serializar.
 */
void serializar_t_kernel_entrada_salida_fs_truncate(t_paquete **paquete, t_kernel_entrada_salida_fs_truncate *truncate);

/**
 * @brief Deserializa una estructura t_kernel_entrada_salida_fs_truncate desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_kernel_entrada_salida_fs_truncate deserializada.
 */
t_kernel_entrada_salida_fs_truncate *deserializar_t_kernel_entrada_salida_fs_truncate(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_cpu_kernel_fs_write en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param write Puntero a la estructura t_cpu_kernel_fs_write a serializar.
 */
void serializar_t_cpu_kernel_fs_write(t_paquete **paquete, t_cpu_kernel_fs_write *write);

/**
 * @brief Deserializa una estructura t_cpu_kernel_fs_write desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_cpu_kernel_fs_write deserializada.
 */
t_cpu_kernel_fs_write *deserializar_t_cpu_kernel_fs_write(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_kernel_entrada_salida_fs_write en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param write Puntero a la estructura t_kernel_entrada_salida_fs_write a serializar.
 */
void serializar_t_kernel_entrada_salida_fs_write(t_paquete **paquete, t_kernel_entrada_salida_fs_write *write);

/**
 * @brief Deserializa una estructura t_kernel_entrada_salida_fs_write desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_kernel_entrada_salida_fs_write deserializada.
 */
t_kernel_entrada_salida_fs_write *deserializar_t_kernel_entrada_salida_fs_write(t_buffer *buffer);

/**
 * @brief Deserializa una estructura t_cpu_memoria_fs_write desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_cpu_memoria_fs_write deserializada.
 */
t_cpu_memoria_fs_write *deserializar_t_cpu_memoria_fs_write(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_cpu_memoria_fs_write en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param write Puntero a la estructura t_cpu_memoria_fs_write a serializar.
 */
void serializar_t_cpu_memoria_fs_write(t_paquete **paquete, t_cpu_memoria_fs_write *write);

/**
 * @brief Deserializa una estructura t_memoria_cpu_fs_write desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_memoria_cpu_fs_write deserializada.
 */
t_memoria_cpu_fs_write *deserializar_t_memoria_cpu_fs_write(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_memoria_cpu_fs_write en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param write Puntero a la estructura t_memoria_cpu_fs_write a serializar.
 */
void serializar_t_memoria_cpu_fs_write(t_paquete **paquete, t_memoria_cpu_fs_write *write);

/**
 * @brief Serializa una estructura t_cpu_kernel_fs_read en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se va a serializar la estructura.
 * @param read Puntero a la estructura t_cpu_kernel_fs_read a serializar.
 */
void serializar_t_cpu_kernel_fs_read(t_paquete **paquete, t_cpu_kernel_fs_read *read);

/**
 * @brief Deserializa una estructura t_cpu_kernel_fs_read desde un buffer.
 *
 * @param buffer Puntero al buffer desde donde se va a deserializar la estructura.
 * @return Puntero a la estructura t_cpu_kernel_fs_read deserializada.
 */
t_cpu_kernel_fs_read *deserializar_t_cpu_kernel_fs_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_kernel_entrada_salida_fs_read en un paquete.
 *
 * Esta función toma una estructura t_kernel_entrada_salida_fs_read y la serializa en un paquete
 * para su posterior envío a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_kernel_entrada_salida_fs_read que se desea serializar.
 */
void serializar_t_kernel_entrada_salida_fs_read(t_paquete **paquete, t_kernel_entrada_salida_fs_read *read);

/**
 * @brief Deserializa una estructura t_kernel_entrada_salida_fs_read desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_kernel_entrada_salida_fs_read serializada
 * y la deserializa en una estructura t_kernel_entrada_salida_fs_read.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_kernel_entrada_salida_fs_read deserializada.
 */
t_kernel_entrada_salida_fs_read *deserializar_t_kernel_entrada_salida_fs_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_entrada_salida_fs_read_kernel en un paquete.
 *
 * Esta función toma una estructura t_entrada_salida_fs_read_kernel y la serializa en un paquete.
 * El paquete resultante puede ser enviado a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_entrada_salida_fs_read_kernel que se desea serializar.
 */
void serializar_t_entrada_salida_fs_read_kernel(t_paquete **paquete, t_entrada_salida_fs_read_kernel *read);

/**
 * @brief Deserializa una estructura t_entrada_salida_fs_read_kernel desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_entrada_salida_fs_read_kernel serializada
 * y la deserializa en una estructura t_entrada_salida_fs_read_kernel.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_entrada_salida_fs_read_kernel deserializada.
 */
t_entrada_salida_fs_read_kernel *deserializar_t_entrada_salida_fs_read_kernel(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_kernel_memoria_fs_read en un paquete.
 *
 * Esta función toma una estructura t_kernel_memoria_fs_read y la serializa en un paquete.
 * El paquete resultante puede ser enviado a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_kernel_memoria_fs_read que se desea serializar.
 */
void serializar_t_kernel_memoria_fs_read(t_paquete **paquete, t_kernel_memoria_fs_read *read);

/**
 * @brief Deserializa una estructura t_kernel_memoria_fs_read desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_kernel_memoria_fs_read serializada
 * y la deserializa en una estructura t_kernel_memoria_fs_read.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_kernel_memoria_fs_read deserializada.
 */
t_kernel_memoria_fs_read *deserializar_t_kernel_memoria_fs_read(t_buffer *buffer);

/**
 * @brief Serializa una estructura t_memoria_kernel_fs_read en un paquete.
 *
 * Esta función toma una estructura t_memoria_kernel_fs_read y la serializa en un paquete
 * para su posterior envío a través de una conexión de red.
 *
 * @param paquete Puntero al puntero del paquete donde se almacenará la estructura serializada.
 * @param read Puntero a la estructura t_memoria_kernel_fs_read que se desea serializar.
 */
void serializar_t_memoria_kernel_fs_read(t_paquete **paquete, t_memoria_kernel_fs_read *read);

/**
 * @brief Deserializa una estructura t_memoria_kernel_fs_read desde un buffer.
 *
 * Esta función toma un buffer que contiene una estructura t_memoria_kernel_fs_read serializada
 * y la deserializa, devolviendo un puntero a la estructura deserializada.
 *
 * @param buffer Puntero al buffer que contiene la estructura serializada.
 * @return Puntero a la estructura t_memoria_kernel_fs_read deserializada.
 */
t_memoria_kernel_fs_read *deserializar_t_memoria_kernel_fs_read(t_buffer *buffer);

#endif