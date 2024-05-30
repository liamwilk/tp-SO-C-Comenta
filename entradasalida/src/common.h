#ifndef COMMON_H_
#define COMMON_H_

#include <utils/serial.h>
#include<commons/string.h>
#include <string.h>

/* Archivo para definiciones de structs comunes al modulo de I/O */

typedef struct t_entradasalida
{
    char *ipMemoria;
    int puertoMemoria;
    char *ipKernel;
    int puertoKernel;
    char *tipoInterfaz;
    int tiempoUnidadDeTrabajo;
    char *pathBaseDialFs;
    int blockSize;
    int blockCount;
    int retrasoCompactacion;
} t_entradasalida;

typedef enum
{
    GEN,
    STDOUT,
    STDIN,
    DIALFS
} t_tipointerfaz;

typedef struct{
    uint32_t size_nombre;
    char* nombre;
} t_entradasalida_identificador;

/**
 * Identifica el módulo con el nombre dado y envía la información al kernel a través del socket especificado.
 *
 * @param nombre_modulo El nombre del módulo a identificar.
 * @param socket_kernel El socket al que se enviará la información de identificación.
 */
void identificar_modulo(char* nombre_modulo, int socket_kernel);

#endif