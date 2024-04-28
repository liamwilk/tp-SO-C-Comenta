#ifndef MODULOS_H_
#define MODULOS_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>

/*--------CPU--------*/

/*Estructura basica de la CPU*/

typedef struct t_cpu
{
    int puertoEscuchaDispatch;
    int puertoEscuchaInterrupt;
    char *ipMemoria;
    int puertoMemoria;
    int cantidadEntradasTlb;
    char *algoritmoTlb;
} t_cpu;

/**
 * @fn    cpu_inicializar
 * @brief Inicializa el cpu junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_cpu cpu_inicializar(t_config *config);

/**
 * @fn    cpu_log
 * @brief Logs necesarios para cpu
 * @param config Instancia de module.config
 * @return t_cpu
 */
void cpu_log(t_cpu cpu, t_log *logger);

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
} t_entradasalida;

/**
 * @fn    entradasalida_inicializar
 * @brief Inicializa la entrada-salida junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_entradasalida entradasalida_inicializar(t_config *config);

/**
 * @fn    entradasalida_log
 * @brief Logs necesarios para entrada-salida
 * @param config Instancia de module.config
 */
void entradasalida_log(t_entradasalida entradasalida, t_log *logger);

/*--------Memoria--------*/
typedef struct t_memoria
{
    int tamMemoria, tamPagina, retardoRespuesta, puertoEscucha;
    char *pathInstrucciones;
} t_memoria;

/**
 * @fn    memoria_inicializar
 * @brief Inicializa la memoria junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_memoria memoria_inicializar(t_config *config);

/**
 * @fn    memoria_log
 * @brief Logs necesarios para memoria
 * @param config Instancia de module.config
 */
void memoria_log(t_memoria memoria, t_log *logger);

#endif /* MODULOS_H_ */
