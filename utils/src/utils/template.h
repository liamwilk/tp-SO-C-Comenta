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

void hilo_ejecutar(t_log *logger, int socket, char *modulo_emisor, t_funcion_ptr switch_case_atencion);

void conexion_crear(t_log *logger, char *ip, int puerto, int *socket_modulo, char *modulo, t_handshake codigo_esperado);
void conexion_recibir(t_log *logger, int socket_servidor, int *socket_modulo, char *modulo, t_handshake codigo_esperado);

#endif /* TEMPLATE_H */