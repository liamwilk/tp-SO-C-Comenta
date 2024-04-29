#ifndef GENERIC_H_
#define GENERIC_H_

#include<commons/log.h>
#include <commons/process.h>
#include <utils/conexiones.h>
#include <utils/handshake.h>
#include <utils/serial.h>
#include "common.h"

typedef enum {
    IO_GEN_SLEEP,
    IO_GEN_SLEEP_TERMINADO
} opcode_gen;

// Aviso de sleep terminado -> paquete con solamente un opcode

// Aviso de instruccion no admitida -> paquete con un opcode

// Nombre -> struct con el tamanio del nombre y el nombre
typedef struct{
    uint32_t size_nombre;
    char* nombre;
} t_entradasalida_identificador;

void* procesar_entradasalida_gen(t_entradasalida entradasalida, t_log *logger);

#endif