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
#include "conexiones.h"
#include <sys/ioctl.h>
#include <dirent.h>
#include <pwd.h>
#include <ctype.h>
#include "kernel/transiciones.h"

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
t_kernel_entrada_salida *entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz);

/**
 * @brief Función para revisar un paquete recibido por el kernel.
 *
 * @param paquete Paquete a revisar.
 * @param args Estructura de argumentos del kernel.
 * @param modulo Nombre del módulo que envía el paquete.
 */
void kernel_revisar_paquete(t_paquete *paquete, hilos_args *args, char *modulo);

/**
 * @brief Función para iniciar un proceso en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void iniciar_proceso(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para finalizar un proceso en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void finalizar_proceso(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para iniciar la planificación de procesos en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void iniciar_planificacion(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para detener la planificación de procesos en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void detener_planificacion(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para establecer el grado de multiprogramación en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void multiprogramacion(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para mostrar el estado de los procesos en el kernel.
 *
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void procesos_estados(hilos_args *hiloArgs);

/**
 * @brief Función para finalizar la consola del kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void finalizar_consola(char **separar_linea, hilos_args *hiloArgs);

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

void hilos_ejecutar_entrada_salida(hilos_io_args *io_args, char *modulo, t_funcion_kernel_io_prt switch_case_atencion);
void switch_case_kernel_entrada_salida_generic(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_stdin(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_stdout(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_dialfs(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);

void kernel_interrumpir_cpu(hilos_args *args, uint32_t pid, char *motivo);

void kernel_avisar_memoria_finalizacion_proceso(hilos_args *args, uint32_t pid);

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

#endif /* KERNEL_H */