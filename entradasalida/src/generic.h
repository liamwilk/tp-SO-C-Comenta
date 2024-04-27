#ifndef GENERIC_H_
#define GENERIC_H_

#include<commons/log.h>
#include <commons/process.h>
#include <utils/conexiones.h>
#include <utils/handshake.h>
#include <utils/serial.h>
#include "common.h"

typedef enum {
    IO_GEN_SLEEP
} opcode_gen;

void* procesar_entradasalida_gen(t_entradasalida entradasalida, t_log *logger);

#endif