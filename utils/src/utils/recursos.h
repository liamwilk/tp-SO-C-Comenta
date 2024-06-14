#ifndef RECURSOS_H_
#define RECURSOS_H_

#include "commons/collections/dictionary.h"
#include "procesos.h"

typedef struct t_recurso
{
    int instancias;
    t_list *procesos_bloqueados;
} t_recurso;

typedef enum t_recurso_motivo_liberacion
{
    SIGNAL_RECURSO,
    INTERRUPCION
} t_recurso_motivo_liberacion;

/**
 * Busca un recurso en el diccionario de recursos dado.
 *
 * @param diccionario_recursos El diccionario de recursos en el que buscar.
 * @param recursoSolicitado El nombre del recurso a buscar.
 * @return Un puntero al recurso encontrado, o NULL si el recurso no se encontró.
 */
t_recurso *recurso_buscar(t_dictionary *diccionario_recursos, char *recursoSolicitado);

/**
 * Busca un recurso asociado a un ID de proceso dado en el diccionario de recursos.
 *
 * @param diccionario_recursos El diccionario de recursos.
 * @param pid El ID de proceso a buscar.
 * @return Un puntero al recurso asociado al ID de proceso dado, o NULL si no se encuentra.
 */
char *recurso_buscar_pid(t_dictionary *diccionario_recursos, uint32_t pid);

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
void recurso_init(t_dictionary *diccionario_recursos, char *instanciasRecursos, char *recursos);

#endif /* RECURSOS_H_ */
