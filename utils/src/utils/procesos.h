#ifndef PROCESOS_H_
#define PROCESOS_H_
#include <commons/log.h>
#include <stdlib.h>
#include <stdint.h>
#include <utils/modulos.h>
#include <utils/serial.h>
#include <commons/collections/queue.h>
#include <commons/string.h>

typedef struct t_new
{
    t_list *cola;
    t_dictionary *diccionario;
} t_new;

typedef struct t_ready
{
    t_list *cola;
    t_dictionary *diccionario;
} t_ready;

typedef struct t_exec
{
    t_list *cola;
    t_dictionary *diccionario;
} t_exec;

typedef struct t_block
{
    t_list *cola;
    t_dictionary *diccionario;
} t_block;

typedef struct t_exit
{
    t_list *cola;
    t_dictionary *diccionario;
} t_exit;

typedef struct diagrama_estados
{
    t_new *new;
    t_ready *ready;
    t_exec *exec;
    t_block *block;
    t_exit *exit;
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
void proceso_agregar_new(t_new *new, t_pcb *pcb);

/**
 * Agrega un PCB a la cola "ready".
 *
 * @param ready La cola "ready".
 * @param pcb El PCB a agregar.
 */
void proceso_agregar_ready(t_ready *ready, t_pcb *pcb);

/**
 * Busca un proceso con el PID dado en la cola "new".
 *
 * @param new La cola "new" en la que buscar.
 * @param pid El PID del proceso a buscar.
 * @return El PCB (Bloque de Control de Procesos) del proceso encontrado, o NULL si no se encuentra.
 */
t_pcb *proceso_buscar_new(t_new *new, int pid);

/**
 * Elimina un proceso de la cola "ready".
 *
 * Esta función elimina un proceso de la cola "ready" y devuelve un puntero al bloque de control de procesos (PCB) eliminado.
 *
 * @param ready La cola "ready" de la cual eliminar el proceso.
 * @return Un puntero al bloque de control de procesos (PCB) eliminado.
 */
t_pcb *proceso_quitar_ready(t_ready *ready);

/**
 * Elimina un PCB de la cola "new".
 *
 * @param new La cola "new".
 * @param pcb El PCB a eliminar.
 */
t_pcb *proceso_quitar_new(t_new *new);

/**
 * Mueve un proceso al estado "ready" en el kernel.
 *
 * @param kernel La instancia del kernel.
 * @param logger La instancia del logger.
 * @param estados El diagrama de estados.
 */
void proceso_mover_ready(int gradoMultiprogramacion, t_log *logger, diagrama_estados *estados);

#endif /* PROCESOS_H_ */
