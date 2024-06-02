#ifndef PROCESOS_H_
#define PROCESOS_H_
#include <commons/log.h>
#include <stdbool.h>
#include <utils/modulos.h>
#include <utils/serial.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/temporal.h>

typedef struct t_diagrama_estados
{
    t_list *new;
    t_list *ready;
    t_list *exec;
    t_list *block;
    t_list *exit;
    t_list *ready_mayor_prioridad; // Para VRR
    t_dictionary *procesos;

    //********** MUTEX **********//
    pthread_mutex_t mutex_ready_exec;
    pthread_mutex_t mutex_exec_ready;
    pthread_mutex_t mutex_exec_exit;
    pthread_mutex_t mutex_block_ready;
    pthread_mutex_t mutex_block_exit;
    pthread_mutex_t mutex_exec_block;
    pthread_mutex_t mutex_new_ready;
    pthread_mutex_t mutex_new;
    pthread_mutex_t mutex_exec_ready_mayor_prioridad;
    pthread_mutex_t mutex_ready_exec_mayor_prioridad;
    pthread_mutex_t mutex_block_ready_mayor_prioridad;
} t_diagrama_estados;

typedef struct pcb
{
    uint32_t pid;
    uint32_t quantum;
    t_registros_cpu *registros_cpu;
    bool memoria_aceptado;
    t_temporal *tiempo_fin; // Esto es utilizado en los algoritmos de planificación con QUANTUM
} t_pcb;

extern uint32_t pid;

/**
 * @file pcb.h
 * @brief Archivo de encabezado para el módulo PCB (Bloque de Control de Procesos).
 *
 * Este archivo contiene las declaraciones de funciones y estructuras relacionadas con el módulo PCB.
 * El módulo PCB es responsable de gestionar los bloques de control de procesos, que almacenan información
 * sobre cada proceso en el sistema.
 */
/**
 * Crea un nuevo PCB (Bloque de Control de Procesos) con los parámetros dados.
 *
 * @param logger El logger a utilizar para registrar mensajes.
 * @param quantum El valor de quantum para el PCB.
 * @return Un puntero al PCB recién creado.
 */
t_pcb *pcb_crear(t_log *logger, int quantum);

/**
 * Genera un nuevo ID de proceso (PID).
 *
 * Esta función genera un nuevo ID de proceso (PID) único incrementando el valor `pid` dado.
 *
 * @param pid Un puntero al valor actual de PID.
 * @return El nuevo PID generado.
 */
uint32_t new_pid();

/**
 * Agrega un PCB a la cola "new".
 *
 * @param estados El diagrama de 5 estados que se debe actualizar.
 * @param pcb El PCB a agregar.
 */
void proceso_push_new(t_diagrama_estados *estados, t_pcb *pcb);

/**
 * Agrega un PCB a la cola "ready".
 *
 * @param estados diagrama de 5 estados
 * @param pcb El PCB a agregar.
 */
void proceso_push_ready(t_diagrama_estados *estados, t_pcb *pcb);

/**
 * Agrega un PCB a la cola "exec".
 *
 * @param estados diagrama de 5 estados
 * @param pcb El PCB a agregar.
 */
void proceso_push_exec(t_diagrama_estados *estados, t_pcb *pcb);

/**
 * Agrega un PCB a la cola "block".
 *
 * @param estados diagrama de 5 estados
 * @param pcb El PCB a agregar.
 */
void proceso_push_block(t_diagrama_estados *estados, t_pcb *pcb);

/**
 * Agrega un PCB a la cola "exit".
 *
 * @param estados diagrama de 5 estados
 * @param pcb El PCB a agregar.
 */
void proceso_push_exit(t_diagrama_estados *estados, t_pcb *pcb);

/**
 * Agrega un PCB a la cola prioritaria de un diagrama de estados.
 *
 * @param estados El diagrama de estados donde se agregará el PCB.
 * @param pcb El PCB que se agregará a la cola prioritaria.
 */
void proceso_push_cola_prioritaria(t_diagrama_estados *estados, t_pcb *pcb);

/**
 * Remueve el proceso de mayor prioridad de la cola prioritaria.
 *
 * @param estados Un puntero al diagrama de estados.
 */
t_pcb *proceso_pop_cola_prioritaria(t_diagrama_estados *estados);

/**
 * Busca un proceso con el PID dado en la cola "new".
 *
 * @param new La cola "new" en la que buscar.
 * @param pid El PID del proceso a buscar.
 * @return El PCB (Bloque de Control de Procesos) del proceso encontrado, o NULL si no se encuentra.
 */
t_pcb *proceso_buscar_new(t_diagrama_estados *estados, int pid);

/**
 * Elimina un proceso de la cola "ready".
 *
 * Esta función elimina un proceso de la cola "ready" y devuelve un puntero al bloque de control de procesos (PCB) eliminado.
 *
 * @param ready La cola "ready" de la cual eliminar el proceso.
 * @return Un puntero al bloque de control de procesos (PCB) eliminado.
 */
t_pcb *proceso_pop_ready(t_diagrama_estados *estados);

/**
 * Quita un proceso de la cola de new sin eliminar su estructura
 *
 * @param new La cola "new".
 * @param pcb El PCB a eliminar.
 */
t_pcb *proceso_pop_new(t_diagrama_estados *estados);

/**
 * Elimina un proceso de la cola "exec".
 *
 * Esta función elimina un proceso de la cola "exec" y devuelve un puntero al bloque de control de procesos (PCB) eliminado.
 *
 * @param exec La cola "exec" de la cual eliminar el proceso.
 * @return Un puntero al bloque de control de procesos (PCB) eliminado.
 */
t_pcb *proceso_pop_exec(t_diagrama_estados *estados);

/**
 * Elimina un proceso de la cola "exit".
 *
 * Esta función elimina un proceso de la cola "exit" y devuelve un puntero al bloque de control de procesos (PCB) eliminado.
 *
 * @param exit La cola "exit" de la cual eliminar el proceso.
 * @return Un puntero al bloque de control de procesos (PCB) eliminado.
 */
t_pcb *proceso_pop_exit(t_diagrama_estados *estados);

/**
 * Elimina un proceso de la cola "block".
 *
 * Esta función elimina un proceso de la cola "block" y devuelve un puntero al bloque de control de procesos (PCB) eliminado.
 *
 * @param block La cola "block" de la cual eliminar el proceso.
 * @return Un puntero al bloque de control de procesos (PCB) eliminado.
 */
t_pcb *proceso_remover_block(t_diagrama_estados *estados, uint32_t pid);

/**
 * @brief Revierte la creacion de un proceso en estado new
 *
 * @param new El proceso nuevo a eliminar.
 * @param pid El ID del proceso nuevo.
 * @return true si el proceso nuevo se eliminó correctamente, false en caso contrario.
 */
int proceso_revertir(t_diagrama_estados *estados, uint32_t processPID);

/**
 *  Busca un proceso con el PID dado en la cola de listos.
 *
 * @param ready La cola de listos en la que buscar.
 * @param pid El PID del proceso a buscar.
 * @return Un puntero al PCB del proceso encontrado, o NULL si no se encuentra.
 */
t_pcb *proceso_buscar_ready(t_diagrama_estados *estados, int pid);

/**
 * Busca un proceso con el PID dado en la cola de ejecución.
 *
 * @param exec La cola de ejecución en la que buscar.
 * @param pid El PID del proceso a buscar.
 * @return Un puntero al PCB del proceso encontrado, o NULL si no se encuentra.
 */
t_pcb *proceso_buscar_exec(t_diagrama_estados *estados, int pid);

/**
 * Busca un proceso con el PID dado en la cola de bloqueados.
 *
 * @param block La cola de bloqueados en la que buscar.
 * @param pid El PID del proceso a buscar.
 * @return Un puntero al PCB del proceso encontrado, o NULL si no se encuentra.
 */
t_pcb *proceso_buscar_block(t_diagrama_estados *estados, int pid);

/**
 * Busca un proceso con el PID dado en la cola de finalizados.
 *
 * @param exit La cola de finalizados en la que buscar.
 * @param pid El PID del proceso a buscar.
 * @return Un puntero al PCB del proceso encontrado, o NULL si no se encuentra.
 */
t_pcb *proceso_buscar_exit(t_diagrama_estados *estados, int pid);

/**
 * Obtiene el estado de un proceso basado en su ID de proceso.
 *
 * @param estados El diagrama de estados que contiene los estados de los procesos.
 * @param pid El ID de proceso del proceso del cual se desea obtener el estado.
 * @return Un puntero a una cadena de caracteres que representa el estado del proceso.
 */
char *proceso_estado(t_diagrama_estados *estados, int pid);

/**
 * @brief Mata un proceso identificado por su PID.
 *
 * Esta función mata un proceso identificado por su PID. Toma como parámetros un puntero a una estructura `t_diagrama_estados`
 * y el PID del proceso a matar.
 *
 * @param estados Un puntero a una estructura `t_diagrama_estados` que contiene los estados de los procesos.
 * @param pid El PID del proceso a matar.
 */
void proceso_matar(t_diagrama_estados *estados, char *pid);

/**
 * Inicializa el buffer de transiciones.
 *
 * @param buffer El diccionario utilizado como buffer de transiciones.
 */
void procesos_inicializar_buffer_transiciones(t_dictionary *buffer);

/**
 * Obtiene una transición del tipo enum como una cadena de caracteres.
 *
 * @return La cadena de caracteres que representa la transición.
 */
char *obtener_transicion_enum();

t_list *proceso_obtener_estado(t_diagrama_estados *estados, char *estado);

/** VRR/ROUND_ROBIN FUNCIONALIDADES**/

void proceso_avisar_timer(char *algoritmoPlanificador, t_pcb *pcb);

/**
 * Verifica si un proceso ha excedido su quantum.
 *
 * @param pcb El bloque de control de procesos del proceso.
 * @return true si el proceso ha excedido su quantum, false en caso contrario.
 */
bool proceso_sobra_quantum(t_pcb *pcb);

void proceso_actualizar_registros(t_pcb *pcb, t_registros_cpu registros_cpu);

#endif