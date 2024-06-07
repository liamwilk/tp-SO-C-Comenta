#include <utils/cpu.h>

void switch_case_memoria(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_CPU_IO_MOV_IN_2:
	{
		t_mov_in *proceso_recibido = deserializar_t_mov_in(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de marco asociada a la instruccion <MOV_IN> para el proceso PID <%d>", proceso_recibido->pid);

		// Si se obtuvo el dato
		if (proceso_recibido->resultado)
		{
			// Determino que tipo de registros debo operar
			char *registro_datos = strdup(args->instruccion.array[1]);

			// Guardo el dato en el registro correspondiente			
			if (proceso_recibido->tamanio_registro_datos == 4)
			{
				log_debug(args->logger, "Se obtuvo el dato <%d> del marco <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->dato_32, proceso_recibido->numero_marco, proceso_recibido->pid);
				uint32_t *registro_datos_ptr = determinar_tipo_registro_uint32_t(registro_datos, &args->proceso);
				*registro_datos_ptr = proceso_recibido->dato_32;
				log_debug(args->logger, "Se guardo el dato <%d> en el registro <%s> del proceso PID <%d>", proceso_recibido->dato_32, args->instruccion.array[1], proceso_recibido->pid);
				imprimir_registros(args);
			}
			else
			{
				log_debug(args->logger, "Se obtuvo el dato <%d> del marco <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->dato_8, proceso_recibido->numero_marco, proceso_recibido->pid);
				uint8_t *registro_datos_ptr = determinar_tipo_registro_uint8_t(registro_datos, &args->proceso);
				*registro_datos_ptr = proceso_recibido->dato_8;
				log_debug(args->logger, "Se guardo el dato <%d> en el registro <%s> del proceso PID <%d>", proceso_recibido->dato_8, args->instruccion.array[1], proceso_recibido->pid);
				imprimir_registros(args);
			}

			free(proceso_recibido);

			// Reanudo el ciclo de ejecucion por finalizacion de la instruccion atomica MOV_IN
			if (args->flag_interrupt)
			{
				instruccion_interrupt(args);
				break;
			}

			instruccion_solicitar(args);
		}
		else // Si no se obtuvo el dato
		{
			log_error(args->logger, "No se pudo obtener el dato del marco <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->numero_marco, proceso_recibido->pid);

			free(proceso_recibido);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);
		}
		break;
	}
	case MEMORIA_CPU_IO_MOV_IN:
	{
		t_mov_in *proceso_recibido = deserializar_t_mov_in(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de marco asociada a la instruccion <MOV_IN> para el proceso PID <%d>", proceso_recibido->pid);

		// Si se obtuvo el marco
		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se obtuvo el marco <%d> de la pagina <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->numero_marco, proceso_recibido->numero_pagina, proceso_recibido->pid);

			// Genero la direccion fisica con la MMU
			uint32_t direccion_fisica = mmu(args, proceso_recibido->registro_direccion, proceso_recibido->numero_marco);

			// Inicio la peticion contra Memoria para que lea
			t_paquete *paquete = crear_paquete(CPU_MEMORIA_MOV_IN_2);
			t_mov_in *proceso_completo = malloc(sizeof(t_mov_in));

			proceso_completo->pid = proceso_recibido->pid;
			proceso_completo->resultado = 1;
			proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
			proceso_completo->registro_datos = proceso_recibido->registro_datos;
			proceso_completo->tamanio_registro_datos = proceso_recibido->tamanio_registro_datos;
			proceso_completo->numero_marco = proceso_recibido->numero_marco;
			proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
			proceso_completo->direccion_fisica = direccion_fisica;
			proceso_completo->dato_32 = proceso_recibido->dato_32;
			proceso_completo->dato_8 = proceso_recibido->dato_8;

			serializar_t_mov_in(&paquete, proceso_completo);
			enviar_paquete(paquete, args->config_leida.socket_memoria);
			eliminar_paquete(paquete);

			log_debug(args->logger, "Se envio la solicitud de la instruccion <MOV_IN> del proceso PID <%d> a Kernel", proceso_recibido->pid);

			free(proceso_completo);
		}
		else // Si no se obtuvo el marco
		{
			log_error(args->logger, "No se pudo obtener el marco de la pagina <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->numero_pagina, proceso_recibido->pid);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);
		}

		free(proceso_recibido);

		break;
	}
	case MEMORIA_CPU_IO_STDOUT_WRITE:
	{
		t_io_stdout_write *proceso_recibido = deserializar_t_io_stdout_write(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de marco asociada a la instruccion <IO_STDOUT_WRITE> para el proceso PID <%d>", proceso_recibido->pid);

		// Si se obtuvo el marco
		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se obtuvo el marco <%d> de la pagina <%d> asociado a la instruccion IO_STDOUT_WRITE del proceso PID <%d>", proceso_recibido->marco, proceso_recibido->numero_pagina, proceso_recibido->pid);

			// Genero la direccion fisica con la MMU
			uint32_t direccion_fisica = mmu(args, proceso_recibido->registro_direccion, proceso_recibido->marco);

			// Inicio la peticion contra Kernel para que retransmita a la interfaz
			t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_STDOUT_WRITE);
			t_io_stdout_write *proceso_completo = malloc(sizeof(t_io_stdout_write));

			proceso_completo->pid = proceso_recibido->pid;
			proceso_completo->resultado = proceso_recibido->resultado;
			proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
			proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
			proceso_completo->marco = proceso_recibido->marco;
			proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
			proceso_completo->direccion_fisica = direccion_fisica;
			proceso_completo->desplazamiento = proceso_recibido->desplazamiento;
			proceso_completo->size_interfaz = proceso_recibido->size_interfaz;
			proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
			proceso_completo->registros = proceso_recibido->registros;

			serializar_t_io_stdout_write(&paquete, proceso_completo);
			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);

			log_debug(args->logger, "Se envio la solicitud de la instruccion IO_STDOUT_WRITE del proceso PID <%d> a Kernel", proceso_recibido->pid);

			free(proceso_completo->interfaz);
			free(proceso_completo);
		}
		else // Si no se obtuvo el marco
		{
			log_error(args->logger, "No se pudo obtener el marco de la pagina <%d> asociado a la instruccion IO_STDOUT_WRITE del proceso PID <%d>", proceso_recibido->numero_pagina, proceso_recibido->pid);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);
		}

		free(proceso_recibido->interfaz);
		free(proceso_recibido);

		break;
	}
	case MEMORIA_CPU_RESIZE:
	{
		t_memoria_cpu_resize *proceso_recibido = deserializar_t_memoria_cpu_resize(buffer);

		log_debug(args->logger, "Se recibio respuesta de Memoria sobre el pedido de RESIZE para el proceso <%d>", proceso_recibido->pid);

		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "No se pudo redimensionar el proceso <%d>", proceso_recibido->pid);
			log_debug(args->logger, "Motivo: <%s>", proceso_recibido->motivo);

			/* "En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación." */

			t_paquete *paquete = crear_paquete(CPU_KERNEL_RESIZE);

			t_cpu_kernel_resize *proceso_fallido = malloc(sizeof(t_cpu_kernel_resize));

			proceso_fallido->pid = proceso_recibido->pid;
			proceso_fallido->motivo = strdup(proceso_recibido->motivo);
			proceso_fallido->size_motivo = strlen(proceso_recibido->motivo) + 1;
			proceso_fallido->resultado = proceso_recibido->resultado;
			proceso_fallido->registros = args->registros;

			serializar_t_cpu_kernel_resize(&paquete, proceso_fallido);

			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);
			free(proceso_fallido);
		}
		else
		{
			log_debug(args->logger, "Se redimensiono el proceso con PID <%d> a <%d> bytes.", proceso_recibido->pid, proceso_recibido->bytes);
		}

		instruccion_solicitar(args);

		free(proceso_recibido->motivo);
		free(proceso_recibido);
		break;
	}
	case MEMORIA_CPU_PROXIMA_INSTRUCCION:
	{
		instruccion_ciclo(args, buffer);
		break;
	}
	case MEMORIA_CPU_TAM_PAGINA:
	{
		uint32_t *tamPag = deserializar_t_memoria_cpu_tam_pagina(buffer);
		args->tam_pagina = *tamPag;
		log_debug(args->logger, "Tamaño de pagina recibido de Memoria: %d", args->tam_pagina);
		free(tamPag);
		break;
	}
	default:
	{
		log_warning(args->logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&args->config_leida.socket_memoria);
		break;
	}
	}
}