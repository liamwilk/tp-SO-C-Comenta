/* Módulo Entrada-Salida */

#include "main.h"

int main(int argc, char *argv[]) {
	
	if(argc<3){

		if(argc==1){
			printf("-----------------------------------------------------------------------------------\n");
			printf("Error: No se ha ingresado el nombre del modulo ni path al archivo de configuración.\n");
			printf("Ejemplo desde carpeta entradasalida: ./bin/entradasalida \"GEN\" config/gen.config\n");
			printf("-----------------------------------------------------------------------------------\n");
			printf("\n");
			printf("Las interfaces disponibles son:\n");
			printf("└─GEN\n");
			printf("└─STDIN\n");
			printf("└─STDOUT\n");
			printf("└─DIALFS\n");
			printf("Vuelva a intentar, por favor.\n");
		}

		if (argc==2){
			printf("----------------------------------------------------------------------------------\n");
			printf("Error: No se ha ingresado el path al archivo de configuración.\n");
			printf("Ejemplo desde carpeta entradasalida: ./bin/entradasalida \"GEN\" config/gen.config\n");
			printf("----------------------------------------------------------------------------------\n");
			printf("\n");
			printf("Las interfaces disponibles son:\n");
			printf("└─GEN\n");
			printf("└─STDIN\n");
			printf("└─STDOUT\n");
			printf("└─DIALFS\n");
			printf("Vuelva a intentar, por favor.\n");
		}
		return EXIT_FAILURE;
	}

	// Obtengo el nombre del modulo.
	nombre_modulo = strdup(argv[1]);
	
	logger = iniciar_logger(nombre_modulo, LOG_LEVEL_DEBUG);
	config = iniciar_config_entrada_salida(logger, argv[2]);

	log_debug(logger, "Nombre del modulo inicializado: %s\n", nombre_modulo);
    
	t_tipointerfaz tipoInterfaz = determinar_tipo_interfaz(config);

	switch(tipoInterfaz){
		case GEN:
			pthread_t thread_gen;
			pthread_create(&thread_gen,NULL,atender_entrada_salida_generic,NULL);
			pthread_join(thread_gen,NULL);
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

	log_info(logger,"Se cierra el modulo %s",nombre_modulo);
	free(nombre_modulo);
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

void* atender_entrada_salida_generic()
{	
	t_entradasalida package = entradasalida_gen_inicializar(config);
	entradasalida_gen_log(entradasalida,logger);

    log_debug(logger, "Iniciando modulo %s", nombre_modulo);

    log_debug(logger, "Conectando a Kernel en %s:%d", package.ipKernel, package.puertoKernel);
    int socket_kernel = crear_conexion(logger, package.ipKernel, package.puertoKernel);
    
    if (socket_kernel == -1)
    {
        log_error(logger, "No se pudo crear la conexion con Kernel.");
        pthread_exit(0);
    }
    
    t_handshake resultado = crear_handshake(logger, socket_kernel, KERNEL_ENTRADA_SALIDA_GENERIC, "Kernel");
    
    if (resultado != CORRECTO)
    {
        log_error(logger, "No se pudo realizar el handshake con Kernel.");
        liberar_conexion(socket_kernel);
        pthread_exit(0);
    }

    log_debug(logger, "Conectado a Kernel en socket %d", socket_kernel);

    int seguir_ejecutando = 1;

    while (seguir_ejecutando)
    {
        log_debug(logger, "Esperando paquete de Kernel en socket %d", socket_kernel);
        t_paquete *paquete = recibir_paquete(logger, socket_kernel);

        if (paquete == NULL)
        {
            log_error(logger, "Kernel se desconecto del socket %d", socket_kernel);
            break;
        }
        
        switch (paquete->codigo_operacion)
        {
            case IO_GEN_SLEEP:
            {   
                revisar_paquete(paquete, logger, seguir_ejecutando, nombre_modulo);

                uint32_t unidades_de_trabajo = deserializar_unidades_de_trabajo(paquete->buffer);

                log_info(logger, "PID : <%u> - Operacion a realizar: IO_GEN_SLEEP", process_getpid());
                                
                log_debug(logger, "Durmiendo %d unidades de trabajo", unidades_de_trabajo);
                usleep(unidades_de_trabajo * package.tiempoUnidadDeTrabajo);

                t_paquete *aviso_sleep = crear_paquete(IO_GEN_SLEEP_TERMINADO);
                enviar_paquete(aviso_sleep, socket_kernel);
            
                eliminar_paquete(aviso_sleep);
                break;
            }
			case DESCONECTAR:
			{
				log_info(logger, "Se recibio la señal de desconexión. Cierro hilo");
				seguir_ejecutando = 0;
				liberar_conexion(socket_kernel);
				break;
			}
            default:
            {	
			log_warning(logger, "[Kernel] Se recibio un codigo de operacion desconocido. Cierro hilo");
			eliminar_paquete(paquete);
			liberar_conexion(socket_kernel);
			pthread_exit(0);
			}
        }
        eliminar_paquete(paquete);
    }
	pthread_exit(0);
}

uint32_t deserializar_unidades_de_trabajo(t_buffer *buffer)
{
    uint32_t dato;
	void *stream = buffer->stream;
    
    deserializar_uint32_t(&stream, &dato);
    return dato;
}