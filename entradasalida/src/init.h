#ifndef INIT_H_
#define INIT_H_

#include<commons/config.h>
#include<commons/log.h>
#include<string.h>


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

/**
 * @fn    determinar_tipo_interfaz
 * @brief Determina el tipo de interfaz del modulo que se esta creando
 * @param config Instancia de module.config
 * @return tipo interfaz
 */
t_tipointerfaz determinar_tipo_interfaz(t_config* config);


/*-------Inicializacion-------*/

/**
 * @fn    entradasalida_gen_inicializar
 * @brief Inicializa la entrada-salida generica junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_entradasalida entradasalida_gen_inicializar(t_config *config);

/**
 * @fn    entradasalida_stdin_inicializar
 * @brief Inicializa la entrada-salida STDIN junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_entradasalida entradasalida_stdin_inicializar(t_config *config);

/**
 * @fn    entradasalida_stdout_inicializar
 * @brief Inicializa la entrada-salida STDOUT junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_entradasalida entradasalida_stdout_inicializar(t_config *config);

/**
 * @fn    entradasalida_dialfs_inicializar
 * @brief Inicializa la entrada-salida DIALFS junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_entradasalida entradasalida_dialfs_inicializar(t_config *config);

/*-------Logueo-------*/

/**
 * @fn    entradasalida_gen_log
 * @brief Logs necesarios para entrada-salida generica
 * @param config Instancia de module.config
 */
void entradasalida_gen_log(t_entradasalida entradasalida, t_log *logger);

/**
 * @fn    entradasalida_stdin_log
 * @brief Logs necesarios para entrada-salida STDIN
 * @param config Instancia de module.config
 */
void entradasalida_stdin_log(t_entradasalida entradasalida, t_log *logger);

/**
 * @fn    entradasalida_stdout_log
 * @brief Logs necesarios para entrada-salida STDOUT
 * @param config Instancia de module.config
 */
void entradasalida_stdout_log(t_entradasalida entradasalida, t_log *logger);

/**
 * @fn    entradasalida_dialfs_log
 * @brief Logs necesarios para entrada-salida DIALFS
 * @param config Instancia de module.config
 */
void entradasalida_dialfs_log(t_entradasalida entradasalida, t_log *logger);

#endif