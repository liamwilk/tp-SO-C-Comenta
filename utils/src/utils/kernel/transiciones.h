#ifndef KERNEL_TRANSICIONES_H_
#define KERNEL_TRANSICIONES_H_
#include "../serial.h"
#include "consola.h"
#include "entradasalida.h"
#include "temporizador.h"

/**
 * Transiciona un PCB del estado EXEC al estado READY en el diagrama de estados del kernel.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_ready(hilos_args *kernel_hilos_args);

/**
 * Transiciona un PCB del estado READY al estado EXEC en el kernel.
 *
 * Esta función toma como parámetros un diagrama de estados y un kernel, y transiciona un Bloque de Control de Proceso (PCB)
 * del estado READY al estado EXEC en el kernel. Retorna un puntero al PCB actualizado.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_ready_exec(hilos_args *kernel_hilos_args);

/**
 * Transiciona un proceso del estado bloqueado al estado listo en el kernel.
 *
 * Esta función toma como parámetros un diagrama de estados y un logger. Extrae un proceso del estado bloqueado,
 * lo coloca en el estado listo y devuelve el proceso. Si no hay procesos en el estado bloqueado, devuelve NULL.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 * @param modulo
 * @param unidad
 */
t_pcb *kernel_transicion_block_ready(hilos_args *kernel_hilos_args, uint32_t pid);

/**
 * @brief Función de transición para ejecutar un bloque en el kernel.
 *
 * Esta función realiza la transición del estado exec al estado "block" y devuelve el siguiente PCB (Bloque de Control de Proceso)
 * a ser ejecutado según el diagrama de estados y el logger proporcionados.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_block(hilos_args *kernel_hilo_args);

/**
 * @brief Realiza la transición del estado EXEC al estado EXIT en el kernel.
 *
 * Esta función actualiza el diagrama de estados y registra la transición en el logger.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_exit(hilos_args *kernel_hilo_args);

/**
 * Transiciona un bloque al estado de salida en el kernel.
 *
 * Esta función se encarga de transicionar un bloque al estado de salida en el kernel.
 *
 * @param kernel_hilos_args Los argumentos para los hilos del kernel.
 * @param pid El ID del proceso del bloque a transicionar.
 */
void kernel_transicion_block_exit(hilos_args *kernel_hilos_args, uint32_t pid);

/**
 * @brief Función de transición para ejecutar un bloque en el kernel.
 *
 * Esta función realiza la transición del estado new al estado "ready" y devuelve el  PCB (Bloque de Control de Proceso)
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_new_ready(hilos_args *kernel_hilo_args);

/**Virtual Round Robin**/

/**
 * Recupera el PCB (Bloque de Control de Proceso) con la mayor prioridad del estado bloqueado y lo mueve al estado listo.
 *
 * @param kernel_hilos_args La estructura de argumentos para los hilos del kernel.
 * @param pid El ID del proceso del PCB a recuperar.
 * @return Un puntero al PCB con la mayor prioridad, o NULL si no se encuentra ningún PCB.
 */
t_pcb *kernel_transicion_block_ready_mayor_prioridad(hilos_args *kernel_hilos_args, uint32_t pid);

/**
 * Ejecuta la transición del estado "exec" al estado "ready" para el hilo con mayor prioridad.
 *
 * Esta función se encarga de seleccionar el hilo con mayor prioridad de la lista de hilos en el estado "exec",
 * y transicionarlo al estado "ready". Retorna un puntero al PCB (Bloque de Control de Proceso) del hilo seleccionado.
 *
 * @param kernel_hilos_args La estructura de argumentos para los hilos del kernel.
 * @return Un puntero al PCB del hilo con mayor prioridad en el estado "exec".
 */
t_pcb *kernel_transicion_exec_ready_mayor_prioridad(hilos_args *kernel_hilos_args);

/**
 * Ejecuta la transición del estado "ready" al estado "exec" para el hilo con mayor prioridad.
 *
 * Esta función se encarga de seleccionar el hilo con mayor prioridad de la lista de hilos en el estado "ready",
 * y transicionarlo al estado "exec". Retorna un puntero al PCB (Bloque de Control de Proceso) del hilo seleccionado.
 *
 * @param kernel_hilos_args La estructura de argumentos para los hilos del kernel.
 * @return Un puntero al PCB del hilo con mayor prioridad en el estado "exec".
 */
t_pcb *kernel_transicion_ready_exec_mayor_prioridad(hilos_args *kernel_hilos_args);

/**
 * Maneja la transición de un proceso al estado listo en el kernel.
 *
 * @param args Los argumentos para el hilo.
 * @param pid El ID del proceso.
 * @param TRANSICION La transición específica que se debe manejar.
 */
void kernel_manejar_ready(hilos_args *args, uint32_t pid, t_transiciones_ready TRANSICION);

#endif /* KERNEL_TRANSICIONES_H_ */
