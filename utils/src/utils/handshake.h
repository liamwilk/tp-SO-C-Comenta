#ifndef HANDSHAKE_H_
#define HANDSHAKE_H_

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
#include <utils/modulos.h>

/**
 * @fn handshake
 * @brief Realiza el handshake con el servidor
 * @param t_log* logger: logger del modulo
 * @param conexion: socket de conexion
 * @param envio: mensaje a enviar
 * @param modulo: nombre del modulo
 * @return uint32_t
 */
uint32_t handshake(t_log *logger_info, t_log *logger_error, int conexion, uint32_t envio, char *modulo);

/**
 * @fn    esperar_handshake
 * @brief Espera el handshake del cliente
 * @param nuevoSocket socket de conexion
 * @return int
 */
int esperar_handshake(int nuevoSocket);

#endif /* HANDSHAKE_H_ */
