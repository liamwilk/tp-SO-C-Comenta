

#ifndef KERNEL_ENTRADASALIDA_H_
#define KERNEL_ENTRADASALIDA_H_
#include "commons/collections/list.h"
#include "../serial.h"
#include "structs.h"
#include "consola.h"
#include "../conexiones.h"
#include "../recursos.h"

/**
 * Procesa una operación de entrada/salida rechazada.
 *
 * Esta función se encarga de manejar una operación de entrada/salida rechazada
 * en el kernel. Recibe como entrada los argumentos para el hilo de E/S y el
 * identificador de la operación que fue rechazada.
 *
 * @param argumentos Los argumentos para el hilo de E/S.
 * @param identificador El identificador de la operación rechazada.
 */
void entrada_salida_procesar_rechazado(hilos_io_args *argumentos, char *identificador);

/**
 * @brief Agrega una interfaz de entrada/salida en el kernel.
 *
 * Esta función se encarga de agregar una interfaz de entrada/salida en el kernel.
 *
 * @param args Los argumentos del hilo.
 * @param tipo El tipo de socket de la interfaz.
 * @param socket El socket de la interfaz.
 *
 * @return Un puntero a la estructura de entrada/salida creada.
 */
t_kernel_entrada_salida *entrada_salida_agregar_interfaz(hilos_args *args, KERNEL_SOCKETS tipo, int socket);

/**
 * @brief Remueve una interfaz de entrada/salida del kernel.
 *
 * Esta función se encarga de remover una interfaz de entrada/salida del kernel.
 *
 * @param args Los argumentos del hilo.
 * @param interfaz La interfaz a remover.
 */
void entrada_salida_remover_interfaz(hilos_args *args, char *interfaz);

/**
 * @brief Busca una interfaz de entrada/salida en el kernel.
 *
 * Esta función se encarga de buscar una interfaz de entrada/salida en el kernel.
 *
 * @param args Los argumentos del hilo.
 * @param interfaz La interfaz a buscar.
 *
 * @return Un puntero a la estructura de entrada/salida encontrada, o NULL si no se encontró.
 */
void entrada_salida_agregar_identificador(hilos_io_args *args, char *identificador);

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
 * @brief Busca una interfaz de entrada/salida en el kernel.
 *
 * Esta función se encarga de buscar una interfaz de entrada/salida en el kernel.
 *
 * @param args Los argumentos del hilo.
 * @param interfaz La interfaz a buscar.
 *
 * @return Un puntero a la estructura de entrada/salida encontrada, o NULL si no se encontró.
 */
t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz);

t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz_pid(hilos_args *args, uint32_t pid);

/**SWITCH CASE PARTICULAR PARA CADA ENTRADASALIDA**/
void switch_case_kernel_entrada_salida_generic(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_stdin(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_stdout(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_dialfs(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * Ejecuta las operaciones de entrada/salida en un hilo separado.
 *
 * Esta función se encarga de ejecutar las operaciones de entrada/salida en un hilo separado.
 * Recibe como entrada los argumentos para el hilo, el nombre del módulo y un puntero a función
 * que maneja el switch case del kernel para las operaciones de entrada/salida.
 *
 * @param io_args Los argumentos para el hilo.
 * @param modulo El nombre del módulo.
 * @param switch_case_atencion El puntero a función que maneja el switch case del kernel.
 */
void hilos_ejecutar_entrada_salida(hilos_io_args *io_args, char *modulo, t_funcion_kernel_io_prt switch_case_atencion);

/**
 * Maneja el rechazo de las operaciones de entrada/salida en el kernel.
 *
 * @param io_args Los argumentos para la operación de E/S.
 */
void kernel_entradasalida_rechazo(hilos_io_args *io_args, char *modulo, char *identificador);

/**
 * Notifica el rechazo de un identificador a través del socket especificado.
 *
 * @param socket El socket al que se enviará la notificación de rechazo.
 */
void avisar_rechazo_identificador(int socket);

/**
 * Agrega una nueva entrada para los sockets del kernel.
 *
 * Esta función agrega una nueva entrada para los sockets del kernel del tipo y socket especificados.
 *
 * @param args Los argumentos para el hilo.
 * @param tipo El tipo de sockets del kernel.
 * @param socket El socket a agregar.
 * @return Un puntero a la estructura t_kernel_entrada_salida recién agregada.
 */
t_kernel_entrada_salida *kernel_sockets_agregar_entrada_salida(hilos_args *args, KERNEL_SOCKETS type, int socket);

typedef enum
{
    CPU_IO_STDOUT_WRITE,
    CPU_IO_STDIN_READ
} t_kernel_cpu_entradasalida_error;

void kernel_cpu_entradasalida_no_conectada(hilos_args *args, t_kernel_cpu_entradasalida_error TIPO, char *interfaz, uint32_t pid);

/**
 * Ejecuta una operación de entrada/salida del kernel CPU de un tipo diferente.
 *
 * Esta función se encarga de realizar una operación de entrada/salida del kernel CPU
 * de un tipo especificado. Recibe como parámetros los argumentos para la operación,
 * el tipo de error a manejar, la interfaz a utilizar y el ID del proceso.
 *
 * @param args      Los argumentos para la operación.
 * @param TIPO      El tipo de error a manejar.
 * @param interfaz  La interfaz a utilizar.
 * @param pid       El ID del proceso.
 */
void kernel_cpu_entradasalida_distinto_tipo(hilos_args *args, t_kernel_cpu_entradasalida_error TIPO, char *interfaz, uint32_t pid);

/**
 * @brief Notifica al kernel que la entrada/salida de una CPU está ocupada.
 *
 * Esta función se llama para informar al kernel que la entrada/salida de una CPU está actualmente ocupada.
 *
 * @param args Los argumentos para el hilo.
 * @param TIPO El tipo de error encontrado durante la entrada/salida.
 * @param interfaz La interfaz utilizada para la entrada/salida.
 * @param pid El ID del proceso asociado con la operación de entrada/salida.
 */
void kernel_cpu_entradasalida_ocupada(hilos_args *args, t_kernel_cpu_entradasalida_error TIPO, char *interfaz, uint32_t pid);

// Se busca el proximo proceso en block esperando utilizar esa io especificamente IO_GENERIC
void kernel_proximo_io_generic(hilos_args *args, t_kernel_entrada_salida *io);

#endif /* KERNEL_ENTRADASALIDA_H_ */
