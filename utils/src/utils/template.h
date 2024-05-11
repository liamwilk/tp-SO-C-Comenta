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

typedef struct hilos_args
{
    t_log *logger;
    t_kernel *kernel;
    t_diagrama_estados *estados;
    int kernel_orden_apagado;
} hilos_args;

typedef struct hilos_io_args
{
    hilos_args *args;
    int socket;
} hilos_io_args;

typedef void (*t_funcion_ptr)(t_log *, t_op_code, t_buffer *);
typedef void (*t_funcion_kernel_ptr)(t_log *, t_op_code, hilos_args *, t_buffer *);

void hilo_ejecutar(t_log *logger, int socket, char *modulo_emisor, t_funcion_ptr switch_case_atencion);
void hilo_ejecutar_kernel(int socket, hilos_args *args, char *modulo, t_funcion_kernel_ptr switch_case_atencion);

void conexion_crear(t_log *logger, char *ip, int puerto, int *socket_modulo, char *modulo, t_handshake codigo_esperado);
void conexion_recibir(t_log *logger, int socket_servidor, int *socket_modulo, char *modulo, t_handshake codigo_esperado);

#endif /* TEMPLATE_H */