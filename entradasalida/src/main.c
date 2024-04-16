/* Módulo Entrada-Salida */

#include "main.h"

int main() {
    
    logger = iniciar_logger("entradasalida");
    config = iniciar_config(logger);

    char* ipMemoria = config_get_string_value(config,"IP_MEMORIA");
    int puertoMemoria = config_get_int_value(config,"PUERTO_MEMORIA");
    char* ipKernel = config_get_string_value(config,"IP_KERNEL");
    int puertoKernel = config_get_int_value(config,"PUERTO_KERNEL");
    char* tipoInterfaz = config_get_string_value(config,"TIPO_INTERFAZ");
    int tiempoUnidadDeTrabajo = config_get_int_value(config,"TIEMPO_UNIDAD_TRABAJO");
    char* pathBaseDialFs = config_get_string_value(config,"PATH_BASE_DIALFS");
    int blockSize = config_get_int_value(config,"BLOCK_SIZE");
	int blockCount = config_get_int_value(config,"BLOCK_COUNT");

    log_info(logger,"IP_MEMORIA: %s",ipMemoria);
	log_info(logger,"PUERTO_MEMORIA: %d",puertoMemoria);
	log_info(logger,"IP_KERNEL: %s",ipKernel);
    log_info(logger,"PUERTO_KERNEL: %d",puertoKernel);
    log_info(logger,"TIPO_INTERFAZ: %s",tipoInterfaz);
    log_info(logger,"TIEMPO_UNIDAD_TRABAJO: %d",tiempoUnidadDeTrabajo);
    log_info(logger,"PATH_BASE_DIALFS: %s",pathBaseDialFs);
    log_info(logger,"BLOCK_SIZE: %d",blockSize);
    log_info(logger,"BLOCK_COUNT: %d",blockCount);

	// Conectamos con Memoria y Kernel.

    socket_memoria = crear_conexion(logger,ipMemoria,puertoMemoria);
	log_info(logger,"Conectado a Memoria en socket %d",socket_memoria);

    socket_kernel = crear_conexion(logger,ipKernel,puertoKernel);
	log_info(logger,"Conectado a Kernel en socket %d",socket_kernel);

	/*
	Aca iria la logica de lo que hace I/O una vez que ya tiene las conexiones abiertas con Kernel y Memoria.
	*/

    log_destroy(logger);
	config_destroy(config);

	liberar_conexion(socket_memoria);
	liberar_conexion(socket_kernel);

    return 0;
}
