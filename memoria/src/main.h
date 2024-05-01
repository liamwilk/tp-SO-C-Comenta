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
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

// Biblioteca de serializacion exclusiva para Memoria
#include "serial.h"

typedef struct
{
	uint32_t pid;	
	uint32_t pc;
    t_list* instrucciones;
} t_proceso;

t_log* logger;
t_config* config;
t_memoria memoria;
t_list* lista_procesos;
t_dictionary* diccionario_procesos;

void* esperar_cpu();
void* esperar_kernel();
void* esperar_entrada_salida();

void* atender_cpu();
void* atender_kernel();
void* atender_entrada_salida(void*);

/**
 * @fn    *armar_ruta
 * @brief Compone a ruta1 con ruta2
 * @param ruta1 Ruta base
 * @param ruta2 Ruta a concatenar
 */
char *armar_ruta(char* ruta1, char* ruta2);

/**
 * @fn    *memoria_leer_instrucciones
 * @brief Lee las instrucciones de un archivo y retorna una lista con las instrucciones leidas
 * @param path_instrucciones Ruta absoluta del archivo de instrucciones
 */
t_list *memoria_leer_instrucciones(char *path_instrucciones);


/**
 * @fn    *obtener_proceso
 * @brief Dado un pid, retorna el proceso asociado de la lista global de procesos
 * @param pid El pid del proceso a buscar
 */
t_proceso *obtener_proceso(uint32_t pid);

pthread_t thread_atender_cpu,thread_atender_kernel,thread_atender_entrada_salida,thread_esperar_cpu,thread_conectar_kernel;

int socket_cpu;
int socket_kernel;
int socket_server_memoria;

// Cuando vale 0, es porque el Kernel ordeno a todos los modulos apagarse
int kernel_orden_apagado = 1;

#endif /* MAIN_H_ */