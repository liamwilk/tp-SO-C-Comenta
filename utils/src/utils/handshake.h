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
 * @NAME: handshake
 * @DESC: Realiza el handshake con el servidor
 * @PARAMS:
 *  - t_log* logger: logger del modulo
 *  - int conexion: socket de conexion
 *  - uint32_t envio: mensaje a enviar
 *  - char* modulo: nombre del modulo
 * @RETURN: uint32_t
 */

uint32_t handshake(t_log *logger, int conexion, uint32_t envio, char *modulo);

/**
 * @NAME: esperar_handshake
 * @DESC: Espera el handshake del cliente
 * @PARAMS:
 *  - int nuevoSocket: socket de conexion
 * @RETURN: int
 */

int esperar_handshake(int nuevoSocket);

#endif /* HANDSHAKE_H_ */
