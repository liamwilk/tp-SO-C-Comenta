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

/**
 * @brief Inicia un servidor en el puerto especificado.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param puerto Puerto en el que se desea iniciar el servidor.
 * @return El descriptor de archivo del socket del servidor, o -1 si ocurrió un error.
 */
int iniciar_servidor(t_log *logger, int puerto);

/**
 * @brief Acepta una conexión entrante en el socket del servidor.
 *
 * @param socket_servidor Descriptor de archivo del socket del servidor.
 * @return El descriptor de archivo del socket del cliente, o -1 si ocurrió un error.
 */
int conexion_socket_recibir(int socket_servidor);

/**
 * @brief Crea una conexión con el servidor en la dirección IP y puerto especificados.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param ip Dirección IP del servidor.
 * @param puerto Puerto del servidor.
 * @return El descriptor de archivo del socket de la conexión, o -1 si ocurrió un error.
 */
int crear_conexion(t_log *logger, char *ip, int puerto);

/**
 * @brief Libera los recursos asociados a la conexión del cliente.
 *
 * @param socket_cliente Puntero al descriptor de archivo del socket del cliente.
 */
void liberar_conexion(int *socket_cliente);

#endif