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

typedef struct t_io
{
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
    char *pathBaseDialFs;
    int blockSize;
    int blockCount;
    int retrasoCompactacion;
    char *identificador;
} t_io;

typedef enum
{
    GEN,
    STDOUT,
    STDIN,
    DIALFS
} t_interfaz;

typedef void (*t_io_funcion_hilo_ptr)(t_io *, t_op_code, t_buffer *);

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
int inicializar_modulo_interfaz(t_io *args, int argc, char *argv[]);

void switch_case_kernel_generic(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_dialfs(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_stdin(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_memoria_stdin(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_memoria_stdout(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_memoria_dialfs(t_io *args, t_op_code codigo_operacion, t_buffer *buffer);

void hilo_ejecutar_interfaz(t_io *args, int *socket, char *modulo, t_io_funcion_hilo_ptr switch_case_atencion);

#endif // ENTRADASALIDA_H