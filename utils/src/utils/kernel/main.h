#ifndef KERNEL_H_
#define KERNEL_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include "../conexiones.h"
#include <sys/ioctl.h>
#include <dirent.h>
#include <pwd.h>
#include <ctype.h>
#include "transiciones.h"
#include "../recursos.h"
#include "temporizador.h"

/*Estructura basica del kernel*/

/**
 * Inicializa los estados de las colas del kernel.
 *
 * Esta función inicializa los estados de las colas del kernel, que incluyen las colas new, ready, exec, block y exit.
 *
 * @param estados  Diagrama de 5 estados
 */
t_diagrama_estados kernel_inicializar_estados(t_diagrama_estados *estados);

/**
 * @brief Desaloja un proceso del kernel.
 *
 * Esta función se encarga de desalojar un proceso del kernel, actualizando el diagrama de estados,
 * Se utiliza principalmente en ROUND ROBIN o VIRTUAL ROUND ROBIN
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 * @param pid Proceso a desalojar
 */
void kernel_desalojar_proceso(hilos_args *kernel_hilos_args, t_pcb *pcb, int quantum);

/**
 * Finaliza el kernel.
 *
 * Esta función se encarga de finalizar el kernel y realizar cualquier limpieza necesaria.
 *
 * @param args Los argumentos para el kernel.
 */
void kernel_finalizar(hilos_args *args);

/**
 * Busca una interfaz en las estructuras de entrada/salida del kernel.
 *
 * @param args Los argumentos para el hilo.
 * @param interfaz El nombre de la interfaz a buscar.
 * @return Un puntero a la estructura de entrada/salida encontrada, o NULL si no se encuentra.
 */
t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz);

/**
 * @brief Función para revisar un paquete recibido por el kernel.
 *
 * @param paquete Paquete a revisar.
 * @param args Estructura de argumentos del kernel.
 * @param modulo Nombre del módulo que envía el paquete.
 */
void kernel_revisar_paquete(t_paquete *paquete, hilos_args *args, char *modulo);

/**
 * @brief Crea un nuevo proceso en el kernel.
 *
 * Esta función se encarga de crear un nuevo proceso en el kernel. Envia el struct t_kernel_memoria_proceso al modulo memoria
 *
 * @param kernel Un puntero a la estructura del kernel.
 * @param new  Un puntero a la cola de new.
 * @param logger Un puntero al logger.
 * @param instrucciones Una cadena de caracteres que contiene las instrucciones para el nuevo proceso.
 */
t_pcb *kernel_nuevo_proceso(hilos_args *args, t_diagrama_estados *estados, t_log *logger, char *instrucciones);

/**
 * @brief Inicia el hilo del planificador.
 *
 * @param args Los argumentos del hilo.
 */
void hilo_planificador_iniciar(hilos_args *args);

/**
 * @brief Cambia el estado del hilo del planificador.
 *
 * @param args Los argumentos del hilo.
 * @param estado El estado del hilo (true para activo, false para inactivo).
 */
void hilo_planificador_estado(hilos_args *args, bool estado);

/**
 * @brief Detiene el hilo del planificador.
 *
 * @param args Los argumentos del hilo.
 */
void hilo_planificador_detener(hilos_args *args);

/**
 * @brief Registra el manejador de señales.
 */
void registrar_manejador_senales();

/**
 *  Ocupa una instancia del recurso solicitado
 *
 * @param args Los argumentos para el hilo.
 * @param recursoDiccionario El diccionario que contiene los recursos.
 * @param pid El ID del proceso.
 * @param recursoSolicitado El nombre del recurso solicitado.
 */
void kernel_wait(hilos_args *args, uint32_t pid, char *recursoSolicitado);

/**
 * Libera una instancia del recurso solicitado.
 *
 * @param args Los argumentos para el hilo.
 * @param recursoDiccionario El diccionario que contiene los recursos.
 * @param pid El ID del proceso.
 * @param recursoSolicitado El nombre del recurso solicitado.
 */
void kernel_signal(hilos_args *args, uint32_t pid, char *recurso, t_recurso_motivo_liberacion MOTIVO);

/**
 * @brief Función para finalizar un proceso en el kernel.
 *
 * @param kernel_hilos_args Estructura de argumentos del kernel.
 * @param pid Identificador del proceso a finalizar.
 * @param MOTIVO Motivo de finalización del proceso.
 * @return true si se pudo finalizar el proceso, false en caso contrario.
 */
bool kernel_finalizar_proceso(hilos_args *kernel_hilos_args, uint32_t pid, KERNEL_MOTIVO_FINALIZACION MOTIVO);

void kernel_avisar_memoria_finalizacion_proceso(hilos_args *args, uint32_t pid);
void kernel_interrumpir_cpu(hilos_args *args, uint32_t pid, char *motivo);
void kernel_fin_quantum(union sigval arg);
void kernel_inicializar_temporizador(hilos_args *args, timer_args_t *temporizador);

#endif /* KERNEL_H */