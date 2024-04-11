#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <unistd.h>
#include <limits.h>
/**
 * @fn    iniciar_config
 * @brief Inicializacion de la config de module.config
 * @param logger Instancia de t_log
 */
t_config *iniciar_config(t_log *logger);

/**
 * @fn    iniciar_logger
 * @brief Inicializacion del logger para los debugs
 * @param nombreDelModulo Es el nombre que le vas a dar al modulo que queres loguear
 */
t_log *iniciar_logger(char *nombreDelModulo);
