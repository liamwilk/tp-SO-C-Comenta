#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<pthread.h>
#include<limits.h>
#include <utils/handshake.h>
#include <utils/modulos.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>
#include <commons/collections/list.h>
#include "serial.h"
#include <utils/template.h>


typedef enum
{
    STDIN,
    STDOUT,
    DIALFS
} t_tipo_entrada_salida;

typedef struct
{
	int ocupado;
	int pid;
	int orden;
    int socket;
    char *interfaz;
    t_tipo_entrada_salida tipo;
} t_entrada_salida;

typedef struct
{
	uint32_t pid;	
	uint32_t pc;
    t_list* instrucciones;
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
	t_log* logger;
	t_config* config;
	t_list* lista_procesos;
	t_list* lista_entrada_salida;
	t_dictionary* diccionario_procesos;
	t_dictionary* diccionario_entrada_salida;
	t_sockets sockets;
	t_threads threads;

	int tamMemoria;
	int tamPagina;
	int retardoRespuesta;
	int puertoEscucha;
	char* pathInstrucciones;
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

t_args args;


void* esperar_cpu();
void* esperar_kernel();
void* esperar_entrada_salida();

void* atender_cpu();
void* atender_kernel();
void* atender_entrada_salida_stdin(void*);
void* atender_entrada_salida_stdout(void*);
void* atender_entrada_salida_dialfs(void*);

/**
 * @fn    *armar_ruta
 * @brief Compone a ruta1 con ruta2
 * @param ruta1 Ruta base
 * @param ruta2 Ruta a concatenar
 */
char *armar_ruta(char* ruta1, char* ruta2);

/**
 * @fn    *leer_instrucciones
 * @brief Lee las instrucciones de un archivo y retorna una lista con las instrucciones leidas
 * @param path_instrucciones Ruta absoluta del archivo de instrucciones
 */
t_list *leer_instrucciones(char *path_instrucciones, uint32_t pid);

/**
 * @fn    *buscar_proceso
 * @brief Dado un pid, retorna el proceso asociado de la lista global de procesos
 * @param pid El pid del proceso a buscar
 */
t_proceso *buscar_proceso(uint32_t pid);

/**
 * @fn    *eliminar_procesos
 * @brief Dado una lista de procesos, los elimina a todos liberando la memoria inclusive.
 * @param lista_procesos La lista de t_proceso a eliminar
 */
void eliminar_procesos(t_list *lista_procesos);

/**
 * @fn    *eliminar_instrucciones
 * @brief Dado una lista de instrucciones, las elimina a todas liberando la memoria inclusive.
 * @param lista_instrucciones La lista de t_memoria_cpu_instruccion a eliminar
 */
void eliminar_instrucciones(t_list *lista_instrucciones);

void switch_case_kernel(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_cpu(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer);

void switch_case_entrada_salida_stdin(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_entrada_salida_stdout(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_entrada_salida_dialfs(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);
void memoria_inicializar();
void memoria_log();

char *agregar_entrada_salida(t_args *argumentos, t_tipo_entrada_salida type, int socket);

t_entrada_salida *agregar_interfaz(t_args *argumentos, t_tipo_entrada_salida tipo, int socket);

void remover_interfaz(t_args *argumentos, char *interfaz);

t_entrada_salida *buscar_interfaz(t_args *argumentos, char *interfaz);

#endif /* MAIN_H_ */