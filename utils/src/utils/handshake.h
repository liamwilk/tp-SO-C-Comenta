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
    SERVIDOR,
    CLIENTE,
    MEMORIA_CPU,
    MEMORIA_KERNEL,
    CPU_DISPATCH_KERNEL,
    CPU_INTERRUPT_KERNEL,
    CPU_ENTRADA_SALIDA,
    KERNEL_ENTRADA_SALIDA_GENERIC,
    KERNEL_ENTRADA_SALIDA_STDIN,
    KERNEL_ENTRADA_SALIDA_STDOUT,
    KERNEL_ENTRADA_SALIDA_DIALFS,
    MEMORIA_ENTRADA_SALIDA_STDIN,
    MEMORIA_ENTRADA_SALIDA_STDOUT,
    MEMORIA_ENTRADA_SALIDA_DIALFS,
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
 * @fn conexion_handshake_recibir
 * @brief Espera el handshake del cliente
 * @param t_log* logger: logger del modulo
 * @param socket_cliente: El socket del cliente
 * @param codigo_a_recibir: Codigo de handshake a recibir del cliente
 * @param modulo: Nombre del modulo que se quiere conectar con el servidor
 * @return t_handshake
 */
t_handshake conexion_handshake_recibir(t_log *logger, int socket_cliente, t_handshake codigo_esperado, char *modulo);

/**
 * Espera el handshake de entrada y salida de un cliente.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param socket_cliente El socket del cliente.
 * @return El handshake recibido.
 */
t_handshake esperar_handshake_entrada_salida(t_log *logger, int socket_cliente);

#endif /* HANDSHAKE_H_ */
