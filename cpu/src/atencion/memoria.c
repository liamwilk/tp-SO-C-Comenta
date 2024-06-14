#include <utils/cpu.h>

void switch_case_memoria(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_CPU_IO_MOV_OUT_2:
	{
		t_mov_out *proceso_recibido = deserializar_t_mov_out(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de escritura asociada a la instruccion <MOV_OUT> para el proceso PID <%d>", proceso_recibido->pid);

		// Si se escribio el dato
		if (proceso_recibido->resultado)
		{
			// Notifico que se escribio el dato
			if (proceso_recibido->tamanio_registro_datos == 4)
			{
				log_debug(args->logger, "Se escribio el dato <%d> en el marco <%d> asociado a la instruccion <MOV_OUT> del proceso PID <%d>", proceso_recibido->dato_32, proceso_recibido->numero_marco, proceso_recibido->pid);
			}
			else
			{
				log_debug(args->logger, "Se escribio el dato <%d> en el marco <%d> asociado a la instruccion <MOV_OUT> del proceso PID <%d>", proceso_recibido->dato_8, proceso_recibido->numero_marco, proceso_recibido->pid);
			}

			free(proceso_recibido);

			// Reanudo el ciclo de ejecucion por finalizacion de la instruccion atomica MOV_OUT
			if (args->flag_interrupt)
			{
				instruccion_interrupt(args);
				break;
			}

			instruccion_solicitar(args);
		}
		else // Si no se escribio el dato
		{
			log_error(args->logger, "No se pudo escribir el dato en el marco <%d> asociado a la instruccion <MOV_OUT> del proceso PID <%d>", proceso_recibido->numero_marco, proceso_recibido->pid);

			free(proceso_recibido);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);
		}
		break;
	}
	case MEMORIA_CPU_IO_MOV_OUT:
	{
		t_mov_out *proceso_recibido = deserializar_t_mov_out(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de marco asociada a la instruccion <MOV_IN> para el proceso PID <%d>", proceso_recibido->pid);

		// Si se obtuvo el marco
		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se obtuvo el marco <%d> de la pagina <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->numero_marco, proceso_recibido->numero_pagina, proceso_recibido->pid);

			// Genero la direccion fisica con la MMU
			uint32_t direccion_fisica = mmu(args, proceso_recibido->registro_direccion, proceso_recibido->numero_marco);

			// Inicio la peticion contra Memoria para que escriba
			t_paquete *paquete = crear_paquete(CPU_MEMORIA_MOV_OUT_2);
			t_mov_out *proceso_completo = malloc(sizeof(t_mov_out));

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

			serializar_t_mov_out(&paquete, proceso_completo);
			enviar_paquete(paquete, args->config_leida.socket_memoria);
			eliminar_paquete(paquete);

			log_debug(args->logger, "Se envio la solicitud de la instruccion <MOV_IN> del proceso PID <%d> a Kernel", proceso_recibido->pid);

			free(proceso_completo);
		}
		else // Si no se obtuvo el marco
		{
			log_error(args->logger, "No se pudo obtener el marco de la pagina <%d> asociado a la instruccion <MOV_OUT> del proceso PID <%d>", proceso_recibido->numero_pagina, proceso_recibido->pid);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);
		}

		free(proceso_recibido);

		break;

		break;
	}
	case MEMORIA_CPU_IO_MOV_IN_2:
	{
		t_mov_in *proceso_recibido = deserializar_t_mov_in(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de lectura asociada a la instruccion <MOV_IN> para el proceso PID <%d>", proceso_recibido->pid);

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
			proceso_completo->registros = args->proceso.registros;

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
	case MEMORIA_CPU_IO_STDIN_READ:
	{
		t_io_stdin_read *proceso_recibido = deserializar_t_io_stdin_read(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de marco asociada a la instruccion IO_STDIN_READ para el proceso PID <%d>", proceso_recibido->pid);

		// Si se obtuvo el marco correctamente
		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se obtuvo el marco inicial <%d> de la pagina <%d> asociado a la instruccion IO_STDIN_READ del proceso PID <%d>", proceso_recibido->marco_inicial, proceso_recibido->numero_pagina, proceso_recibido->pid);

			// Genero la direccion fisica con la MMU
			uint32_t direccion_fisica = mmu(args, proceso_recibido->registro_direccion, proceso_recibido->marco_inicial);

			// Inicio la peticion contra Kernel para que retransmita a la interfaz
			t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_STDIN_READ);
			t_io_stdin_read *proceso_completo = malloc(sizeof(t_io_stdin_read));

			proceso_completo->pid = proceso_recibido->pid;
			proceso_completo->resultado = proceso_recibido->resultado;
			proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
			proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
			proceso_completo->marco_inicial = proceso_recibido->marco_inicial;
			proceso_completo->marco_final = proceso_recibido->marco_final;
			proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
			proceso_completo->direccion_fisica = direccion_fisica;
			proceso_completo->desplazamiento = proceso_recibido->desplazamiento;
			proceso_completo->size_interfaz = proceso_recibido->size_interfaz;
			proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
			proceso_completo->registros = proceso_recibido->registros;

			serializar_t_io_stdin_read(&paquete, proceso_completo);
			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);

			log_debug(args->logger, "Se envio la solicitud de la instruccion IO_STDIN_READ del proceso PID <%d> a Kernel", proceso_recibido->pid);

			free(proceso_completo->interfaz);
			free(proceso_completo);
		}
		else // Si no se obtuvo el marco
		{
			log_error(args->logger, "No se pudo obtener el marco de la pagina <%d> asociado a la instruccion IO_STDIN_READ del proceso PID <%d>", proceso_recibido->pid, proceso_recibido->numero_pagina);

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

		t_paquete *paquete = crear_paquete(CPU_KERNEL_RESIZE);
		t_cpu_kernel_resize *proceso_enviar = malloc(sizeof(t_cpu_kernel_resize));

		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se redimensiono el proceso con PID <%d> a <%d> bytes.", proceso_recibido->pid, proceso_recibido->bytes);

			// Notifico que se redimensiono el proceso

			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->motivo = strdup(proceso_recibido->motivo);
			proceso_enviar->size_motivo = strlen(proceso_recibido->motivo) + 1;
			proceso_enviar->resultado = proceso_recibido->resultado;
			proceso_enviar->registros = args->registros;

			serializar_t_cpu_kernel_resize(&paquete, proceso_enviar);

			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);
			free(proceso_recibido->motivo);
			free(proceso_recibido);
			free(proceso_enviar->motivo);
			free(proceso_enviar);

			// Reanudo el ciclo de ejecucion por finalizacion de la instruccion atomica RESIZE

			if (args->flag_interrupt)
			{
				instruccion_interrupt(args);
				break;
			}

			instruccion_solicitar(args);
		}
		else
		{
			log_debug(args->logger, "No se pudo redimensionar el proceso <%d>", proceso_recibido->pid);
			log_debug(args->logger, "Motivo: <%s>", proceso_recibido->motivo);

			/* "En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación." */

			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->motivo = strdup(proceso_recibido->motivo);
			proceso_enviar->size_motivo = strlen(proceso_recibido->motivo) + 1;
			proceso_enviar->resultado = proceso_recibido->resultado;
			proceso_enviar->registros = args->registros;

			serializar_t_cpu_kernel_resize(&paquete, proceso_enviar);

			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);
			free(proceso_recibido->motivo);
			free(proceso_recibido);
			free(proceso_enviar->motivo);
			free(proceso_enviar);
		}
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
	case MEMORIA_CPU_COPY_STRING:
	{
		t_copy_string *proceso_recibido = deserializar_t_copy_string(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de marcos para SI y DI asociada a la instruccion <COPY_STRING> para el proceso PID <%d>", proceso_recibido->pid);

		t_copy_string *proceso_completo = malloc(sizeof(t_copy_string));

		// Si se copio el string
		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se recibio el marco <%d> para SI y <%d> para DI asociado a la instruccion <COPY_STRING> del proceso PID <%d>", proceso_recibido->marco_si,proceso_recibido->marco_di, proceso_recibido->pid);

			t_paquete *paquete = crear_paquete(CPU_MEMORIA_COPY_STRING_2);

			// Genero la direccion fisica con la MMU
			proceso_completo->direccion_fisica_si = mmu(args, proceso_recibido->direccion_si, proceso_recibido->marco_si);
			proceso_completo->direccion_fisica_di = mmu(args, proceso_recibido->direccion_di, proceso_recibido->marco_di);

			log_warning(args->logger, "Se genero la direccion fisica <%d> para SI y <%d> para DI asociada a la instruccion <COPY_STRING> del proceso PID <%d>", proceso_completo->direccion_fisica_si, proceso_completo->direccion_fisica_di, proceso_recibido->pid);

			proceso_completo->direccion_si = proceso_recibido->direccion_si;
			proceso_completo->direccion_di = proceso_recibido->direccion_di;
			proceso_completo->pid = proceso_recibido->pid;
			proceso_completo->resultado = 1;
			proceso_completo->marco_si = proceso_recibido->marco_si;
			proceso_completo->marco_di = proceso_recibido->marco_di;
			proceso_completo->frase = strdup(proceso_recibido->frase);
			proceso_completo->size_frase = proceso_recibido->size_frase;
			proceso_completo->cant_bytes = proceso_recibido->cant_bytes;

			serializar_t_copy_string(&paquete, proceso_completo);
			enviar_paquete(paquete, args->config_leida.socket_memoria);
			eliminar_paquete(paquete);
		}
		else
		{
			log_error(args->logger, "No se encontró uno de los marcos asociados a la instruccion <COPY_STRING> del proceso PID <%d>", proceso_recibido->pid);

			t_paquete *paquete = crear_paquete(CPU_KERNEL_COPY_STRING);

			proceso_completo->pid = proceso_recibido->pid;
			proceso_completo->direccion_fisica_di = proceso_recibido->direccion_fisica_di;
			proceso_completo->direccion_fisica_si = proceso_recibido->direccion_fisica_si;
			proceso_completo->direccion_si = proceso_recibido->direccion_si;
			proceso_completo->direccion_di = proceso_recibido->direccion_di;
			proceso_completo->resultado = 0;
			proceso_completo->marco_si = proceso_recibido->marco_si;
			proceso_completo->marco_di = proceso_recibido->marco_di;
			proceso_completo->frase = strdup(proceso_recibido->frase);
			proceso_completo->size_frase = proceso_recibido->size_frase;
			proceso_completo->cant_bytes = proceso_recibido->cant_bytes;

			serializar_t_copy_string(&paquete, proceso_completo);
			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);
		}

		free(proceso_completo->frase);
		free(proceso_recibido->frase);
		free(proceso_completo);
		free(proceso_recibido);

		break;
	}
	case MEMORIA_CPU_COPY_STRING_2:
	{
		t_copy_string *proceso_recibido = deserializar_t_copy_string(buffer);

		t_paquete *paquete = crear_paquete(CPU_KERNEL_COPY_STRING);

		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se escribio la frase <%s> en la direccion logica apuntada por el registro DI asociado a la instruccion <COPY_STRING> del proceso PID <%d>", proceso_recibido->frase, proceso_recibido->pid);
			
			serializar_t_copy_string(&paquete, proceso_recibido);
			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);
			free(proceso_recibido->frase);
			free(proceso_recibido);

			if (args->flag_interrupt)
			{
				instruccion_interrupt(args);
				break;
			}

			instruccion_solicitar(args);
			break;
		}
		else
		{
			log_error(args->logger, "No se pudo escribir la frase referenciada por SI en el registro DI asociados a la instruccion <COPY_STRING> del proceso PID <%d>", proceso_recibido->pid);
			log_error(args->logger, "Motivo: %s", proceso_recibido->frase);
			serializar_t_copy_string(&paquete, proceso_recibido);
			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
			eliminar_paquete(paquete);

			free(proceso_recibido->frase);
			free(proceso_recibido);
			break;
		}
	}
	default:
	{
		log_warning(args->logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&args->config_leida.socket_memoria);
		break;
	}
	}
}