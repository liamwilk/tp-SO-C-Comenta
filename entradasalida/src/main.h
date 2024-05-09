#ifndef MAIN_H_
#define MAIN_H_

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
#include "common.h"
#include "init.h"
#include <commons/process.h>
#include <utils/template.h>

t_log* logger;
t_config* config;
t_entradasalida entradasalida;

pthread_t thread_conectar_kernel_generic,thread_atender_kernel_generic;
pthread_t thread_conectar_memoria_stdin,thread_atender_memoria_stdin;
pthread_t thread_conectar_kernel_stdin,thread_atender_kernel_stdin;

void* conectar_kernel_stdin();
void* conectar_memoria_stdin();

void* conectar_kernel_generic();
void* atender_kernel_generic();

void* atender_kernel_stdin();
void* atender_memoria_stdin();


int socket_memoria;
int socket_kernel_generic;
int socket_kernel_stdin;
int socket_kernel_stdout;
int socket_kernel_dialfs;
char *nombre_modulo;

void procesar_entradasalida_generic();	
void procesar_entradasalida_stdin();
void procesar_entradasalida_stdout();
void procesar_entradasalida_dialfs();	

void* atender_entrada_salida_generic();

#endif /* MAIN_H_ */