#include "consola.h"

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
