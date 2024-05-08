#include "modulos.h"

/*--------CPU--------*/

/*--------MEMORIA--------*/
t_memoria memoria_inicializar(t_config *config)
{
    t_memoria memoria;
    memoria.puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    memoria.tamMemoria = config_get_int_value(config, "TAM_MEMORIA");
    memoria.tamPagina = config_get_int_value(config, "TAM_PAGINA");
    memoria.pathInstrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    memoria.retardoRespuesta = config_get_int_value(config, "RETARDO_RESPUESTA");
    return memoria;
};

void memoria_log(t_memoria memoria, t_log *logger)
{
    log_info(logger, "PUERTO_ESCUCHA: %d", memoria.puertoEscucha);
    log_info(logger, "TAM_MEMORIA: %d", memoria.tamMemoria);
    log_info(logger, "TAM_PAGINA: %d", memoria.tamPagina);
    log_info(logger, "PATH_INSTRUCCIONES: %s", memoria.pathInstrucciones);
    log_info(logger, "RETARDO_RESPUESTA: %d", memoria.retardoRespuesta);
    printf("\n");
};
