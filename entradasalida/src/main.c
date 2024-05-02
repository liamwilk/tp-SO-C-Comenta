/* Módulo Entrada-Salida */

#include "main.h"

// ACLARACION: Hay que pasar por parametro un nombre de modulo (string) cuando se lo levanta por consola.

int main(int argc, char *argv[]) {
	
	if(argc<3){
		printf("----------------------------------------------------------------------------------\n");
		printf("Error: No se ha ingresado el formato correcto.\n");
		printf("Se espera que se ingrese el nombre del modulo y el path absoluto del archivo de configuracion.\n");
		printf("Ejemplo desde carpeta entradasalida: ./bin/entradasalida \"GEN\" config/gen.config\n");
		printf("----------------------------------------------------------------------------------\n");
		printf("\n");
		printf("Las interfaces disponibles son:\n");
		printf("└─GEN\n");
		printf("└─STDIN\n");
		printf("└─STDOUT\n");
		printf("└─DIALFS\n");
		printf("Vuelva a ejecutar el programa con el nombre de la interfaz deseada como parametro.\n");
		return EXIT_FAILURE;
	}

	// Obtengo el nombre del modulo.
	char *nombre_modulo = argv[1];
	
	logger = iniciar_logger(nombre_modulo, LOG_LEVEL_INFO);
	config = iniciar_config_entrada_salida(logger, argv[2]);

	log_debug(logger, "Nombre del modulo inicializado: %s\n", nombre_modulo);
    
	t_tipointerfaz tipoInterfaz = determinar_tipo_interfaz(config);

	switch(tipoInterfaz){
		case GEN:
			entradasalida = entradasalida_gen_inicializar(config);
		    entradasalida_gen_log(entradasalida,logger);
			procesar_entradasalida_gen(entradasalida, logger, nombre_modulo);
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
			return EXIT_FAILURE;
	}

	log_warning(logger,"Se cierra modulo I/O.");

    log_destroy(logger);
	config_destroy(config);
	
	liberar_conexion(socket_memoria);
	liberar_conexion(socket_kernel);

	return EXIT_SUCCESS;
}

void* conectar_memoria(){
	socket_memoria = crear_conexion(logger,entradasalida.ipMemoria,entradasalida.puertoMemoria);
	
	if(socket_memoria == -1){
		pthread_exit(0);
	}

	t_handshake resultado = crear_handshake(logger,socket_memoria,MEMORIA_ENTRADA_SALIDA, "Memoria");

	if(resultado != CORRECTO){
		liberar_conexion(socket_memoria);
		pthread_exit(0);
	}

	log_info(logger,"Conectado a Memoria en socket %d",socket_memoria);
	pthread_exit(0);
}

void* conectar_kernel(){
	socket_kernel = crear_conexion(logger,entradasalida.ipKernel,entradasalida.puertoKernel);
	
	if(socket_kernel == -1){
		pthread_exit(0);
	}

	t_handshake resultado = crear_handshake(logger,socket_kernel,KERNEL_ENTRADA_SALIDA, "Kernel");

	if(resultado != CORRECTO){
		liberar_conexion(socket_kernel);
		pthread_exit(0);
	}

	log_info(logger,"Conectado a Kernel en socket %d",socket_kernel);
	pthread_exit(0);
}


void procesar_entradasalida_stdin(t_entradasalida entradasalida,t_log *logger)
{
	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);
}		
void procesar_entradasalida_stdout(t_entradasalida entradasalida,t_log *logger)
{
	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);
}
void procesar_entradasalida_dialfs(t_entradasalida entradasalida,t_log *logger)
{
	pthread_create(&thread_conectar_memoria,NULL,conectar_memoria,NULL);
	pthread_join(thread_conectar_memoria,NULL);
	
	pthread_create(&thread_conectar_kernel,NULL,conectar_kernel,NULL);
	pthread_join(thread_conectar_kernel,NULL);
}