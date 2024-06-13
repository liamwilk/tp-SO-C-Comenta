#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <commons/log.h>
#include <utils/serial.h>
#include <utils/template.h>
#include <utils/kernel/main.h>
#include <utils/kernel/consola.h>
#include "planificacion.h"

/**
 * @brief Función que realiza un switch case para ejecutar diferentes operaciones de memoria.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param codigo_operacion Código de la operación a ejecutar.
 * @param args Argumentos del hilo.
 * @param buffer Puntero al buffer utilizado para enviar y recibir datos.
 */
void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer);

/**
 * @brief Función que obtiene el estado del hilo planificador.
 *
 * @param args Argumentos del hilo.
 * @return true si el hilo planificador está en estado activo, false de lo contrario.
 */
bool hilo_planificador_obtener_estado(hilos_args *args);

#endif /* MEMORIA_H_ */
