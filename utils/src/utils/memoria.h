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
    uint32_t pid;
    uint32_t pc;
    t_list *instrucciones;
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
{
    void *espacio_usuario;
    t_bitarray *bitmap;
    char *bitmap_data;
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

typedef void (*t_mem_funcion_hilo_ptr)(t_args_hilo *, char *, t_op_code, t_buffer *);
typedef void (*t_mem_funcion_ptr)(t_args *, t_op_code, t_buffer *);

t_entrada_salida *agregar_interfaz(t_args *argumentos, t_tipo_entrada_salida tipo, int socket);
t_entrada_salida *buscar_interfaz(t_args *argumentos, char *interfaz);
t_entrada_salida *agregar_entrada_salida(t_args *argumentos, t_tipo_entrada_salida type, int socket);
t_list *leer_instrucciones(t_args *argumentos, char *path_instrucciones, uint32_t pid);
t_proceso *buscar_proceso(t_args *argumentos, uint32_t pid);
char *armar_ruta(char *ruta1, char *ruta2);
void memoria_inicializar_config(t_args *argumentos);
void memoria_imprimir_config(t_args *argumentos);
void remover_interfaz(t_args *, char *);
void eliminar_procesos(t_args *argumentos);
void eliminar_instrucciones(t_args *argumentos, t_list *lista_instrucciones);
void *atender_entrada_salida_stdin(void *);
void switch_case_memoria_entrada_salida_stdin(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_memoria_entrada_salida_dialfs(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void *atender_entrada_salida_dialfs(void *);
void switch_case_memoria_entrada_salida_stdout(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void *atender_entrada_salida_stdout(void *);
void *esperar_entrada_salida(void *paquete);
void *atender_kernel();
void switch_case_kernel(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);
void *esperar_cpu(void *paquete);
void memoria_hilo_ejecutar(t_args *argumentos, int socket, char *modulo, t_mem_funcion_ptr switch_case_atencion);
void switch_case_cpu(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);
void *esperar_cpu(void *paquete);
void *atender_cpu(void *paquete);
void switch_case_cpu(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer);
void *atender_kernel(void *paquete);
void *esperar_kernel(void *paquete);
void memoria_hilo_ejecutar_entrada_salida(t_args_hilo *io_args, char *modulo, t_mem_funcion_hilo_ptr switch_case_atencion);
void memoria_inicializar_argumentos(t_args *argumentos);
void memoria_inicializar_hilos(t_args *argumentos);
void memoria_inicializar(t_args *argumentos);
void memoria_finalizar(t_args *argumentos);
void inicializar_modulo(t_args *argumentos);
void inicializar_logger(t_args *argumentos, t_log_level nivel);
void inicializar(t_args *args, t_log_level nivel, int argc, char *argv[]);
void agregar_identificador(t_args_hilo *argumentos, char *identificador);
void avisar_rechazo_identificador_memoria(int socket);
void agregar_identificador_rechazado(t_args_hilo *argumentos, char *identificador);

// Internas de memoria
void espacio_usuario_inicializar_contiguo(t_args *args);
void espacio_usuario_liberar_contiguo(t_args *args);
void espacio_usuario_inicializar_bitmap(t_args *args);
void espacio_usuario_liberar_bitmap(t_args *args);
void marcar_frame_usado(t_args *args, t_bitarray *bitmap, uint32_t frame);
void liberar_frame(t_bitarray *bitmap, uint32_t frame);
bool frame_esta_libre(t_bitarray *bitmap, uint32_t frame);
void espacio_usuario_escribir(t_args *args, uint32_t direccion_fisica, void *dato, size_t tamano);
void espacio_usuario_liberar_frames(t_args *args, uint32_t direccion_fisica, size_t tamano);
void escribir_int(t_args *args, uint32_t direccion_fisica, int valor);
void escribir_float(t_args *args, uint32_t direccion_fisica, float valor);
void escribir_char(t_args *args, uint32_t direccion_fisica, const char *cadena);
void escribir_generic(t_args *args, uint32_t direccion_fisica, void *estructura, size_t tamano_estructura);
void escribir_uint32(t_args *args, uint32_t direccion_fisica, uint32_t valor);
void espacio_usuario_leer(t_args *args, uint32_t direccion_fisica, void *destino, size_t tamano);
uint32_t leer_uint32(t_args *args, uint32_t direccion_fisica);
int leer_int(t_args *args, uint32_t direccion_fisica);
float leer_float(t_args *args, uint32_t direccion_fisica);
void leer_char(t_args *args, uint32_t direccion_fisica, char *destino, size_t tamano_max);
void leer_generic(t_args *args, uint32_t direccion_fisica, void *estructura, size_t tamano_estructura);

#endif // MEMORIA_H