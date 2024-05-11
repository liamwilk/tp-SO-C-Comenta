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
#include <utils/cpu.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>
#include <utils/procesos.h>
#include <utils/template.h>
#include <utils/instrucciones.h>

/*  Registros utilizados por la cpu */
t_cpu_proceso proceso;

// Process ID
uint32_t pid;

// Flag de interrupt: identifica una interrupcion pendiente
int flag_interrupt; 

t_cpu cpu;
t_log *logger;
t_config *config;
t_memoria_cpu_instruccion instruccion;

void *atender_kernel_dispatch();
void *atender_kernel_interrupt();
void *atender_memoria();

void *esperar_kernel_dispatch();
void *esperar_kernel_interrupt();
void *conectar_memoria();

t_memoria_cpu_instruccion *deserializar_t_memoria_cpu_instruccion(t_buffer *buffer);
t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer);

pthread_t thread_atender_kernel_dispatch, thread_atender_kernel_interrupt, thread_conectar_memoria, thread_atender_memoria, thread_esperar_kernel_dispatch, thread_esperar_kernel_interrupt;

void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_dispatch(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_interrupt(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);
t_instruccion determinar_codigo_instruccion(char *instruccion);

#endif /* MAIN_H_ */