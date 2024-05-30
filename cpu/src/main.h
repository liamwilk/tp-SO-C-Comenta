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

/**
 * @brief Función para atender el kernel dispatch.
 * 
 * @return void* Puntero a la función atender_kernel_dispatch.
 */
void *atender_kernel_dispatch();

/**
 * @brief Función para atender la interrupción del kernel.
 * 
 * @return void* Puntero a la función atender_kernel_interrupt.
 */
void *atender_kernel_interrupt();

/**
 * @brief Función para atender la memoria.
 * 
 * @return void* Puntero a la función atender_memoria.
 */
void *atender_memoria();

/**
 * @brief Función para esperar el kernel dispatch.
 * 
 * @return void* Puntero a la función esperar_kernel_dispatch.
 */
void *esperar_kernel_dispatch();

/**
 * @brief Función para esperar la interrupción del kernel.
 * 
 * @return void* Puntero a la función esperar_kernel_interrupt.
 */
void *esperar_kernel_interrupt();

/**
 * @brief Función para conectar con la memoria.
 * 
 * @return void* Puntero a la función conectar_memoria.
 */
void *conectar_memoria();

/**
 * @brief Deserializa un buffer en una estructura t_memoria_cpu_instruccion.
 *
 * Esta función toma un buffer y lo deserializa en una estructura t_memoria_cpu_instruccion.
 *
 * @param buffer El buffer a deserializar.
 * @return Un puntero a la estructura t_memoria_cpu_instruccion deserializada.
 */
t_memoria_cpu_instruccion *deserializar_t_memoria_cpu_instruccion(t_buffer *buffer);

/**
 * @brief Deserializa un buffer en una estructura t_kernel_cpu_proceso.
 *
 * Esta función toma un buffer y lo deserializa en una estructura t_kernel_cpu_proceso.
 *
 * @param buffer El buffer a deserializar.
 * @return Un puntero a la estructura t_kernel_cpu_proceso deserializada.
 */
t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer);

pthread_t thread_atender_kernel_dispatch, thread_atender_kernel_interrupt, thread_conectar_memoria, thread_atender_memoria, thread_esperar_kernel_dispatch, thread_esperar_kernel_interrupt;

/**
 * @brief Función que realiza un switch case para determinar la acción a realizar en la memoria.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param codigo_operacion Código de operación que indica la acción a realizar.
 * @param buffer Puntero al buffer que contiene los datos necesarios para la operación.
 */
void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función que realiza un switch case para determinar la acción a realizar en el kernel durante un dispatch.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param codigo_operacion Código de operación que indica la acción a realizar.
 * @param buffer Puntero al buffer que contiene los datos necesarios para la operación.
 */
void switch_case_kernel_dispatch(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función que realiza un switch case para determinar la acción a realizar en el kernel durante una interrupción.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param codigo_operacion Código de operación que indica la acción a realizar.
 * @param buffer Puntero al buffer que contiene los datos necesarios para la operación.
 */
void switch_case_kernel_interrupt(t_log *logger, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función que determina el código de instrucción a partir de una cadena de caracteres.
 *
 * @param instruccion Cadena de caracteres que representa una instrucción.
 * @return El código de instrucción correspondiente.
 */
t_instruccion determinar_codigo_instruccion(char *instruccion);

#endif /* MAIN_H_ */