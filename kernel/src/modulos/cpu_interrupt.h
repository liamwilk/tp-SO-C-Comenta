#ifndef CPU_INTERRUPT_H_
#define CPU_INTERRUPT_H_
#include <commons/log.h>
#include <utils/serial.h>
#include <utils/template.h>
#include "../consola.h"

/**
 * @brief Maneja la interrupción de la CPU realizando diferentes acciones según el código de operación.
 *
 * Esta función se encarga de manejar la interrupción de la CPU realizando diferentes acciones según el código de operación.
 * Toma como parámetros un logger, un código de operación, un puntero a la estructura de argumentos y un buffer.
 *
 * @param logger El logger utilizado para registrar mensajes.
 * @param codigo_operacion El código de operación que indica el tipo de interrupción.
 * @param args Un puntero a la estructura de argumentos.
 * @param buffer El buffer que contiene los datos asociados a la interrupción.
 */
void switch_case_cpu_interrupt(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer);

#endif /* CPU_INTERRUPT_H_ */