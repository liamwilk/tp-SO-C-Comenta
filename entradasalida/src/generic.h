#ifndef GENERIC_H_
#define GENERIC_H_

#include <commons/log.h>
#include <commons/process.h>
#include <utils/conexiones.h>
#include <utils/handshake.h>
#include <utils/serial.h>
#include "common.h"

void procesar_entradasalida_gen(t_entradasalida entradasalida, t_log *logger, char *nombre_modulo);
uint32_t deserializar_unidades_de_trabajo(t_paquete *paquete);

#endif