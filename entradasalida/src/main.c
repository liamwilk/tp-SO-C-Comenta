/* Módulo Entrada-Salida */

#include "main.h"

int main() {
    
    logger = iniciar_logger("entradasalida" , LOG_LEVEL_INFO);
    config = iniciar_config(logger);

	t_tipointerfaz tipoInterfaz = determinar_tipo_interfaz(config);

	switch(tipoInterfaz){
		case GEN:
			entradasalida = entradasalida_gen_inicializar(config);
		    entradasalida_gen_log(entradasalida,logger);
			procesar_entradasalida_gen(entradasalida, logger);
			break;
		case STDIN:
			entradasalida = entradasalida_stdin_inicializar(config);
		    entradasalida_stdin_log(entradasalida,logger);
			procesar_entradasalida_stdin(entradasalida, logger);		
			break;
		case STDOUT:
			entradasalida = entradasalida_stdout_inicializar(config);
		    entradasalida_stdout_log(entradasalida,logger);
			procesar_entradasalida_stdout(entradasalida, logger);
			break;		
		case DIALFS:
			entradasalida = entradasalida_dialfs_inicializar(config);
		    entradasalida_dialfs_log(entradasalida,logger);		
			procesar_entradasalida_dialfs(entradasalida, logger);
			break;
		default:
			log_error(logger, "Tipo de interfaz desconocida");
			return 0;
	}


	// TODO: Estaria bueno cambiar esto, para tener el switch antes de las 2 funciones de arriba,
	// y que ese switch inicialize ese tipo, lo loguee, y mande el hilo a procesar los mensajes que reciba.

	// AKA, Movete todas las funciones y las structs desde modulos.c/.h a un init.c/.h

	log_warning(logger,"Se cierra modulo I/O.");

    log_destroy(logger);
	config_destroy(config);

	liberar_conexion(socket_memoria);
	liberar_conexion(socket_kernel);

	return 0;
}

void* conectar_memoria(){
	socket_memoria = crear_conexion(logger,entradasalida.ipMemoria,entradasalida.puertoMemoria);
	handshake(logger,socket_memoria,1,"Memoria");
	log_info(logger,"Conectado a Memoria en socket %d",socket_memoria);
	pthread_exit(0);
}

void* conectar_kernel(){
	socket_kernel = crear_conexion(logger,entradasalida.ipKernel,entradasalida.puertoKernel);
	handshake(logger,socket_kernel,1,"Kernel");
	log_info(logger,"Conectado a Kernel en socket %d",socket_kernel);
	pthread_exit(0);
}


void* procesar_entradasalida_stdin(t_entradasalida entradasalida,t_log *logger)
{

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);
	return NULL;
}		
void* procesar_entradasalida_stdout(t_entradasalida entradasalida,t_log *logger)
{

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);
	return NULL;
}
void* procesar_entradasalida_dialfs(t_entradasalida entradasalida,t_log *logger)
{

	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);
	return NULL;
}