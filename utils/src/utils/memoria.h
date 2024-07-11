#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <utils/handshake.h>
#include <utils/modulos.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>
#include <commons/collections/list.h>
#include <utils/template.h>
#include <commons/bitarray.h>

typedef enum
{
    STDIN,
    STDOUT,
    DIALFS
} t_tipo_entrada_salida;

typedef struct
{
    int pid;
    bool identificado;
    bool valido;
    int ocupado;
    int orden;
    int socket;
    char *interfaz;
    t_tipo_entrada_salida tipo;
} t_entrada_salida;

typedef struct
{
    int marco;
    int validez;
    int offset;
    int bytes;
    int offset_inicio;
    int offset_fin;
} t_pagina;

typedef struct
{
    uint32_t pid;
    uint32_t pc;
    t_list *instrucciones;
    t_list *tabla_paginas;
    int bytes_usados;
    pthread_mutex_t mutex_tabla_paginas;
} t_proceso;

typedef struct
{
    pthread_t thread_atender_cpu;
    pthread_t thread_atender_kernel;
    pthread_t thread_atender_entrada_salida;
    pthread_t thread_esperar_cpu;
    pthread_t thread_conectar_kernel;
    pthread_t thread_atender_entrada_salida_stdin;
    pthread_t thread_atender_entrada_salida_stdout;
    pthread_t thread_atender_entrada_salida_dialfs;
} t_threads;

typedef struct
{
    int socket_cpu;
    int socket_kernel;
    int socket_server_memoria;
    int socket_entrada_salida_stdin;
    int socket_entrada_salida_stdout;
    int socket_entrada_salida_dialfs;
    int id_entrada_salida;
} t_sockets;

typedef struct
{ // TODO: Re-estructurar
    void *datos;
    int *bytes_usados;
    int *bitmap_array;
} t_espacio_usuario;

typedef struct
{
    void *espacio_usuario;
    int *bytes_usados;
    int *bitmap_array;
    pthread_mutex_t mutex_diccionario_procesos;
    t_log *logger;
    t_config *config;
    t_list *lista_procesos;
    t_list *lista_entrada_salida;
    t_dictionary *diccionario_procesos;
    t_dictionary *diccionario_entrada_salida;
    t_sockets sockets;
    t_threads threads;
    int tamMemoria;
    int tamPagina;
    int retardoRespuesta;
    int puertoEscucha;
    char *pathInstrucciones;
} t_memoria;

typedef struct
{
    t_log *logger;
    t_memoria memoria;
} t_args;

typedef struct
{
    t_args *argumentos;
    t_entrada_salida *entrada_salida;
} t_args_hilo;

typedef struct
{
    uint32_t direccion_fisica;
    uint32_t frame;
} t_frame_disponible;

typedef struct
{
    uint32_t marco;
    uint32_t bytes;
    uint32_t offset;
} t_frame_bytes;

typedef void (*t_mem_funcion_hilo_ptr)(t_args_hilo *, char *, t_op_code, t_buffer *);
typedef void (*t_mem_funcion_ptr)(t_args *, t_op_code, t_buffer *);

/**
 * @brief Agrega una interfaz a la lista de entradas/salidas.
 *
 * @param argumentos Los argumentos del programa.
 * @param tipo El tipo de entrada/salida.
 * @param socket El socket de la interfaz.
 * @return Un puntero a la estructura de entrada/salida agregada.
 */
t_entrada_salida *agregar_interfaz(t_args *argumentos, t_tipo_entrada_salida tipo, int socket);

/**
 * @brief Busca una interfaz en la lista de entradas/salidas.
 *
 * @param argumentos Los argumentos del programa.
 * @param interfaz El nombre de la interfaz a buscar.
 * @return Un puntero a la estructura de entrada/salida encontrada, o NULL si no se encontró.
 */
t_entrada_salida *buscar_interfaz(t_args *argumentos, char *interfaz);

/**
 * @brief Lee las instrucciones de un archivo y las guarda en una lista.
 *
 * @param argumentos Los argumentos del programa.
 * @param path_instrucciones La ruta del archivo de instrucciones.
 * @param pid El ID del proceso.
 * @return Una lista de instrucciones leídas.
 */
t_list *leer_instrucciones(t_args *argumentos, char *path_instrucciones, uint32_t pid);

t_entrada_salida *agregar_entrada_salida(t_args *argumentos, t_tipo_entrada_salida type, int socket);

/**
 * @brief Busca un proceso en la lista de procesos.
 *
 * @param argumentos Los argumentos del programa.
 * @param pid El ID del proceso a buscar.
 * @return Un puntero a la estructura de proceso encontrada, o NULL si no se encontró.
 */
t_proceso *buscar_proceso(t_args *argumentos, uint32_t pid);

/**
 * @brief Combina dos rutas en una sola.
 *
 * @param ruta1 La primera ruta.
 * @param ruta2 La segunda ruta.
 * @return La ruta combinada.
 */
char *armar_ruta(char *ruta1, char *ruta2);

/**
 * @brief Agrega una entrada/salida a la lista de entradas/salidas.
 *
 * @param argumentos Los argumentos del programa.
 * @param type El tipo de entrada/salida.
 * @param socket El socket de la interfaz.
 * @return Un puntero a la estructura de entrada/salida agregada.
 */
t_entrada_salida *agregar_entrada_salida(t_args *argumentos, t_tipo_entrada_salida type, int socket);

/**
 * @brief Inicializa la configuración de la memoria.
 *
 * @param argumentos Los argumentos del programa.
 */
void memoria_inicializar_config(t_args *argumentos);

/**
 * @brief Imprime la configuración de la memoria.
 *
 * @param argumentos Los argumentos del programa.
 */
void memoria_imprimir_config(t_args *argumentos);

/**
 * @brief Remueve una interfaz de la lista de entradas/salidas.
 *
 * @param argumentos Los argumentos del programa.
 * @param interfaz El nombre de la interfaz a remover.
 */
void remover_interfaz(t_args *argumentos, char *interfaz);

/**
 * @brief Elimina todos los procesos de la lista de procesos.
 *
 * @param argumentos Los argumentos del programa.
 */
void eliminar_procesos(t_args *argumentos);

/**
 * @brief Elimina una lista de instrucciones.
 *
 * @param argumentos Los argumentos del programa.
 * @param lista_instrucciones La lista de instrucciones a eliminar.
 */
void eliminar_instrucciones(t_args *argumentos, t_list *lista_instrucciones);

/**
 * @brief Función para atender las entradas/salidas desde stdin.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *atender_entrada_salida_stdin(void *paquete);

/**
 * @brief Función para manejar las operaciones de entrada/salida desde stdin.
 *
 * @param argumentos Los argumentos del programa.
 * @param codigo_operacion El código de operación.
 * @param buffer El buffer recibido.
 */
void switch_case_memoria_entrada_salida_stdin(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función para manejar las operaciones de entrada/salida desde dialfs.
 *
 * @param argumentos Los argumentos del programa.
 * @param codigo_operacion El código de operación.
 * @param buffer El buffer recibido.
 */
void switch_case_memoria_entrada_salida_dialfs(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función para atender las entradas/salidas desde dialfs.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *atender_entrada_salida_dialfs(void *paquete);

/**
 * @brief Función para manejar las operaciones de entrada/salida hacia stdout.
 *
 * @param argumentos Los argumentos del programa.
 * @param codigo_operacion El código de operación.
 * @param buffer El buffer recibido.
 */
void switch_case_memoria_entrada_salida_stdout(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función para atender las entradas/salidas hacia stdout.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *atender_entrada_salida_stdout(void *paquete);

/**
 * @brief Función para esperar una entrada/salida.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *esperar_entrada_salida(void *paquete);

/**
 * @brief Función para manejar las operaciones del kernel.
 *
 * @param argumentos Los argumentos del programa.
 * @param codigo_operacion El código de operación.
 * @param buffer El buffer recibido.
 */
void switch_case_kernel(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función para esperar un paquete desde una CPU.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *esperar_cpu(void *paquete);

/**
 * @brief Función para ejecutar un hilo de memoria para una CPU.
 *
 * @param argumentos Los argumentos del programa.
 * @param socket El socket de la CPU.
 * @param modulo El módulo de la CPU.
 * @param switch_case_atencion La función de manejo de operaciones.
 */
void memoria_hilo_ejecutar(t_args *argumentos, int socket, char *modulo, t_mem_funcion_ptr switch_case_atencion);

/**
 * @brief Función para manejar las operaciones de una CPU.
 *
 * @param argumentos Los argumentos del programa.
 * @param codigo_operacion El código de operación.
 * @param buffer El buffer recibido.
 */
void switch_case_cpu(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función para esperar un paquete desde una CPU.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *esperar_cpu(void *paquete);

/**
 * @brief Función para atender un paquete desde una CPU.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *atender_cpu(void *paquete);

/**
 * @brief Función para manejar las operaciones de una CPU.
 *
 * @param argumentos Los argumentos del programa.
 * @param codigo_operacion El código de operación.
 * @param buffer El buffer recibido.
 */
void switch_case_cpu(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función para manejar las operaciones del kernel.
 *
 * @param argumentos Los argumentos del programa.
 * @param codigo_operacion El código de operación.
 * @param buffer El buffer recibido.
 */
void switch_case_kernel(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función para atender un paquete desde el kernel.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *atender_kernel(void *paquete);

/**
 * @brief Función para esperar un paquete desde el kernel.
 *
 * @param paquete El paquete recibido.
 * @return NULL.
 */
void *esperar_kernel(void *paquete);

/**
 * @brief Función para ejecutar un hilo de memoria para una entrada/salida.
 *
 * @param io_args Los argumentos del hilo de entrada/salida.
 * @param modulo El módulo de la entrada/salida.
 * @param switch_case_atencion La función de manejo de operaciones.
 */
void memoria_hilo_ejecutar_entrada_salida(t_args_hilo *io_args, char *modulo, t_mem_funcion_hilo_ptr switch_case_atencion);

/**
 * @brief Inicializa los argumentos de la memoria.
 *
 * @param argumentos Los argumentos del programa.
 */
void memoria_inicializar_argumentos(t_args *argumentos);

/**
 * @brief Inicializa los hilos de la memoria.
 *
 * @param argumentos Los argumentos del programa.
 */
void memoria_inicializar_hilos(t_args *argumentos);

/**
 * @brief Inicializa la memoria.
 *
 * @param argumentos Los argumentos del programa.
 */
void memoria_inicializar(t_args *argumentos);

/**
 * @brief Finaliza la memoria.
 *
 * @param argumentos Los argumentos del programa.
 */
void memoria_finalizar(t_args *argumentos);

/**
 * @brief Inicializa el módulo de memoria.
 *
 * @param argumentos Los argumentos del programa.
 */
void inicializar_modulo(t_args *argumentos);

/**
 * @brief Inicializa el logger de memoria.
 *
 * @param argumentos Los argumentos del programa.
 * @param nivel El nivel de log.
 */
void inicializar_logger(t_args *argumentos, t_log_level nivel);

/**
 * @brief Inicializa la memoria.
 *
 * @param args Los argumentos del programa.
 * @param nivel El nivel de log.
 * @param argc La cantidad de argumentos.
 * @param argv Los argumentos.
 */
void inicializar(t_args *args, t_log_level nivel, int argc, char *argv[]);

/**
 * @brief Crea una tabla de páginas en memoria.
 *
 * @param argumentos Los argumentos del programa.
 * @param pid El ID del proceso.
 * @param cantidad_paginas La cantidad de páginas.
 */
void tabla_paginas_inicializar(t_args *argumentos, t_proceso *proceso);

/**
 * @brief Destruye una tabla de páginas en memoria.
 *
 * @param args Los argumentos del programa.
 * @param pid El ID del proceso.
 */
void tabla_paginas_liberar(t_args *argumentos, t_proceso *proceso);

/**
 * @brief Accede a una tabla de páginas en memoria.
 *
 * @param argumentos Los argumentos del programa.
 * @param proceso El proceso.
 * @param numero_pagina El número de página.
 * @return El marco de la página.
 */
int tabla_paginas_acceder_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina);

/**
 * Asigna un número de marco a una página en la tabla de páginas de un proceso.
 *
 * @param argumentos Puntero a la estructura de argumentos.
 * @param proceso El proceso.
 * @param numero_pagina Número de página a asignar.
 * @param frame Número de marco a asignar.
 */
t_pagina *tabla_paginas_asignar_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina, uint32_t frame);

void agregar_identificador(t_args_hilo *argumentos, char *identificador);
void avisar_rechazo_identificador_memoria(int socket);
void agregar_identificador_rechazado(t_args_hilo *argumentos, char *identificador);

// Internas de memoria
void espacio_usuario_inicializar(t_args *args);
void espacio_usuario_liberar(t_args *args);
int espacio_usuario_escribir_dato(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, void *dato, size_t tamano);
int espacio_usuario_liberar_dato(t_args *args, uint32_t direccion_fisica, size_t tamano);
int espacio_usuario_escribir_char(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, const char *cadena);
/**
 * Escribe un valor de tipo uint32_t en la dirección física especificada.
 *
 * @param args Puntero a la estructura de argumentos.
 * @param pagina Puntero a la página donde se escribirá el valor.
 * @param direccion_fisica Dirección física donde se escribirá el valor.
 * @param valor Valor a escribir.
 */
void espacio_usuario_escribir_uint32_t(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, uint32_t valor);

/**
 * Lee un dato de memoria de la dirección física especificada y lo guarda en el destino proporcionado.
 *
 * @param args Puntero a la estructura de argumentos.
 * @param direccion_fisica Dirección física de donde se leerá el dato.
 * @param destino Puntero al destino donde se guardará el dato leído.
 * @param tamano Tamaño del dato a leer.
 */
int espacio_usuario_leer_dato(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, void *destino, size_t tamano);

/**
 * Lee un valor de tipo uint32_t de la dirección física especificada.
 *
 * @param args Puntero a la estructura de argumentos.
 * @param direccion_fisica Dirección física de donde se leerá el valor.
 * @return Valor de tipo uint32_t leído.
 */
uint32_t espacio_usuario_leer_uint32(t_args *args, t_proceso *proceso, uint32_t direccion_fisica);

/**
 * Lee una cadena de caracteres de la dirección física especificada.
 *
 * @param args Puntero a la estructura de argumentos.
 * @param direccion_fisica Dirección física de donde se leerá la cadena de caracteres.
 * @param tamano_max Tamaño máximo de la cadena de caracteres a leer.
 * @return Puntero a la cadena de caracteres leída.
 */
char *espacio_usuario_leer_char(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, size_t tamano_max);

/**
 * Obtiene el próximo frame disponible para asignar memoria de tamaño especificado.
 *
 * @param args Puntero a la estructura de argumentos.
 * @param tamano Tamaño de memoria a asignar.
 * @return Próximo frame disponible.
 */
int espacio_usuario_proximo_frame(t_args *args, size_t tamano);

int tabla_paginas_liberar_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina);

int tabla_paginas_frames_ocupados(t_args *args, t_proceso *proceso);

int tabla_paginas_resize(t_args *args, t_proceso *proceso, uint32_t bytes_nuevos);

int tabla_paginas_bytes_ocupados(t_args *args, t_proceso *proceso);

int espacio_usuario_bytes_disponibles(t_args *args);

uint8_t espacio_usuario_leer_uint8(t_args *args, t_proceso *proceso, uint8_t direccion_fisica);

void espacio_usuario_escribir_uint8_t(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, uint8_t valor);

uint32_t espacio_usuario_obtener_frame(uint32_t direccion_fisica, uint32_t tamPagina);

#endif // MEMORIA_H