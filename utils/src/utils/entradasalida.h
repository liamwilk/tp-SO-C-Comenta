#ifndef ENTRADASALIDA_H
#define ENTRADASALIDA_H

#include <stdio.h>
#include <string.h>
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
#include <commons/process.h>
#include <utils/template.h>
#include <time.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <commons/bitarray.h>

typedef struct
{
    int socket_memoria_stdin;
    int socket_memoria_stdout;
    int socket_memoria_dialfs;
    int socket_kernel_generic;
    int socket_kernel_stdin;
    int socket_kernel_stdout;
    int socket_kernel_dialfs;
} t_sockets;

typedef struct
{
    pthread_t thread_conectar_kernel_generic, thread_atender_kernel_generic;
    pthread_t thread_conectar_memoria_stdin, thread_atender_memoria_stdin;
    pthread_t thread_conectar_kernel_stdin, thread_atender_kernel_stdin;
    pthread_t thread_conectar_memoria_stdout, thread_atender_memoria_stdout;
    pthread_t thread_conectar_kernel_stdout, thread_atender_kernel_stdout;
    pthread_t thread_conectar_memoria_dialfs, thread_atender_memoria_dialfs;
    pthread_t thread_conectar_kernel_dialfs, thread_atender_kernel_dialfs;
} t_threads;

typedef struct
{
    timer_t timer;
    struct sigevent sev;
    struct itimerspec its;
} t_timer;

typedef enum
{
    GEN,
    STDOUT,
    STDIN,
    DIALFS
} t_interfaz;

typedef struct
{
    int blockSize;
    int blockCount;
    int retrasoCompactacion;
    char *pathBaseDialFs;
    char *path_bloques;
    char *path_bitmap;
    void *archivo_bloques;
    char *archivo_bitmap;
    FILE *archivo_metadata;
    int tamanio_archivo;
    int tamanio_bitmap;
    t_dictionary *archivos;
    t_bitarray *bitarray;
} t_dial_fs;

typedef struct
{
    t_timer timer;
    t_log *logger;
    t_config *config;
    t_sockets sockets;
    t_threads threads;
    char *ipMemoria;
    int puertoMemoria;
    char *ipKernel;
    int puertoKernel;
    char *tipoInterfaz;
    int tiempoUnidadDeTrabajo;
    char *identificador;
    int pid;
    double duracion;
    int unidades;
    t_dial_fs dial_fs;
} t_io;

typedef void (*t_io_funcion_hilo_ptr)(t_io *, t_op_code, t_buffer *);

typedef struct
{
    t_io *args;
} timer_args_io_t;

typedef struct t_fcb
{
    uint32_t total_size;
    uint32_t inicio;
    uint32_t fin_bloque;
    t_config *metadata; // Esto es para ir guardando los cambios en el archivo de metadata
} t_fcb;

void *conectar_kernel_stdin(void *args);
void *conectar_memoria_stdin(void *args);

void *conectar_kernel_stdout(void *args);
void *conectar_memoria_stdout(void *args);

void *atender_kernel_stdout(void *args);
void *atender_memoria_stdout(void *args);

void *conectar_kernel_generic(void *args);
void *atender_kernel_generic(void *args);

void *atender_kernel_stdin(void *args);
void *atender_memoria_stdin(void *args);

void *conectar_kernel_dialfs(void *args);
void *conectar_memoria_dialfs(void *args);

void *atender_kernel_dialfs(void *args);
void *atender_memoria_dialfs(void *args);

t_interfaz determinar_interfaz(t_config *config);
void interfaz_generic_inicializar_config(t_io *io);
void interfaz_stdin_inicializar_config(t_io *io);
void interfaz_stdout_inicializar_config(t_io *io);
void interfaz_dialfs_inicializar_config(t_io *io);

void interfaz_generic_imprimir_log(t_io *io);
void interfaz_stdin_imprimir_log(t_io *io);
void interfaz_stdout_imprimir_log(t_io *io);
void interfaz_dialfs_imprimir_log(t_io *io);

void interfaz_generic(t_io *io);
void interfaz_stdin(t_io *io);
void interfaz_stdout(t_io *io);
void interfaz_dialfs(t_io *io);

int verificar_argumentos(int argc);
void interfaz_identificar(t_op_code opcode, char *identificador, int socket);
void inicializar_argumentos(t_io *args, char *argv[]);
void inicializar_interfaz(t_io *io);
void finalizar_interfaz(t_io *args);
int inicializar_modulo_interfaz(t_io *args, int argc, char *argv[], timer_args_io_t *temporizador);

void switch_case_kernel_generic(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_dialfs(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_stdin(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_memoria_stdin(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_memoria_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_memoria_dialfs(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);

void hilo_ejecutar_interfaz(t_io *args, int *socket, char *modulo, t_io_funcion_hilo_ptr switch_case_atencion);

void interfaz_iniciar_temporizador(t_io *args, double duracion);
void interfaz_inicializar_temporizador(t_io *args, timer_args_io_t *temporizador);
void interfaz_manejador_temporizador(union sigval arg);
void interfaz_interrumpir_temporizador(t_io *args);

char *leer_input_usuario(uint32_t size_input);

void bloques_mapear(t_io *args);
void bloques_desmapear(t_io *args);

void bitmap_mapear(t_io *args);
void bitmap_desmapear(t_io *args);
void metadata_inicializar(t_io *args);

/**
 * Devuelve el índice de un bloque libre en el sistema de archivos.
 *
 * @param args La estructura de entrada/salida que contiene la información necesaria.
 * @return El índice de un bloque libre, o -1 si no hay bloques libres disponibles.
 */
int fs_buscar_bloque_libre(t_io *args);

/**
 * Crea un archivo en el sistema de archivos.
 *
 * @param args Los argumentos de E/S.
 * @param nombre El nombre del archivo a crear.
 */
void fs_archivo_crear(t_io *args, char *nombre, int indice_bloque_libre);

#endif // ENTRADASALIDA_H