#ifndef CONFIGS_H_
#define CONFIGS_H_
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <unistd.h>
#include <limits.h>
#include "conexiones.h"

/**
 * @fn    crear_config
 * @brief Inicializacion de la config de module.config
 * @param logger Instancia de t_log
 */
t_config *crear_config(t_log *logger);

void inicializar_config(t_config **config, t_log *logger, int argc, char *argv[]);

t_config *crear_config_parametro(t_log *logger, char *path);

/**
 * @fn    iniciar_logger
 * @brief Inicializacion del logger
 * @param nombreDelModulo Es el nombre que le vas a dar al modulo que queres loguear
 * @param nivel Es el nivel que le asignas al logger
 */
t_log *iniciar_logger(char *nombreDelModulo, t_log_level nivel);

/**
 * @fn    terminar_programa
 * @brief Termina el programa liberando memoria
 * @param conexion Socket de conexion
 * @param logger
 * @param config
 */
void terminar_programa(int conexion, t_log *logger, t_config *config);

#endif