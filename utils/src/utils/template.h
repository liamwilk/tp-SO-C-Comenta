#ifndef TEMPLATE_H
#define TEMPLATE_H
#include <stdio.h>
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
#include <utils/handshake.h>
#include <utils/modulos.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>
#include <commons/collections/list.h>
#include <utils/kernel.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/types.h>
#include <pwd.h>
#include "kernel.h"

typedef void (*t_funcion_ptr)(t_log *, t_op_code, t_buffer *);

/**
 * Ejecuta un hilo de ejecución que realiza una determinada tarea.
 *
 * @param logger El logger utilizado para registrar eventos.
 * @param socket El socket utilizado para la comunicación.
 * @param modulo_emisor El módulo que envía la solicitud.
 * @param switch_case_atencion La función que se ejecutará en el hilo.
 */
void hilo_ejecutar(t_log *logger, int socket, char *modulo_emisor, t_funcion_ptr switch_case_atencion);

/**
 * Crea una conexión con un servidor remoto.
 *
 * @param logger El logger utilizado para registrar eventos.
 * @param ip La dirección IP del servidor remoto.
 * @param puerto El puerto del servidor remoto.
 * @param socket_modulo El socket utilizado para la comunicación con el servidor.
 * @param modulo El módulo que realiza la conexión.
 * @param codigo_esperado El código de handshake esperado por el servidor.
 */
void conexion_crear(t_log *logger, char *ip, int puerto, int *socket_modulo, char *modulo, t_handshake codigo_esperado);

/**
 * Recibe una conexión entrante de un cliente remoto.
 *
 * @param logger El logger utilizado para registrar eventos.
 * @param socket_servidor El socket del servidor que acepta la conexión.
 * @param socket_modulo El socket utilizado para la comunicación con el cliente.
 * @param modulo El módulo que acepta la conexión.
 * @param codigo_esperado El código de handshake esperado por el cliente.
 */
void conexion_recibir(t_log *logger, int socket_servidor, int *socket_modulo, char *modulo, t_handshake codigo_esperado);

#endif /* TEMPLATE_H */