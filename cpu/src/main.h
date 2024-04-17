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



pthread_t dispatch,interrupt;

t_cpu cpu;

t_log* logger;
t_config* config;



// Esto es para abrir sockets//////
void* atender_kernel_dispatch();///
void* atender_kernel_interrupt();//
///////////////////////////////////

int socket_kernel_dispatch;
int socket_kernel_interrupt;
int socket_memoria_dispatch;
int socket_memoria_interrupt;
int socket_server_dispatch;
int socket_server_interrupt;

#endif /* MAIN_H_ */