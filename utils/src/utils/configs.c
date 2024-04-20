#include "configs.h"

t_config *iniciar_config(t_log *logger)
{
	t_config *nuevo_config;

	char *current_dir = getcwd(NULL, 0);

	char ruta_completa[PATH_MAX];
	sprintf(ruta_completa, "%s/module.config", current_dir);

	nuevo_config = config_create(ruta_completa);

	if (nuevo_config == NULL)
	{
		log_error(logger, "No se pudo crear la config.");
		perror("No se pudo crear la config.");
	}

	free(current_dir);
	return nuevo_config;
}

t_log *iniciar_logger(char *nombreDelModulo , t_log_level nivel)
{

	char* name_logfile;

	switch (nivel)
	{
	case LOG_LEVEL_DEBUG:
		name_logfile = "debug.log";
		break;

	case LOG_LEVEL_WARNING:
		name_logfile = "warning.log";
		break;

	case LOG_LEVEL_ERROR:
		name_logfile = "error.log";
		break;
	
	case LOG_LEVEL_INFO:
		name_logfile = "module.log";
		break;
	
	case LOG_LEVEL_TRACE:
		name_logfile = "trace.log";
		break;

	default:
		name_logfile = "module.log";
		break;
	}

	t_log *nuevo_logger;

	nuevo_logger = log_create(name_logfile, nombreDelModulo, true, nivel);

	if (nuevo_logger == NULL)
	{
		log_error(nuevo_logger, "No se pudo crear el logger.");
		perror("No se puedo crear el logger.");
	}

	return nuevo_logger;
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
	if (logger != NULL)
	{
		log_destroy(logger);
	}

	if (config != NULL)
	{
		config_destroy(config);
	}

	liberar_conexion(conexion);
}
