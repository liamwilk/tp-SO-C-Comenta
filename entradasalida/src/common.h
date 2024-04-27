#ifndef COMMON_H_
#define COMMON_H_

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

#endif