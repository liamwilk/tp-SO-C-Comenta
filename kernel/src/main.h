#ifndef MAIN_H_
#define MAIN_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<pthread.h>
#include<limits.h>
#include <utils/modulos.h>
#include <utils/handshake.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>

t_kernel kernel;
t_log* logger;
t_config* config;

void terminar_programa(int, t_log*, t_config*);
int esperar_cliente(t_log*, int);
int recibir_operacion(int);
void *recibir_buffer(int*,int);

void* conectar_memoria();
void* conectar_cpu_dispatch();
void* conectar_cpu_interrupt();

void* atender_memoria();
void* atender_cpu_dispatch();
void* atender_cpu_interrupt();

void* conectar_io();
void* atender_io(void*);

void* atender_consola();

// Definición del array de strings
char* FuncionesStrings[] = {
    "EJECUTAR_SCRIPT",
    "INICIAR_PROCESO",
    "FINALIZAR_PROCESO",
    "DETENER_PLANIFICACION",
    "INICIAR_PLANIFICACION",
    "MULTIPROGRAMACION",
    "PROCESO_ESTADO",
    "FINALIZAR"
};

typedef enum{
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    MULTIPROGRAMACION,
    PROCESO_ESTADO,
    FINALIZAR,
    NUM_FUNCIONES // siempre mantener este al final para saber el tamaño del enum
} funciones;


funciones obtener_funcion(char* funcion);

pthread_t thread_conectar_io,thread_conectar_memoria,thread_conectar_cpu_dispatch,thread_conectar_cpu_interrupt,thread_atender_memoria,thread_atender_cpu_dispatch,thread_atender_cpu_interrupt,thread_atender_consola;

int socket_server_kernel;
int socket_memoria;
int socket_cpu_interrupt;
int socket_cpu_dispatch;

// Cuando vale 0, es porque el Kernel ordeno a todos los modulos apagarse
int kernel_orden_apagado = 1;

#endif /* MAIN_H_ */