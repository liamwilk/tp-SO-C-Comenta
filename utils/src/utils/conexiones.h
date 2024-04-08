#ifndef UTILS_CONEXIONES_H_
#define UTILS_CONEXIONES_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
/**
* @fn    decir_hola
* @brief Imprime un saludo al nombre que se pase por parámetro por consola.
*/
int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);

int esperar_cliente(t_log* logger, const char* name, int socket_servidor);

int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto);

void liberar_conexion(int* socket_cliente);

#endif
