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

/*--------Entrada salida--------*/

typedef struct
{
    char *nombre;
    uint32_t tamanio;
} t_entradasalida_id;

// FUNCIONES DE ENTRADA SALIDA ESTAN EN ENTRADASALIDA/SRC/INIT.H

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
