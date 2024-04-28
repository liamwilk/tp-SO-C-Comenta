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

t_log* logger;
t_config* config;
t_memoria memoria;
t_queue* instrucciones_cpu;

void* conectar_cpu();
void* conectar_kernel();
void* conectar_io();

void* atender_cpu();
void* atender_kernel();
void* atender_io(void*);

char* armar_ruta(char* ruta1, char* ruta2);
int encolar_instrucciones(t_queue** cola,char* pathInstrucciones);
t_memoria_cpu_instruccion* desencolar_instruccion(t_queue* instrucciones);

// Manejo del t_kernel_memoria
t_kernel_memoria* deserializar_t_kernel_memoria(t_buffer* buffer);

pthread_t thread_atender_cpu,thread_atender_kernel,thread_atender_io,thread_conectar_cpu,thread_conectar_kernel;

int socket_cpu;
int socket_kernel;
int socket_server_memoria;

// Cuando vale 0, es porque el Kernel ordeno a todos los modulos apagarse
int kernel_orden_apagado = 1;

#endif /* MAIN_H_ */