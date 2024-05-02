#ifndef PROCESOS_H_
#define PROCESOS_H_
#include <commons/log.h>
#include <stdlib.h>
#include <stdint.h>
#include <utils/modulos.h>
#include <utils/serial.h>
#include <commons/collections/queue.h>

typedef struct diagrama_estados
{
    t_queue *new;
    t_queue *ready;
    t_queue *exec;
    t_queue *block;
    t_queue *exit;
} diagrama_estados;

typedef struct pcb
{
    uint32_t pid;
    uint32_t quantum;
    t_registros_cpu *registros_cpu;
    bool memoria_aceptado;
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
 * @param new La cola "new".
 * @param pcb El PCB a agregar.
 */
void proceso_agregar_new(t_queue *new, t_pcb *pcb);

/**
 * Busca un proceso con el PID dado en la cola "new".
 *
 * @param new La cola "new" en la que buscar.
 * @param pid El PID del proceso a buscar.
 * @return El PCB (Bloque de Control de Procesos) del proceso encontrado, o NULL si no se encuentra.
 */
t_pcb proceso_buscar_new(t_queue *new, int pid);

/**
 * Agrega un PCB a la cola "ready".
 *
 * @param ready La cola "ready".
 * @param pcb El PCB a agregar.
 */
void proceso_agregar_ready(t_queue *ready, t_pcb *pcb);

/**
 * Elimina un PCB de la cola "new".
 *
 * @param new La cola "new".
 * @param pcb El PCB a eliminar.
 */
void proceso_quitar_new(t_queue *new);

/**
 * Elimina un PCB de la cola "ready".
 *
 * @param ready La cola "ready".
 * @param pcb El PCB a eliminar.
 */
void proceso_quitar_ready(t_queue *ready);

/**
 * Mueve un proceso al estado "ready" en el kernel.
 *
 * @param kernel La instancia del kernel.
 * @param logger La instancia del logger.
 * @param estados El diagrama de estados.
 */
void proceso_mover_ready(int gradoMultiprogramacion, t_log *logger, diagrama_estados *estados);

#endif /* PROCESOS_H_ */
