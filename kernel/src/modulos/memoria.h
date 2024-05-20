#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <commons/log.h>
#include <utils/serial.h>
#include <utils/template.h>
#include <utils/kernel.h>
#include "consola.h"
void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer);
bool hilo_planificador_obtener_estado(hilos_args *args);
#endif /* MEMORIA_H_ */
