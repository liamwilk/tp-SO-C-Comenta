#include "init.h"

t_tipointerfaz determinar_tipo_interfaz(t_config* config){
	char* tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
	if(!strcmp(tipoInterfaz, "GEN")){
        return GEN;
	}
	if(!strcmp(tipoInterfaz, "STDOUT")){
        return STDOUT;
	}
	if(!strcmp(tipoInterfaz, "STDIN")){
        return STDIN;
	}
	if(!strcmp(tipoInterfaz, "DIALFS")){
        return DIALFS;
	}
    free(tipoInterfaz);
	return -1;
};

t_entradasalida entradasalida_gen_inicializar(t_config *config)
{
    t_entradasalida ret;
    
    ret.tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    ret.ipKernel = config_get_string_value(config, "IP_KERNEL");
    ret.puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
    ret.tiempoUnidadDeTrabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    return ret;
};

t_entradasalida entradasalida_stdin_inicializar(t_config *config)
{
    t_entradasalida ret;

    ret.tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    ret.ipKernel = config_get_string_value(config, "IP_KERNEL");
    ret.puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
    ret.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    ret.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");

    return ret;
};

t_entradasalida entradasalida_stdout_inicializar(t_config *config)
{
    t_entradasalida ret;

    ret.tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    ret.tiempoUnidadDeTrabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    ret.ipKernel = config_get_string_value(config, "IP_KERNEL");
    ret.puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
    ret.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    ret.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");

    return ret;
};

t_entradasalida entradasalida_dialfs_inicializar(t_config *config)
{
    t_entradasalida ret;

    ret.tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    ret.tiempoUnidadDeTrabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    ret.ipKernel = config_get_string_value(config, "IP_KERNEL");
    ret.puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
    ret.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    ret.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");    
    ret.pathBaseDialFs = config_get_string_value(config, "PATH_BASE_DIALFS");
    ret.blockCount = config_get_int_value(config, "BLOCK_COUNT");
    ret.blockSize = config_get_int_value(config, "BLOCK_SIZE");
    ret.retrasoCompactacion = config_get_int_value(config, "RETRASO_COMPACTACION");

    return ret;
}

void entradasalida_gen_log(t_entradasalida entradasalida, t_log *logger)
{
    log_info(logger, "TIPO_INTERFAZ: GEN");
    log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", entradasalida.tiempoUnidadDeTrabajo);
    log_info(logger, "IP_KERNEL: %s", entradasalida.ipKernel);
    log_info(logger, "PUERTO_KERNEL: %d", entradasalida.puertoKernel);    
}

void entradasalida_stdin_log(t_entradasalida entradasalida, t_log *logger)
{
    log_info(logger, "TIPO_INTERFAZ: STDIN");
    log_info(logger, "IP_KERNEL: %s", entradasalida.ipKernel);
    log_info(logger, "PUERTO_KERNEL: %d", entradasalida.puertoKernel);
    log_info(logger, "IP_MEMORIA: %s", entradasalida.ipMemoria);
    log_info(logger, "PUERTO_MEMORIA: %d", entradasalida.puertoMemoria);    
}

void entradasalida_stdout_log(t_entradasalida entradasalida, t_log *logger)
{
    log_info(logger, "TIPO_INTERFAZ: STDOUT");
    log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", entradasalida.tiempoUnidadDeTrabajo);    
    log_info(logger, "IP_KERNEL: %s", entradasalida.ipKernel);
    log_info(logger, "PUERTO_KERNEL: %d", entradasalida.puertoKernel);
    log_info(logger, "IP_MEMORIA: %s", entradasalida.ipMemoria);
    log_info(logger, "PUERTO_MEMORIA: %d", entradasalida.puertoMemoria);     
}

void entradasalida_dialfs_log(t_entradasalida entradasalida, t_log *logger)
{
    log_info(logger, "TIPO_INTERFAZ: DIALFS");
    log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", entradasalida.tiempoUnidadDeTrabajo);    
    log_info(logger, "IP_KERNEL: %s", entradasalida.ipKernel);
    log_info(logger, "PUERTO_KERNEL: %d", entradasalida.puertoKernel);
    log_info(logger, "IP_MEMORIA: %s", entradasalida.ipMemoria);
    log_info(logger, "PUERTO_MEMORIA: %d", entradasalida.puertoMemoria);
    log_info(logger, "PATH_BASE_DIALFS: %s", entradasalida.pathBaseDialFs);
    log_info(logger, "BLOCK_SIZE: %d", entradasalida.blockSize);
    log_info(logger, "BLOCK_COUNT: %d", entradasalida.blockCount);
    log_info(logger, "RETRASO_COMPACTACION: %d", entradasalida.retrasoCompactacion);    
}

// void entradasalida_log(t_entradasalida entradasalida, t_log *logger, t_tipointerfaz tipoInterfaz)
// {
//     log_info(logger, "TIPO_INTERFAZ: %s", entradasalida.tipoInterfaz);
//     log_info(logger, "IP_KERNEL: %s", entradasalida.ipKernel);
//     log_info(logger, "PUERTO_KERNEL: %d", entradasalida.puertoKernel);

//     switch (tipoInterfaz)
//     {
//     case GEN:
//         log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", entradasalida.tiempoUnidadDeTrabajo);
//         break;
//     case STDIN:
//         log_info(logger, "IP_MEMORIA: %s", entradasalida.ipMemoria);
//         log_info(logger, "PUERTO_MEMORIA: %d", entradasalida.puertoMemoria);
//         break;
//     case STDOUT:
//         log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", entradasalida.tiempoUnidadDeTrabajo);
//         log_info(logger, "IP_MEMORIA: %s", entradasalida.ipMemoria);
//         log_info(logger, "PUERTO_MEMORIA: %d", entradasalida.puertoMemoria);
//         break;
//     case DIALFS:
//         log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", entradasalida.tiempoUnidadDeTrabajo);
//         log_info(logger, "IP_MEMORIA: %s", entradasalida.ipMemoria);
//         log_info(logger, "PUERTO_MEMORIA: %d", entradasalida.puertoMemoria);
//         log_info(logger, "PATH_BASE_DIALFS: %s", entradasalida.pathBaseDialFs);
//         log_info(logger, "BLOCK_SIZE: %d", entradasalida.blockSize);
//         log_info(logger, "BLOCK_COUNT: %d", entradasalida.blockCount);
//         log_info(logger, "RETRASO_COMPACTACION: %d", entradasalida.retrasoCompactacion);
//         break;
//     default:
//         break;
//     }
// };
