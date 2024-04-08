#ifndef UTILS_CONEXIONES_H_
#define UTILS_CONEXIONES_H_

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>

int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto);
int esperar_cliente(t_log *logger, const char *name, int socket_servidor);
int crear_conexion(t_log *logger, const char *server_name, char *ip, char *puerto);
void liberar_conexion(int *socket_cliente);

#endif
