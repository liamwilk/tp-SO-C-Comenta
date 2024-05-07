/* Módulo CPU */

#include "main.h"

int main()
{
	logger = iniciar_logger("cpu", LOG_LEVEL_DEBUG);
	config = iniciar_config(logger);
	cpu = cpu_inicializar(config);
	cpu_log(cpu, logger);

	socket_server_dispatch = iniciar_servidor(logger, cpu.puertoEscuchaDispatch);
	log_debug(logger, "Servidor Dispatch listo para recibir al cliente en socket %d", socket_server_dispatch);

	socket_server_interrupt = iniciar_servidor(logger, cpu.puertoEscuchaInterrupt);
	log_debug(logger, "Servidor Interrupt listo para recibir al cliente en socket %d", socket_server_interrupt);

	pthread_create(&thread_conectar_memoria, NULL, conectar_memoria, NULL);
	pthread_join(thread_conectar_memoria, NULL);

	pthread_create(&thread_atender_memoria, NULL, atender_memoria, NULL);
	pthread_detach(thread_atender_memoria);

	pthread_create(&thread_esperar_kernel_dispatch, NULL, esperar_kernel_dispatch, NULL);
	pthread_join(thread_esperar_kernel_dispatch, NULL);

	pthread_create(&thread_atender_kernel_dispatch, NULL, atender_kernel_dispatch, NULL);
	pthread_detach(thread_atender_kernel_dispatch);

	pthread_create(&thread_esperar_kernel_interrupt, NULL, esperar_kernel_interrupt, NULL);
	pthread_join(thread_esperar_kernel_interrupt, NULL);

	/* Ejemplo de envio de instruccion a Memoria
	sleep(15);

	t_paquete *paquete = crear_paquete(CPU_MEMORIA_PROXIMA_INSTRUCCION);
	t_cpu_memoria_instruccion instruccion;

	instruccion.pid = 1;
	instruccion.program_counter = 17;

	actualizar_buffer(paquete, sizeof(uint32_t) + sizeof(uint32_t));

	serializar_uint32_t(instruccion.program_counter, paquete);

	serializar_uint32_t(instruccion.pid, paquete);

	// Envio la instruccion a Memoria
	enviar_paquete(paquete, socket_memoria);

	log_debug(logger,"Paquete enviado a Memoria");

	// Libero la memoria del paquete de instruccion
	eliminar_paquete(paquete);
	*/
	
	pthread_create(&thread_atender_kernel_interrupt, NULL, atender_kernel_interrupt, NULL);
	pthread_join(thread_atender_kernel_interrupt, NULL);

	log_destroy(logger);
	config_destroy(config);

	return 0;
}

void *conectar_memoria()
{
	socket_memoria = crear_conexion(logger, cpu.ipMemoria, cpu.puertoMemoria);

	if (socket_memoria == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = crear_handshake(logger, socket_memoria, MEMORIA_CPU, "Memoria");

	if (resultado != CORRECTO)
	{
		liberar_conexion(&socket_memoria);
		pthread_exit(0);
	}

	log_debug(logger, "Conectado a Memoria en socket %d", socket_memoria);
	pthread_exit(0);
}

void *atender_memoria()
{	
	hilo_ejecutar(logger, socket_memoria, "Memoria", switch_case_memoria);
	pthread_exit(0);
}

void *esperar_kernel_dispatch()
{
	socket_kernel_dispatch = esperar_conexion(logger, socket_server_dispatch);

	if (socket_kernel_dispatch == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_kernel_dispatch, CPU_DISPATCH_KERNEL, "Kernel por Dispatch");

	if (resultado != CORRECTO)
	{
		liberar_conexion(&socket_kernel_dispatch);
		pthread_exit(0);
	}

	log_debug(logger, "Kernel conectado por Dispatch en socket %d", socket_kernel_dispatch);
	liberar_conexion(&socket_server_dispatch);
	pthread_exit(0);
}

void *atender_kernel_dispatch()
{
	hilo_ejecutar(logger, socket_kernel_dispatch, "Kernel Dispatch", switch_case_kernel_dispatch);
	pthread_exit(0);
}

void *esperar_kernel_interrupt()
{
	socket_kernel_interrupt = esperar_conexion(logger, socket_server_interrupt);

	if (socket_kernel_interrupt == -1)
	{
		pthread_exit(0);
	}

	t_handshake resultado = esperar_handshake(logger, socket_kernel_interrupt, CPU_INTERRUPT_KERNEL, "Kernel por Interrupt");

	if (resultado != CORRECTO)
	{
		liberar_conexion(&socket_kernel_interrupt);
		pthread_exit(0);
	}

	log_debug(logger, "Kernel conectado por Interrupt en socket %d", socket_kernel_interrupt);
	liberar_conexion(&socket_server_interrupt);
	pthread_exit(0);
}

void *atender_kernel_interrupt()
{	
	hilo_ejecutar(logger, socket_kernel_interrupt, "Kernel Interrupt", switch_case_kernel_interrupt);
	pthread_exit(0);
}

t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer)
{
	t_kernel_cpu_proceso *proceso = malloc(sizeof(t_kernel_cpu_proceso));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(proceso->pid));
	deserializar_uint32_t(&stream, &(proceso->registros.pc));
	deserializar_uint32_t(&stream, &(proceso->registros.eax));
	deserializar_uint32_t(&stream, &(proceso->registros.ebx));
	deserializar_uint32_t(&stream, &(proceso->registros.ecx));
	deserializar_uint32_t(&stream, &(proceso->registros.edx));
	deserializar_uint32_t(&stream, &(proceso->registros.si));
	deserializar_uint32_t(&stream, &(proceso->registros.di));
	deserializar_uint8_t(&stream, &(proceso->registros.ax));
	deserializar_uint8_t(&stream, &(proceso->registros.bx));
	deserializar_uint8_t(&stream, &(proceso->registros.cx));
	deserializar_uint8_t(&stream, &(proceso->registros.dx));
	
	return proceso;
}

void switch_case_memoria(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer){
	switch (codigo_operacion)
	{
		case MEMORIA_CPU_PROXIMA_INSTRUCCION:
		{
			// Fetch:
			// Deserializo la instruccion recibida de memoria
			t_memoria_cpu_instruccion *dato = deserializar_t_memoria_cpu_instruccion(buffer);

			log_debug(logger, "Instruccion recibidas de Memoria");
			for(int i = 0; i < dato->cantidad_elementos; i++){
				log_debug(logger, "instruccion[%d]: %s",i,dato->array[i]);
			}

			// Aumento el PC para cuando pida la proxima instruccion
			pc++;

			// Decode:
			// Determino la instruccion que se solicita
			t_instruccion codigo_instruccion = determinar_codigo_instruccion(dato->array[0]);

			// Execute:
			// Ejecuto la instruccion del opcode
			switch(codigo_instruccion)
			{
				case SET:
					log_debug(logger, "reconoci un SET");
					break;

				case SUM:
					log_debug(logger, "reconoci un SUM");
					break;

				case SUB:
					log_debug(logger, "reconoci un SUB");
					break;

				case JNZ:
					log_debug(logger, "reconoci un JNZ");
					break;

				case IO_GEN_SLEEP:
					log_debug(logger, "reconoci un IO_GEN_SLEEP");
					break;

				case EXIT:
					
					// TODO: implementar logica del exit
					// Tiene que cargar todos los registros a un pcb, y mandarselo a kernel
					
					// Liberar

					for(int i = 0; i < dato->cantidad_elementos; i++){
						free(dato->array[i]);
					}
					log_debug(logger,"Se libero la memoria de las instrucciones");

					// Con este break salta directamente a esperar otro paquete de memoria
					break;
				default:
					log_debug(logger, "todavia no entiendo esta operacion");
					break;
			}

			// // Check interrupt
			// if(hay_interrupt)
			// {
			// 	atendiendo_interrupcion = true;
			// 	// Logica
			// } else {
			// 	t_paquete *solicitud_instruccion = crear_paquete(PROXIMA_INSTRUCCION);
			// 	serializar_t_cpu_memoria_instruccion(&solicitud_instruccion, pc, pid);
			// 	enviar_paquete(solicitud_instruccion, socket_memoria);
			// }
			break;
		}
		default:
			{	
			log_warning(logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&socket_memoria);
			break;
		}
	}
}

void switch_case_kernel_dispatch(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer){
	
	switch (codigo_operacion)
		{
		case PLACEHOLDER:
		{
			/*
			La logica
			*/
			break;
		}
		case KERNEL_CPU_ENVIAR_REGISTROS:
		{
			t_kernel_cpu_proceso *proceso_cpu = deserializar_t_kernel_cpu_proceso(buffer);
			log_debug(logger, "Registros cpu recibido de Kernel Dispatch");
			log_debug(logger, "PID: %d", proceso_cpu->pid);
			log_debug(logger, "PC: %d", proceso_cpu->registros.pc);
			log_debug(logger, "EAX: %d", proceso_cpu->registros.eax);
			log_debug(logger, "EBX: %d", proceso_cpu->registros.ebx);
			log_debug(logger, "ECX: %d", proceso_cpu->registros.ecx);
			log_debug(logger, "EDX: %d", proceso_cpu->registros.edx);€
			log_debug(logger, "AX: %d", proceso_cpu->registros.ax);
			log_debug(logger, "BX: %d", proceso_cpu->registros.bx);
			log_debug(logger, "CX: %d", proceso_cpu->registros.cx);
			log_debug(logger, "DX: %d", proceso_cpu->registros.dx);
			log_debug(logger, "SI: %d", proceso_cpu->registros.si);
			log_debug(logger, "DI: %d", proceso_cpu->registros.di);


			// TODO: Continuar con la logica de recibir registros desde kernel
			free(proceso_cpu);
			break;
		}
		default:
			{
			log_warning(logger, "[Kernel Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&socket_kernel_dispatch);
			break;
			}
		}
}

void switch_case_kernel_interrupt(t_log* logger, t_op_code codigo_operacion, t_buffer* buffer){
	
	switch (codigo_operacion)
		{
		case PLACEHOLDER:
		{
			/*
			La logica
			*/
			break;
		}
		case FINALIZAR_SISTEMA:
		{		
			pthread_cancel(thread_atender_memoria);
			pthread_cancel(thread_atender_kernel_dispatch);
			pthread_cancel(thread_atender_kernel_interrupt);
			
			liberar_conexion(&socket_memoria);
			liberar_conexion(&socket_kernel_dispatch);
			liberar_conexion(&socket_kernel_interrupt);

			break;
		}
		default:
			{	
			log_warning(logger, "[Kernel Interrupt] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&socket_kernel_interrupt);
			break;
		}
	}
}

t_instruccion determinar_codigo_instruccion(char* instruccion)
{
	if(!strcmp(instruccion, "SET"))
	{
		return SET;
	}
	if(!strcmp(instruccion, "SUM"))
	{
		return SUM;
	}
	if(!strcmp(instruccion, "SUB"))
	{
		return SUB;
	}
	if(!strcmp(instruccion, "JNZ"))
	{
		return JNZ;
	}
	if(!strcmp(instruccion, "IO_GEN_SLEEP"))
	{
		return IO_GEN_SLEEP;
	}
	if(!strcmp(instruccion, "EXIT"))
	{
		return EXIT;
	}

	// TODO: determinar que devolver si el opcode es cualquier cosa
	return -1;				
};