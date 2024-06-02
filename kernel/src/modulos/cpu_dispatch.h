#ifndef CPU_DISPATCH_H_
#define CPU_DISPATCH_H_
#include <commons/log.h>
#include <utils/serial.h>
#include <utils/template.h>
#include "../planificacion.h"

/**
 * @brief Maneja el despacho de operaciones de la CPU basado en el código de operación dado.
 *
 * Esta función se encarga de manejar el despacho de operaciones de la CPU basado en el código de operación proporcionado.
 * Toma un logger, un código de operación, argumentos y un buffer como parámetros.
 *
 * @param logger El logger utilizado para registrar cualquier información relevante.
 * @param codigo_operacion El código de operación que determina el tipo de operación de la CPU a realizar.
 * @param args Los argumentos requeridos para la operación de la CPU.
 * @param buffer El buffer utilizado para almacenar los datos relacionados con la operación de la CPU.
 */
void switch_case_cpu_dispatch(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer);

#endif /* CPU_DISPATCH_H_ */