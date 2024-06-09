#ifndef HILOS_H_
#define HILOS_H_
#include <consola.h>
#include <commons/log.h>
#include <pthread.h>
#include <utils/serial.h>
#include <utils/handshake.h>
#include <utils/conexiones.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <utils/modulos.h>
#include <utils/configs.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "planificacion.h"
#include <utils/template.h>
#include "modulos/cpu_dispatch.h"
#include "modulos/cpu_interrupt.h"
#include "modulos/memoria.h"
#include <utils/kernel/main.h>

/*----ATENDER----*/

/**
 * @brief Función que atiende las solicitudes de la consola.
 *
 * Esta función se encarga de atender las solicitudes de la consola y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_consola(void *args);

/**
 * @brief Función que atiende las solicitudes de la memoria.
 *
 * Esta función se encarga de atender las solicitudes de la memoria y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_memoria(void *args);

/**
 * @brief Función que atiende las solicitudes de la CPU en modo dispatch.
 *
 * Esta función se encarga de atender las solicitudes de la CPU en modo dispatch y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_cpu_dispatch(void *args);

/**
 * @brief Función que atiende las solicitudes de la CPU en modo interrupt.
 *
 * Esta función se encarga de atender las solicitudes de la CPU en modo interrupt y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_cpu_interrupt(void *args);

/**
 * @brief Función que atiende las solicitudes de entrada/salida genéricas.
 *
 * Esta función se encarga de atender las solicitudes de entrada/salida genéricas y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_entrada_salida_generic(void *args);

/**
 * @brief Función que atiende las solicitudes de entrada/salida de stdin.
 *
 * Esta función se encarga de atender las solicitudes de entrada/salida de stdin y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_entrada_salida_stdin(void *args);

/**
 * @brief Función que atiende las solicitudes de entrada/salida de stdout.
 *
 * Esta función se encarga de atender las solicitudes de entrada/salida de stdout y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_entrada_salida_stdout(void *args);

/**
 * @brief Función que atiende las solicitudes de entrada/salida de dialfs.
 *
 * Esta función se encarga de atender las solicitudes de entrada/salida de dialfs y realizar las acciones correspondientes.
 *
 * @param args Argumentos necesarios para la ejecución de la función.
 * @return Puntero a void.
 */
void *hilos_atender_entrada_salida_dialfs(void *args);

/*----CONECTAR----*/

/**
 * @brief Función que se ejecuta en un hilo para conectar con la memoria.
 *
 * @param args Argumentos necesarios para la ejecución del hilo.
 * @return void* Puntero a un valor de retorno (no utilizado en este caso).
 */
void *hilos_conectar_memoria(void *args);

/**
 * @brief Función que se ejecuta en un hilo para conectar con la CPU en modo dispatch.
 *
 * @param args Argumentos necesarios para la ejecución del hilo.
 * @return void* Puntero a un valor de retorno (no utilizado en este caso).
 */
void *hilos_conectar_cpu_dispatch(void *args);

/**
 * @brief Función que se ejecuta en un hilo para conectar con la CPU en modo interrupt.
 *
 * @param args Argumentos necesarios para la ejecución del hilo.
 * @return void* Puntero a un valor de retorno (no utilizado en este caso).
 */
void *hilos_conectar_cpu_interrupt(void *args);

/**
 * @brief Función que se ejecuta en un hilo para esperar entrada/salida.
 *
 * @param args Argumentos necesarios para la ejecución del hilo.
 * @return void* Puntero a un valor de retorno (no utilizado en este caso).
 */
void *hilos_esperar_entrada_salida(void *args);

/**
 * @brief Función que se ejecuta en un hilo para el planificador.
 *
 * @param args Argumentos necesarios para la ejecución del hilo.
 * @return void* Puntero a un valor de retorno (no utilizado en este caso).
 */
void *hilo_planificador(void *args);

/**
 * @brief Función que inicializa los hilos relacionados con la memoria.
 *
 * @param args Argumentos necesarios para la inicialización de los hilos.
 * @param thread_conectar_memoria Identificador del hilo de conexión con la memoria.
 * @param thread_atender_memoria Identificador del hilo de atención a la memoria.
 */
void hilos_memoria_inicializar(hilos_args *args, pthread_t thread_conectar_memoria, pthread_t thread_atender_memoria);

/**
 * @brief Función que inicializa los hilos relacionados con la CPU.
 *
 * @param args Argumentos necesarios para la inicialización de los hilos.
 * @param thread_conectar_cpu_dispatch Identificador del hilo de conexión con la CPU en modo dispatch.
 * @param thread_atender_cpu_dispatch Identificador del hilo de atención a la CPU en modo dispatch.
 * @param thread_conectar_cpu_interrupt Identificador del hilo de conexión con la CPU en modo interrupt.
 * @param thread_atender_cpu_interrupt Identificador del hilo de atención a la CPU en modo interrupt.
 */
void hilos_cpu_inicializar(hilos_args *args, pthread_t thread_conectar_cpu_dispatch, pthread_t thread_atender_cpu_dispatch, pthread_t thread_conectar_cpu_interrupt, pthread_t thread_atender_cpu_interrupt);

/**
 * @brief Función que inicializa el hilo relacionado con la entrada/salida.
 *
 * @param args Argumentos necesarios para la inicialización del hilo.
 * @param thread_esperar_entrada_salida Identificador del hilo de espera de entrada/salida.
 */
void hilos_io_inicializar(hilos_args *args, pthread_t thread_esperar_entrada_salida);

/**
 * @brief Función que inicializa el hilo relacionado con la consola.
 *
 * @param args Argumentos necesarios para la inicialización del hilo.
 * @param thread_atender_consola Identificador del hilo de atención a la consola.
 */
void hilos_consola_inicializar(hilos_args *args, pthread_t thread_atender_consola);

/**
 * @brief Función que inicializa el hilo relacionado con el planificador.
 *
 * @param args Argumentos necesarios para la inicialización del hilo.
 * @param thread_planificador Identificador del hilo del planificador.
 */
void hilos_planificador_inicializar(hilos_args *args, pthread_t thread_planificador);

/**
 * @brief Función que obtiene la clave de finalización de un hilo.
 *
 * @param args Argumentos necesarios para la obtención de la clave.
 * @return int Clave de finalización del hilo.
 */
int obtener_key_finalizacion_hilo(hilos_args *args);

/**
 * @brief Función que obtiene la clave de detención del algoritmo.
 *
 * @param args Argumentos necesarios para la obtención de la clave.
 * @return int Clave de detención del algoritmo.
 */
int obtener_key_detencion_algoritmo(hilos_args *args);
void avisar_rechazo_identificador(int socket);

#endif /* HILOS_H_ */