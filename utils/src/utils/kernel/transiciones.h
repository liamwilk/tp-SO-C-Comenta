#ifndef KERNEL_TRANSICIONES_H_
#define KERNEL_TRANSICIONES_H_
#include "../serial.h"
#include "consola.h"

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

void kernel_manejar_ready(hilos_args *args, uint32_t pid, t_transiciones_ready TRANSICION);

/**
 * @brief Función para finalizar un proceso en el kernel.
 *
 * @param kernel_hilos_args Estructura de argumentos del kernel.
 * @param pid Identificador del proceso a finalizar.
 * @param MOTIVO Motivo de finalización del proceso.
 * @return true si se pudo finalizar el proceso, false en caso contrario.
 */
bool kernel_finalizar_proceso(hilos_args *kernel_hilos_args, uint32_t pid, KERNEL_MOTIVO_FINALIZACION MOTIVO);

typedef enum t_recurso_motivo_liberacion
{
    SIGNAL_RECURSO,
    INTERRUPCION
} t_recurso_motivo_liberacion;

/**
 * Inicializa los recursos del kernel.
 *
 * Esta función inicializa los recursos del kernel creando un diccionario para almacenar los recursos
 * y poblándolo con las instancias y nombres de recursos proporcionados.
 *
 * @param diccionario_recursos Un puntero al diccionario que almacenará los recursos.
 * @param instanciasRecursos Una cadena que contiene las instancias de los recursos.
 * @param recursos Una cadena que contiene los nombres de los recursos.
 */
void kernel_recurso_init(t_dictionary *diccionario_recursos, char *instanciasRecursos, char *recursos);

/**
 * Busca un recurso en el diccionario de recursos dado.
 *
 * @param diccionario_recursos El diccionario de recursos en el que buscar.
 * @param recursoSolicitado El nombre del recurso a buscar.
 * @return Un puntero al recurso encontrado, o NULL si el recurso no se encontró.
 */
t_recurso *kernel_recurso_buscar(t_dictionary *diccionario_recursos, char *recursoSolicitado);

/**
 * Recupera el recurso del kernel asociado con el ID de proceso dado.
 *
 * @param args Los argumentos para el hilo.
 * @param pid El ID del proceso.
 * @return Un puntero al recurso del kernel.
 */
char *kernel_recurso(hilos_args *args, uint32_t pid);

/**
 *  Ocupa una instancia del recurso solicitado
 *
 * @param args Los argumentos para el hilo.
 * @param recursoDiccionario El diccionario que contiene los recursos.
 * @param pid El ID del proceso.
 * @param recursoSolicitado El nombre del recurso solicitado.
 */
void kernel_recurso_wait(hilos_args *args, t_dictionary *recursoDiccionario, uint32_t pid, char *recursoSolicitado);

/**
 * Libera una instancia del recurso solicitado.
 *
 * @param args Los argumentos para el hilo.
 * @param recursoDiccionario El diccionario que contiene los recursos.
 * @param pid El ID del proceso.
 * @param recursoSolicitado El nombre del recurso solicitado.
 */
void kernel_recurso_signal(hilos_args *args, t_dictionary *recursos, uint32_t pid, char *recurso, t_recurso_motivo_liberacion MOTIVO);

/**
 * Registra en el log el recurso del kernel.
 *
 * @param args Los argumentos para el recurso del kernel.
 */
void kernel_recurso_log(hilos_args *args);
// TODO: Mover a un archivo de interrupciones/utilidades
void manejador_interrupciones(union sigval arg);
int interrumpir_temporizador(hilos_args *args);
void kernel_interrumpir_cpu(hilos_args *args, uint32_t pid, char *motivo);
void kernel_avisar_memoria_finalizacion_proceso(hilos_args *args, uint32_t pid);

// TODO: Mover a un archivo de entrada/salida
/**
 * @brief Interrumpe las operaciones de E/S para un proceso específico en el kernel.
 *
 * Esta función interrumpe las operaciones de E/S para un proceso específico identificado por su PID.
 * La interrupción se realiza con una razón dada especificada por el parámetro `motivo`.
 *
 * @param args Un puntero a la estructura `hilos_args` que contiene los argumentos necesarios para el kernel.
 * @param pid El PID (Identificador de Proceso) del proceso para interrumpir sus operaciones de E/S.
 * @param motivo Una cadena que especifica la razón para interrumpir las operaciones de E/S.
 */
void kernel_interrumpir_io(hilos_args *args, uint32_t pid, char *motivo);

/**
 * Busca una interfaz de entrada/salida del kernel basada en los argumentos del hilo y el ID del proceso.
 *
 * @param args Los argumentos del hilo.
 * @param pid El ID del proceso.
 * @return Un puntero a la interfaz t_kernel_entrada_salida encontrada, o NULL si no se encuentra.
 */
t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, uint32_t pid);

#endif /* KERNEL_TRANSICIONES_H_ */
