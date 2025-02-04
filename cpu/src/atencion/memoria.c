#include <utils/cpu.h>

void switch_case_memoria(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_CPU_NUMERO_FRAME:
	{
		t_memoria_cpu_numero_marco *recibido = deserializar_t_memoria_cpu_numero_marco(buffer);

		if (recibido->resultado)
		{
			log_debug(args->logger, "Numero de frame recibido de Memoria: %d", recibido->numero_marco);

			log_info(args->logger, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", recibido->pid, recibido->numero_pagina, recibido->numero_marco);

			if (args->config_leida.cantidadEntradasTlb == 0)
			{
				args->marco = recibido->numero_marco;
			}
			else
			{
				agregar_en_tlb(recibido->pid, recibido->numero_pagina, recibido->numero_marco, args);
			}
			args->resultado = 1;
		}
		else
		{
			log_error(args->logger, "No se pudo obtener el numero de frame de la pagina <%d> solicitado por el proceso PID <%d>", recibido->numero_pagina, recibido->pid);

			args->resultado = 0;
			args->proceso.ejecutado = 0;

			instruccion_finalizar(args);
		}

		sem_post(&args->mmu_ejecucion);

		free(recibido);
		break;
	}
	case MEMORIA_CPU_IO_MOV_OUT:
	{
		t_mov_out *proceso_recibido = deserializar_t_mov_out(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de escritura asociada a la instruccion <MOV_OUT> para el proceso PID <%d>", proceso_recibido->pid);

		// Si se escribio el dato
		if (proceso_recibido->resultado)
		{
			// Notifico que se escribio el dato
			if (proceso_recibido->tamanio_registro_datos == 4)
			{

				// Imprimo log: “PID: <PID> - Acción: <LEER / ESCRIBIR> - Dirección Física: <DIRECCION_FISICA> - Valor: <VALOR LEIDO / ESCRITO>”.
				log_info(args->logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", proceso_recibido->pid, proceso_recibido->direccion_fisica, proceso_recibido->registro_datos);

				log_debug(args->logger, "Se escribio el dato <%d> en el marco <%d> asociado a la instruccion <MOV_OUT> del proceso PID <%d>", proceso_recibido->dato_32, proceso_recibido->numero_marco, proceso_recibido->pid);
			}
			else
			{
				log_info(args->logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", proceso_recibido->pid, proceso_recibido->direccion_fisica, proceso_recibido->registro_datos);

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
	case MEMORIA_CPU_IO_MOV_IN:
	{
		t_mov_in *proceso_recibido = deserializar_t_mov_in(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de lectura asociada a la instruccion <MOV_IN> para el proceso PID <%d>", proceso_recibido->pid);

		// Si se obtuvo el dato
		if (proceso_recibido->resultado)
		{
			// Determino que tipo de registros debo operar
			char *registro_datos = strdup(args->instruccion[1]);

			// Guardo el dato en el registro correspondiente
			if (proceso_recibido->tamanio_registro_datos == 4)
			{
				log_info(args->logger, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>", proceso_recibido->pid, proceso_recibido->direccion_fisica, proceso_recibido->dato_32);

				log_debug(args->logger, "Se obtuvo el dato <%d> del marco <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->dato_32, proceso_recibido->numero_marco, proceso_recibido->pid);
				uint32_t *registro_datos_ptr = determinar_tipo_registro_uint32_t(registro_datos, &args->proceso);
				*registro_datos_ptr = proceso_recibido->dato_32;
				log_debug(args->logger, "Se guardo el dato <%d> en el registro <%s> del proceso PID <%d>", proceso_recibido->dato_32, args->instruccion[1], proceso_recibido->pid);
				imprimir_registros(args);
			}
			else
			{
				log_info(args->logger, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>", proceso_recibido->pid, proceso_recibido->direccion_fisica, proceso_recibido->dato_8);

				log_debug(args->logger, "Se obtuvo el dato <%d> del marco <%d> asociado a la instruccion <MOV_IN> del proceso PID <%d>", proceso_recibido->dato_8, proceso_recibido->numero_marco, proceso_recibido->pid);
				uint8_t *registro_datos_ptr = determinar_tipo_registro_uint8_t(registro_datos, &args->proceso);
				*registro_datos_ptr = proceso_recibido->dato_8;
				log_debug(args->logger, "Se guardo el dato <%d> en el registro <%s> del proceso PID <%d>", proceso_recibido->dato_8, args->instruccion[1], proceso_recibido->pid);
				imprimir_registros(args);
			}

			free(proceso_recibido);
			free(registro_datos);

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
	case MEMORIA_CPU_IO_STDIN_READ:
	{
		t_io_stdin_read *proceso_recibido = deserializar_t_io_stdin_read(buffer);

		log_debug(args->logger, "Se recibio una respuesta de Memoria acerca de la solicitud de asignacion de marcos asociada a la instruccion IO_STDIN_READ para el proceso PID <%d>", proceso_recibido->pid);

		// Si se asigno el marco correctamente
		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se asignaron los marcos asociados a la instruccion IO_STDIN_READ del proceso PID <%d> a partir de la dirección logica <%d>", proceso_recibido->pid, proceso_recibido->registro_direccion);

			// FIXME: Esto está mal

			// // Genero la direccion fisica con la MMU
			// if (args->config_leida.cantidadEntradasTlb > 0)
			// {
			// 	agregar_en_tlb(proceso_recibido->pid, proceso_recibido->numero_pagina, proceso_recibido->marco_inicial, args);
			// }

			mmu_iniciar(args, IO_STDIN_READ, proceso_recibido->registro_direccion, (void *)proceso_recibido);
		}
		else // Si no se pudo asignar el marco
		{
			log_error(args->logger, "No se pudo asignar el marco de la pagina <%d> asociado a la instruccion IO_STDIN_READ del proceso PID <%d>", proceso_recibido->pid, proceso_recibido->numero_pagina);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);

			free(proceso_recibido->interfaz);
			free(proceso_recibido);
		}
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
			log_debug(args->logger, "Se recibio el marco <%d> para SI y <%d> para DI asociado a la instruccion <COPY_STRING> del proceso PID <%d>", proceso_recibido->marco_si, proceso_recibido->marco_di, proceso_recibido->pid);

			t_paquete *paquete = crear_paquete(CPU_MEMORIA_COPY_STRING_2);

			// Genero la direccion fisica con la MMU
			proceso_completo->direccion_fisica_si = mmu(args, proceso_recibido->direccion_si, proceso_recibido->marco_si);
			proceso_completo->direccion_fisica_di = mmu(args, proceso_recibido->direccion_di, proceso_recibido->marco_di);

			log_debug(args->logger, "Se genero la direccion fisica <%d> para SI y <%d> para DI asociada a la instruccion <COPY_STRING> del proceso PID <%d>", proceso_completo->direccion_fisica_si, proceso_completo->direccion_fisica_di, proceso_recibido->pid);

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
			log_info(args->logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>", proceso_recibido->pid, proceso_recibido->direccion_fisica_di, proceso_recibido->frase);

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
	case MEMORIA_CPU_IO_FS_WRITE:
	{
		t_memoria_cpu_fs_write *proceso_recibido = deserializar_t_memoria_cpu_fs_write(buffer);

		if (proceso_recibido->resultado)
		{
			log_debug(args->logger, "Se recibió una respuesta de Memoria acerca de la solicitud del dato asociada a la instrucción <FS_WRITE> para el proceso PID <%d>", proceso_recibido->pid);

			t_cpu_kernel_fs_write *proceso_enviar = malloc(sizeof(t_cpu_kernel_fs_write));

			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->resultado = proceso_recibido->resultado;
			proceso_enviar->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
			proceso_enviar->size_nombre_archivo = proceso_recibido->size_nombre_archivo;
			proceso_enviar->escribir = strdup(proceso_recibido->dato);
			proceso_enviar->size_escribir = proceso_recibido->size_dato;
			proceso_enviar->interfaz = strdup(proceso_recibido->interfaz);
			proceso_enviar->size_interfaz = proceso_recibido->size_interfaz;
			proceso_enviar->registros = proceso_recibido->registros;
			proceso_enviar->puntero_archivo = proceso_recibido->puntero_archivo;
			t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_FS_WRITE);
			serializar_t_cpu_kernel_fs_write(&paquete, proceso_enviar);
			enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

			log_debug(args->logger, "Se envió la solicitud de la instrucción <IO_FS_WRITE> del proceso PID <%d> a Kernel", proceso_recibido->pid);

			eliminar_paquete(paquete);
			free(proceso_enviar->nombre_archivo);
			free(proceso_enviar->escribir);
			free(proceso_enviar->interfaz);
			free(proceso_enviar);
			break;
		}

		log_error(args->logger, "No se pudo obtener el dato asociado a la instrucción <IO_FS_WRITE> del proceso PID <%d>", proceso_recibido->pid);
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