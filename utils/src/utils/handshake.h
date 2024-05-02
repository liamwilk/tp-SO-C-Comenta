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

typedef enum
{ // Nomenclatura: SERVIDOR_CLIENTE
    CORRECTO,
    ERROR,
    MEMORIA_CPU,
    MEMORIA_KERNEL,
    MEMORIA_ENTRADA_SALIDA,
    CPU_DISPATCH_KERNEL,
    CPU_INTERRUPT_KERNEL,
    CPU_ENTRADA_SALIDA,
    KERNEL_ENTRADA_SALIDA,
    ENTRADA_SALIDA_KERNEL
} t_handshake;

/**
 * @fn crear_handshake
 * @brief Realiza el handshake con el servidor
 * @param t_log* logger: logger del modulo
 * @param socket_servidor: El socket del servidor
 * @param codigo_a_recibir: Codigo de handshake a recibir en el servidor
 * @param modulo: Nombre del modulo que recibe el handshake
 * @return t_handshake
 */
t_handshake crear_handshake(t_log *logger, int socket_servidor, t_handshake codigo_a_recibir, char *modulo);

/**
 * @fn esperar_handshake
 * @brief Espera el handshake del cliente
 * @param t_log* logger: logger del modulo
 * @param socket_cliente: El socket del cliente
 * @param codigo_a_recibir: Codigo de handshake a recibir del cliente
 * @param modulo: Nombre del modulo que se quiere conectar con el servidor
 * @return t_handshake
 */
t_handshake esperar_handshake(t_log *logger, int socket_cliente, t_handshake codigo_esperado, char *modulo);

#endif /* HANDSHAKE_H_ */
