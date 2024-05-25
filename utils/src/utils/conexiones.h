#ifndef UTILS_CONEXIONES_H_
#define UTILS_CONEXIONES_H_

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>

int iniciar_servidor(t_log *logger, int puerto);
int conexion_socket_recibir(int socket_servidor);
int crear_conexion(t_log *logger, char *ip, int puerto);
void liberar_conexion(int *socket_cliente);

#endif