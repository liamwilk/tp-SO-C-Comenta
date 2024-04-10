#include "configs.h"


t_config* iniciar_config(t_log* logger){
	t_config* nuevo_config;

	char *current_dir = getcwd(NULL, 0);

	char ruta_completa[PATH_MAX]; 
    sprintf(ruta_completa, "%s/module.config", current_dir);
	printf("%s",ruta_completa);

	nuevo_config = config_create(ruta_completa);

    if(nuevo_config == NULL){
		log_error(logger,"No se pudo crear la config.");
		perror("No se pudo crear la config.");
	}

	free(current_dir);
	return nuevo_config;
}



t_log* iniciar_logger(char* nombreDelModulo){
	
    t_log* nuevo_logger;

	nuevo_logger = log_create("module.log", nombreDelModulo, true, LOG_LEVEL_DEBUG);

	if (nuevo_logger == NULL) {
		log_error(nuevo_logger, "No se pudo crear el logger.");
		perror("No se puedo crear el logger.");
	}

	return nuevo_logger;
}