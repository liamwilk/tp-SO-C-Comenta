#include "configs.h"

t_config *crear_config(t_log *logger)
{
	char *current_dir = getcwd(NULL, 0);

	char ruta_completa[PATH_MAX];
	sprintf(ruta_completa, "%s/module.config", current_dir);
	t_config *nuevo_config = config_create(ruta_completa);

	if (nuevo_config == NULL)
	{
		log_error(logger, "No se pudo crear la config.");
		perror("No se pudo crear la config.");
	}

	free(current_dir);
	return nuevo_config;
}

t_config *crear_config_parametro(t_log *logger, char *path)
{
	char *current_dir = getcwd(NULL, 0);

	char ruta_completa[PATH_MAX];
	sprintf(ruta_completa, "%s/%s", current_dir, path);

	t_config *nuevo_config = config_create(path);

	if (nuevo_config == NULL)
	{
		log_error(logger, "No se pudo crear la config.");
	}

	return nuevo_config;
}

void inicializar_config(t_config **config, t_log *logger, int argc, char *argv[])
{
	if (argc != 2)
	{
		log_debug(logger, "No se introdujo config personalizada. Se utiliza default 'module.config'");
		*config = crear_config(logger);
	}
	else
	{
		*config = crear_config_parametro(logger, argv[1]);
	}
}

t_log *iniciar_logger(char *nombreDelModulo, t_log_level nivel)
{

	t_log *nuevo_logger;

	nuevo_logger = log_create("module.log", nombreDelModulo, true, nivel);

	if (nuevo_logger == NULL)
	{
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

	liberar_conexion(&conexion);
}
