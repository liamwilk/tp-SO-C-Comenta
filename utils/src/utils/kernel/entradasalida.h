

#ifndef KERNEL_ENTRADASALIDA_H_
#define KERNEL_ENTRADASALIDA_H_
#include "commons/collections/list.h"
#include "../serial.h"
#include "structs.h"

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
void kernel_interrumpir_io(t_list *list_entrada_salida, uint32_t pid, char *motivo);

/**
 * Busca una interfaz de entrada/salida del kernel basada en los argumentos del hilo y el ID del proceso.
 *
 * @param args Los argumentos del hilo.
 * @param pid El ID del proceso.
 * @return Un puntero a la interfaz t_kernel_entrada_salida encontrada, o NULL si no se encuentra.
 */
t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(t_list *list_entrada_salida, uint32_t pid);

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

#endif /* KERNEL_ENTRADASALIDA_H_ */
