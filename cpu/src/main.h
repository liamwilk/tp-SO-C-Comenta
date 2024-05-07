#ifndef MAIN_H_
#define MAIN_H_

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
#include <utils/procesos.h>
#include <utils/template.h>

/*  Registros utilizados por la cpu */

// Registros auxiliares de 1 byte
uint8_t ax, bx, cx, dx;

// Registros auxiliares de 4 bytes
uint32_t eax, ebx, ecx, edx;

// Contador de programa
uint32_t pc;

// Registros para el copiado de strings
uint32_t si, di;

// Process ID
uint32_t pid;

t_cpu cpu;
t_log *logger;
t_config *config;

void *atender_kernel_dispatch();
void *atender_kernel_interrupt();
void *atender_memoria();

void *esperar_kernel_dispatch();
void *esperar_kernel_interrupt();
void *conectar_memoria();

t_memoria_cpu_instruccion *deserializar_t_memoria_cpu_instruccion(t_buffer *buffer);

t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer);

int socket_kernel_dispatch;
int socket_kernel_interrupt;
int socket_memoria;
int socket_server_dispatch;
int socket_server_interrupt;

pthread_t thread_atender_kernel_dispatch, thread_atender_kernel_interrupt, thread_conectar_memoria, thread_atender_memoria, thread_esperar_kernel_dispatch, thread_esperar_kernel_interrupt;

void switch_case_memoria(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer);
void switch_case_kernel_dispatch(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer);
void switch_case_kernel_interrupt(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer);
t_instruccion determinar_codigo_instruccion(char* instruccion);

#endif /* MAIN_H_ */