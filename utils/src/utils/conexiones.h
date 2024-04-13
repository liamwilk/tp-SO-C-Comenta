#ifndef UTILS_CONEXIONES_H_
#define UTILS_CONEXIONES_H_

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>

// Threads
pthread_t kernelThread;

// Sockets
int serverMemoria;
int socketKernel;

uint32_t respuesta;

int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto);
int esperar_cliente(t_log *logger, const char *name, int socket_servidor);
int crear_conexion(t_log *logger, char *ip, int puerto, char *modulo);
void liberar_conexion(int socket_cliente);

#endif
