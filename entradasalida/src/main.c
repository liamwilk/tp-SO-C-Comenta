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

	nombre_modulo = strdup(argv[1]);
	
	logger = iniciar_logger(nombre_modulo, LOG_LEVEL_DEBUG);
	config = iniciar_config_entrada_salida(logger, argv[2]);

	log_debug(logger, "Nombre del modulo inicializado: %s\n", nombre_modulo);
    
	t_tipointerfaz tipoInterfaz = determinar_tipo_interfaz(config);

	switch(tipoInterfaz){
		case GEN:
			procesar_entradasalida_generic();
			break;
		case STDIN:
			procesar_entradasalida_stdin();		
			break;
		case STDOUT:
			procesar_entradasalida_stdout();
			break;		
		case DIALFS:
			procesar_entradasalida_dialfs();
			break;
		default:
			log_error(logger, "Se introdujo por parametro un tipo de interfaz desconocida. ");
			break;
	}

	log_info(logger,"Se cierra el modulo I/O %s",nombre_modulo);

	free(nombre_modulo);
    log_destroy(logger);
	config_destroy(config);

	return EXIT_SUCCESS;
}

void* conectar_memoria_stdin(){
	conexion_crear(logger,entradasalida.ipMemoria,entradasalida.puertoMemoria,&socket_memoria,"Memoria",MEMORIA_ENTRADA_SALIDA_STDIN);
	pthread_exit(0);
}

void* conectar_kernel_stdin(){
	conexion_crear(logger,entradasalida.ipKernel,entradasalida.puertoKernel,&socket_kernel_stdin,"Kernel",KERNEL_ENTRADA_SALIDA_STDIN);
	pthread_exit(0);
}

void procesar_entradasalida_generic()
{
	entradasalida = entradasalida_generic_inicializar(config);
	entradasalida_generic_log(entradasalida,logger);

	pthread_create(&thread_conectar_kernel_generic,NULL,conectar_kernel_generic,NULL);
	pthread_join(thread_conectar_kernel_generic,NULL);

	pthread_create(&thread_atender_kernel_generic,NULL,atender_kernel_generic,NULL);
	pthread_join(thread_atender_kernel_generic,NULL);
}		

void procesar_entradasalida_stdin()
{

	// Cuando haga handhsake con Memoria, tiene que mandar el codigo de operacion MEMORIA_ENTRADA_SALIDA_STDIN
	// Cuando haga handhsake con Kernel, tiene que mandar el codigo de operacion KERNEL_ENTRADA_SALIDA_STDIN

	entradasalida = entradasalida_stdin_inicializar(config);
	entradasalida_stdin_log(entradasalida,logger);

	pthread_create(&thread_conectar_memoria_stdin,NULL,conectar_memoria_stdin,NULL);
	pthread_join(thread_conectar_memoria_stdin,NULL);

	// TODO: Falta el servidor que atiende las solicitudes de Memoria

	pthread_create(&thread_conectar_kernel_stdin,NULL,conectar_kernel_stdin,NULL);
	pthread_join(thread_conectar_kernel_stdin,NULL);

	// TODO: Falta el servidor que atiende las solicitudes de Kernel
}		

void procesar_entradasalida_stdout()
{
	// Cuando haga handhsake con Memoria, tiene que mandar el codigo de operacion MEMORIA_ENTRADA_SALIDA_STDIN
	// Cuando haga handhsake con Kernel, tiene que mandar el codigo de operacion KERNEL_ENTRADA_SALIDA_STDIN

	entradasalida = entradasalida_stdout_inicializar(config);
	entradasalida_stdout_log(entradasalida,logger);

	// pthread_create(&thread_conectar_memoria_stdout,NULL,conectar_memoria_stdout,NULL);
	// pthread_join(thread_conectar_memoria_stdout,NULL);

	// TODO: Falta el servidor que atiende las solicitudes de Memoria

	// pthread_create(&thread_conectar_kernel_stdout,NULL,conectar_kernel_stdout,NULL);
	// pthread_join(thread_conectar_kernel_stdout,NULL);

	// TODO: Falta el servidor que atiende las solicitudes de Kernel
}		

void procesar_entradasalida_dialfs()
{
	// Cuando haga handhsake con Memoria, tiene que mandar el codigo de operacion MEMORIA_ENTRADA_SALIDA_STDIN
	// Cuando haga handhsake con Kernel, tiene que mandar el codigo de operacion KERNEL_ENTRADA_SALIDA_STDIN

	entradasalida = entradasalida_dialfs_inicializar(config);
	entradasalida_dialfs_log(entradasalida,logger);	

	// pthread_create(&thread_conectar_memoria_dialfs,NULL,conectar_memoria_dialfs,NULL);
	// pthread_join(thread_conectar_memoria_dialfs,NULL);

	// TODO: Falta el servidor que atiende las solicitudes de Memoria

	// pthread_create(&thread_conectar_kernel_dialfs,NULL,conectar_kernel_dialfs,NULL);
	// pthread_join(thread_conectar_kernel_dialfs,NULL);

	// TODO: Falta el servidor que atiende las solicitudes de Kernel
}		

void* conectar_kernel_generic(){

	conexion_crear(logger,entradasalida.ipKernel,entradasalida.puertoKernel,&socket_kernel_generic,"Kernel",KERNEL_ENTRADA_SALIDA_GENERIC);
	pthread_exit(0);
}

void *atender_kernel_generic()
{
	while (1)
	{
		pthread_testcancel();
		
		log_debug(logger, "Esperando paquete de Kernel en socket %d", socket_kernel_generic);

		t_paquete *paquete = recibir_paquete(logger, socket_kernel_generic);
		
		if (paquete == NULL)
        {
            log_info(logger, "Kernel se desconecto del socket %d.", socket_kernel_generic);
			break;
        }
		
		revisar_paquete(paquete, logger, nombre_modulo);

		switch (paquete->codigo_operacion)
		{
			case KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP:
            {   
                
                t_kernel_entrada_salida_unidad_de_trabajo* unidades = deserializar_t_kernel_entrada_salida_unidad_de_trabajo(paquete->buffer);

                log_info(logger, "PID : <%u> - Operacion a realizar: KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP", process_getpid());
                                
                log_debug(logger, "Durmiendo %d unidades de trabajo", unidades->unidad_de_trabajo);
                usleep(unidades->unidad_de_trabajo * entradasalida.tiempoUnidadDeTrabajo);

                t_paquete *aviso_sleep = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP);
				
				t_entrada_salida_kernel_unidad_de_trabajo* unidad = malloc(sizeof(t_entrada_salida_kernel_unidad_de_trabajo));
				unidad->terminado = true;

				serializar_t_entrada_salida_kernel_unidad_de_trabajo(&aviso_sleep, unidad);

                enviar_paquete(aviso_sleep, socket_kernel_generic);

				log_debug(logger, "Se envió aviso a Kernel de que termino el sleep.");

				free(unidad);
                eliminar_paquete(aviso_sleep);
                break;
            }
			case FINALIZAR_SISTEMA:
			{
				log_info(logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
				pthread_cancel(thread_atender_kernel_generic);
				liberar_conexion(&socket_kernel_generic);
				break;
			}
			default:
			{
				log_warning(logger, "[Kernel] Se recibio un codigo de operacion desconocido. Cierro hilo");
				eliminar_paquete(paquete);
				liberar_conexion(&socket_kernel_generic);
				pthread_exit(0);
			}
			
		}
		eliminar_paquete(paquete);
	}
	pthread_exit(0);
}