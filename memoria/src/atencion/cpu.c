#include <utils/memoria.h>

void switch_case_cpu(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case CPU_MEMORIA_MOV_OUT:
	{
		t_mov_out *paquete_recibido = deserializar_t_mov_out(buffer);

		log_debug(argumentos->logger, "Se recibio la solicitud de escritura de <%d> bytes desde la direccion fisica <%d> asociado al proceso PID <%d>", paquete_recibido->tamanio_registro_datos, paquete_recibido->direccion_fisica, paquete_recibido->pid);

		t_paquete *paquete = crear_paquete(MEMORIA_CPU_IO_MOV_OUT);
		t_mov_out *paquete_enviar = malloc(sizeof(t_mov_out));
		t_proceso *proceso = buscar_proceso(argumentos, paquete_recibido->pid);

		// Escribo el dato en el espacio de usuario
		if (paquete_recibido->tamanio_registro_datos > 0 && paquete_recibido->tamanio_registro_datos <= 4)
		{	
			if (paquete_recibido->tamanio_registro_datos == 4) // Debo escribir 4 bytes desde DF
			{
				espacio_usuario_escribir_uint32_t(argumentos, proceso, paquete_recibido->direccion_fisica, paquete_recibido->registro_datos);
				log_debug(argumentos->logger, "Se escribio en espacio de usuario desde a la direccion fisica <%d> el siguiente numero de 4 bytes: %d", paquete_recibido->direccion_fisica, paquete_recibido->registro_datos);
			}
			else if (paquete_recibido->tamanio_registro_datos == 1) // Debo escribir 1 byte desde DF
			{
				espacio_usuario_escribir_uint8_t(argumentos, proceso, paquete_recibido->direccion_fisica, paquete_recibido->registro_datos);
				log_debug(argumentos->logger, "Se escribio en espacio de usuario desde a la direccion fisica <%d> el siguiente numero de 1 byte: %d", paquete_recibido->direccion_fisica, paquete_recibido->registro_datos);
			}
		}
		else
		{
			log_error(argumentos->logger, "No se puede escribir un registro de <%d> bytes porque la instruccion solicitada no lo contempla.", paquete_recibido->tamanio_registro_datos);

			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;
			paquete_enviar->tamanio_registro_datos = paquete_recibido->tamanio_registro_datos;
			paquete_enviar->numero_marco = paquete_recibido->numero_marco;
			paquete_enviar->numero_pagina = paquete_recibido->numero_pagina;
			paquete_enviar->registro_datos = paquete_recibido->registro_datos;
			paquete_enviar->registro_direccion = paquete_recibido->registro_direccion;
			paquete_enviar->resultado = 0;
			paquete_enviar->dato_32 = 0;
			paquete_enviar->dato_8 = 0;

			serializar_t_mov_out(&paquete, paquete_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(paquete_enviar);
			free(paquete_recibido);
			break;
		}

		paquete_enviar->dato_8 = paquete_recibido->dato_8;
		paquete_enviar->dato_32 = paquete_recibido->dato_32;
		paquete_enviar->pid = paquete_recibido->pid;
		paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;
		paquete_enviar->tamanio_registro_datos = paquete_recibido->tamanio_registro_datos;
		paquete_enviar->numero_marco = paquete_recibido->numero_marco;
		paquete_enviar->numero_pagina = paquete_recibido->numero_pagina;
		paquete_enviar->registro_datos = paquete_recibido->registro_datos;
		paquete_enviar->registro_direccion = paquete_recibido->registro_direccion;
		paquete_enviar->resultado = 1;

		serializar_t_mov_out(&paquete, paquete_enviar);
		enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
		eliminar_paquete(paquete);

		free(paquete_enviar);
		free(paquete_recibido);

		break;
	}
	case CPU_MEMORIA_MOV_IN:
	{
		t_mov_in *paquete_recibido = deserializar_t_mov_in(buffer);

		log_debug(argumentos->logger, "Se recibio la solicitud de lectura de <%d> bytes en la direccion fisica <%d>", paquete_recibido->tamanio_registro_datos, paquete_recibido->direccion_fisica);

		t_paquete *paquete = crear_paquete(MEMORIA_CPU_IO_MOV_IN);
		t_mov_in *paquete_enviar = malloc(sizeof(t_mov_in));
		t_proceso *proceso = buscar_proceso(argumentos, paquete_recibido->pid);

		// Cargo el dato a enviar
		if (paquete_recibido->tamanio_registro_datos > 0 && paquete_recibido->tamanio_registro_datos <= 4)
		{
			if (paquete_recibido->tamanio_registro_datos == 4)
			{
				paquete_enviar->dato_8 = 0;
				paquete_enviar->dato_32 = espacio_usuario_leer_uint32(argumentos, proceso, paquete_recibido->direccion_fisica);
				log_debug(argumentos->logger, "Se leyo del espacio de usuario asociado a la direccion fisica <%d> el siguiente numero: %d", paquete_recibido->direccion_fisica, paquete_enviar->dato_32);
			}
			else if (paquete_recibido->tamanio_registro_datos == 1)
			{
				paquete_enviar->dato_32 = 0;
				paquete_enviar->dato_8 = espacio_usuario_leer_uint8(argumentos, proceso, paquete_recibido->direccion_fisica);
				log_debug(argumentos->logger, "Se leyo del espacio de usuario asociado a la direccion fisica <%d> el siguiente numero: %d", paquete_recibido->direccion_fisica, paquete_enviar->dato_8);
			}
		}
		else
		{
			log_error(argumentos->logger, "No se puede leer un registro de <%d> bytes porque la instruccion solicitada no lo contempla.", paquete_recibido->tamanio_registro_datos);

			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;
			paquete_enviar->tamanio_registro_datos = paquete_recibido->tamanio_registro_datos;
			paquete_enviar->numero_marco = paquete_recibido->numero_marco;
			paquete_enviar->numero_pagina = paquete_recibido->numero_pagina;
			paquete_enviar->registro_datos = paquete_recibido->registro_datos;
			paquete_enviar->registro_direccion = paquete_recibido->registro_direccion;
			paquete_enviar->resultado = 0;
			paquete_enviar->dato_32 = 0;
			paquete_enviar->dato_8 = 0;

			serializar_t_mov_in(&paquete, paquete_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(paquete_enviar);
			free(paquete_recibido);
			break;
		}

		paquete_enviar->pid = paquete_recibido->pid;
		paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;
		paquete_enviar->tamanio_registro_datos = paquete_recibido->tamanio_registro_datos;
		paquete_enviar->numero_marco = paquete_recibido->numero_marco;
		paquete_enviar->numero_pagina = paquete_recibido->numero_pagina;
		paquete_enviar->registro_datos = paquete_recibido->registro_datos;
		paquete_enviar->registro_direccion = paquete_recibido->registro_direccion;
		paquete_enviar->resultado = 1;

		serializar_t_mov_in(&paquete, paquete_enviar);
		enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
		eliminar_paquete(paquete);

		free(paquete_enviar);
		free(paquete_recibido);
		break;
	}
	case CPU_MEMORIA_IO_STDIN_READ:
	{
		t_io_stdin_read *proceso_recibido = deserializar_t_io_stdin_read(buffer);

		log_debug(argumentos->logger, "Se recibio una peticion de asignacion de marco de <%d> bytes para el proceso PID <%d>", proceso_recibido->registro_tamanio, proceso_recibido->pid);

		// Busco el proceso en la lista de procesos globales
		t_proceso *proceso = buscar_proceso(argumentos, proceso_recibido->pid);

		// Si el proceso no existe, envio una señal a CPU
		if (proceso == NULL)
		{
			log_error(argumentos->logger, "No se encontro el proceso con PID <%d> en Memoria", proceso_recibido->pid);

			t_paquete *paquete = crear_paquete(MEMORIA_CPU_IO_STDIN_READ);
			t_io_stdin_read *proceso_enviar = malloc(sizeof(t_io_stdin_read));

			// Le devuelvo lo que recibi, pero con resultado 0 indicando que fallo
			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->resultado = 1;
			proceso_enviar->registro_direccion = proceso_recibido->registro_direccion;
			proceso_enviar->registro_tamanio = proceso_recibido->registro_tamanio;
			proceso_enviar->marco_inicial = proceso_recibido->marco_inicial;
			proceso_enviar->marco_final = proceso_recibido->marco_final;
			proceso_enviar->numero_pagina = proceso_recibido->numero_pagina;
			proceso_enviar->direccion_fisica = proceso_recibido->direccion_fisica;
			proceso_enviar->desplazamiento = proceso_recibido->desplazamiento;
			proceso_enviar->interfaz = strdup(proceso_recibido->interfaz);
			proceso_enviar->size_interfaz = proceso_recibido->size_interfaz;
			proceso_enviar->registros = proceso_recibido->registros;

			serializar_t_io_stdin_read(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(proceso_enviar->interfaz);
			free(proceso_enviar);

			free(proceso_recibido->interfaz);
			free(proceso_recibido);
			break;
		}

		// Verifico si la página solicitada existe
		t_pagina *pagina = list_get(proceso->tabla_paginas, proceso_recibido->numero_pagina);

		if (pagina == NULL)
		{
			log_error(argumentos->logger, "No se encontro la pagina %d para el proceso con PID <%d> asociado a IO_STDIN_READ", proceso_recibido->numero_pagina, proceso_recibido->pid);
			t_paquete *paquete = crear_paquete(MEMORIA_CPU_IO_STDIN_READ);
			t_io_stdin_read *proceso_enviar = malloc(sizeof(t_io_stdin_read));

			// Le devuelvo lo que recibi, pero con resultado 0 indicando que fallo
			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->resultado = 0;
			proceso_enviar->registro_direccion = proceso_recibido->registro_direccion;
			proceso_enviar->registro_tamanio = proceso_recibido->registro_tamanio;
			proceso_enviar->marco_inicial = proceso_recibido->marco_inicial;
			proceso_enviar->marco_final = proceso_recibido->marco_final;
			proceso_enviar->numero_pagina = proceso_recibido->numero_pagina;
			proceso_enviar->direccion_fisica = proceso_recibido->direccion_fisica;
			proceso_enviar->desplazamiento = proceso_recibido->desplazamiento;
			proceso_enviar->interfaz = strdup(proceso_recibido->interfaz);
			proceso_enviar->size_interfaz = proceso_recibido->size_interfaz;
			proceso_enviar->registros = proceso_recibido->registros;

			serializar_t_io_stdin_read(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(proceso_enviar->interfaz);
			free(proceso_enviar);

			free(proceso_recibido->interfaz);
			free(proceso_recibido);
			break;
		}

		// Si no hay espacio en la tabla de paginas para escribir el tamaño solicitado, envio un mensaje a CPU
		if (proceso_recibido->registro_tamanio > espacio_usuario_bytes_disponibles(argumentos))
		{
			log_error(argumentos->logger, "No hay espacio suficiente para poder escribir %d bytes en el proceso con PID <%d> asociado a IO_STDIN_READ", proceso_recibido->registro_tamanio, proceso_recibido->pid);

			// Notifico a CPU que el tamaño solicitado a leer no coincide con lo que fue escrito por el proceso
			t_paquete *paquete = crear_paquete(MEMORIA_CPU_IO_STDIN_READ);
			t_io_stdin_read *proceso_enviar = malloc(sizeof(t_io_stdin_read));

			// Le devuelvo lo que recibi, pero con resultado 0 indicando que fallo
			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->resultado = 0;
			proceso_enviar->registro_direccion = proceso_recibido->registro_direccion;
			proceso_enviar->registro_tamanio = proceso_recibido->registro_tamanio;
			proceso_enviar->marco_inicial = proceso_recibido->marco_inicial;
			proceso_enviar->marco_final = proceso_recibido->marco_final;
			proceso_enviar->numero_pagina = proceso_recibido->numero_pagina;
			proceso_enviar->direccion_fisica = proceso_recibido->direccion_fisica;
			proceso_enviar->desplazamiento = proceso_recibido->desplazamiento;
			proceso_enviar->interfaz = strdup(proceso_recibido->interfaz);
			proceso_enviar->size_interfaz = proceso_recibido->size_interfaz;
			proceso_enviar->registros = proceso_recibido->registros;

			serializar_t_io_stdin_read(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(proceso_enviar->interfaz);
			free(proceso_enviar);

			free(proceso_recibido->interfaz);
			free(proceso_recibido);
			break;
		}

		int j = 0;
		proceso_recibido->marco_inicial = proceso_recibido->registro_direccion / argumentos->memoria.tamPagina;
		proceso_recibido->marco_final = (proceso_recibido->registro_direccion + proceso_recibido->registro_tamanio - 1) / argumentos->memoria.tamPagina;

		log_debug(argumentos->logger, "Se asignaron los marcos <%d> al <%d> para el proceso con PID <%d> asociado a IO_STDIN_READ", proceso_recibido->marco_inicial, proceso_recibido->marco_final, proceso_recibido->pid);

		for (int i = proceso_recibido->marco_inicial; i <= proceso_recibido->marco_final; i++)
		{
			// Le asigno el marco y pagina al proceso
			tabla_paginas_asignar_pagina(argumentos, proceso, proceso_recibido->numero_pagina + j, i);
			j++;
		}

		// Notifico a CPU que se asigno el marco
		t_paquete *paquete = crear_paquete(MEMORIA_CPU_IO_STDIN_READ);
		t_io_stdin_read *proceso_enviar = malloc(sizeof(t_io_stdin_read));

		proceso_enviar->pid = proceso_recibido->pid;
		proceso_enviar->resultado = 1;
		proceso_enviar->registro_direccion = proceso_recibido->registro_direccion;
		proceso_enviar->registro_tamanio = proceso_recibido->registro_tamanio;
		proceso_enviar->marco_inicial = proceso_recibido->marco_inicial;
		proceso_enviar->marco_final = proceso_recibido->marco_final;
		proceso_enviar->numero_pagina = proceso_recibido->numero_pagina;
		proceso_enviar->direccion_fisica = proceso_recibido->direccion_fisica;
		proceso_enviar->desplazamiento = proceso_recibido->desplazamiento;
		proceso_enviar->interfaz = strdup(proceso_recibido->interfaz);
		proceso_enviar->size_interfaz = proceso_recibido->size_interfaz;
		proceso_enviar->registros = proceso_recibido->registros;

		serializar_t_io_stdin_read(&paquete, proceso_enviar);
		enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
		eliminar_paquete(paquete);

		free(proceso_enviar->interfaz);
		free(proceso_enviar);

		free(proceso_recibido->interfaz);
		free(proceso_recibido);
		break;
	}
	case CPU_MEMORIA_RESIZE:
	{
		t_cpu_memoria_resize *proceso_recibido = deserializar_t_cpu_memoria_resize(buffer);

		log_debug(argumentos->logger, "Se recibio un pedido de RESIZE de %d bytes para el proceso con PID <%d>", proceso_recibido->bytes, proceso_recibido->pid);

		t_proceso *proceso = buscar_proceso(argumentos, proceso_recibido->pid);

		// Si el proceso no existe, envio un mensaje a CPU
		if (proceso == NULL)
		{
			log_error(argumentos->logger, "No se encontro el proceso con PID <%d> para efectuar el resize.", proceso_recibido->pid);

			// Aviso a CPU que no se encontro el proceso
			t_paquete *paquete = crear_paquete(MEMORIA_CPU_RESIZE);
			t_memoria_cpu_resize *proceso_enviar = malloc(sizeof(t_memoria_cpu_resize));
			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->bytes = proceso_recibido->bytes;
			proceso_enviar->resultado = 0;
			proceso_enviar->motivo = strdup("No se encontro el proceso con PID solicitado");
			proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

			serializar_t_memoria_cpu_resize(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);

			free(proceso_enviar->motivo);
			free(proceso_enviar);
			eliminar_paquete(paquete);
			break;
		}

		pthread_mutex_lock(&proceso->mutex_tabla_paginas);
		int bytes_disponibles_memoria = espacio_usuario_bytes_disponibles(argumentos);
		pthread_mutex_unlock(&proceso->mutex_tabla_paginas);

		int frames_disponibles_memoria = bytes_disponibles_memoria / argumentos->memoria.tamPagina;

		pthread_mutex_lock(&proceso->mutex_tabla_paginas);
		int bytes_ocupados_proceso = tabla_paginas_bytes_ocupados(argumentos, proceso);
		pthread_mutex_unlock(&proceso->mutex_tabla_paginas);

		int frames_solicitados = proceso_recibido->bytes / argumentos->memoria.tamPagina;

		// Si el proceso existe, verifico si el resize es positivo o negativo
		if (proceso_recibido->bytes > bytes_ocupados_proceso)
		{
			log_debug(argumentos->logger, "Se solicito un resize positivo de %d bytes", proceso_recibido->bytes);

			// Si la cantidad total de bytes pedidos supera a la cantidad maxima de bytes disponibles en memoria, no se puede realizar el resize
			if (proceso_recibido->bytes > bytes_disponibles_memoria)
			{
				log_error(argumentos->logger, "Out of Memory. Se solicito hacer un resize de %d bytes (%d frames) al proceso PID <%d> y el espacio disponible en Memoria es de %d bytes (%d frames)", proceso_recibido->bytes, frames_solicitados, proceso_recibido->pid, bytes_disponibles_memoria, frames_disponibles_memoria);

				t_paquete *paquete = crear_paquete(MEMORIA_CPU_RESIZE);
				t_memoria_cpu_resize *proceso_enviar = malloc(sizeof(t_memoria_cpu_resize));
				proceso_enviar->pid = proceso_recibido->pid;
				proceso_enviar->bytes = proceso_recibido->bytes;
				proceso_enviar->resultado = 0;
				proceso_enviar->motivo = strdup("Out of Memory");
				proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

				serializar_t_memoria_cpu_resize(&paquete, proceso_enviar);
				enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);

				free(proceso_enviar->motivo);
				free(proceso_enviar);
				eliminar_paquete(paquete);
				break;
			}

			log_info(argumentos->logger, "Ampliación de Proceso: PID: %d - Tamaño Actual: <%d bytes> - Tamaño a Ampliar: <%d bytes>", proceso_recibido->pid, bytes_ocupados_proceso, proceso_recibido->bytes);

			tabla_paginas_resize(argumentos, proceso, proceso_recibido->bytes);

			// Retorno el resultado del resize a CPU

			t_paquete *paquete = crear_paquete(MEMORIA_CPU_RESIZE);
			t_memoria_cpu_resize *proceso_enviar = malloc(sizeof(t_memoria_cpu_resize));

			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->bytes = proceso_recibido->bytes;
			proceso_enviar->resultado = 1;
			proceso_enviar->motivo = strdup("OK");
			proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

			serializar_t_memoria_cpu_resize(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);
			free(proceso_enviar);
		}
		else
		{
			log_debug(argumentos->logger, "Se solicito un resize negativo de %d bytes", proceso_recibido->bytes);

			/*
			Reducción de un proceso
			Se reducirá el mismo desde el final, liberando, en caso de ser necesario, las páginas que ya no sean utilizadas (desde la última hacia la primera).
			*/

			log_info(argumentos->logger, "Reducción de Proceso: PID: <%d> - Tamaño Actual: <%d bytes> - Tamaño a Reducir: <%d bytes>", proceso_recibido->pid, bytes_ocupados_proceso, proceso_recibido->bytes);

			tabla_paginas_resize(argumentos, proceso, proceso_recibido->bytes);

			// Retorno el resultado del resize a CPU

			t_paquete *paquete = crear_paquete(MEMORIA_CPU_RESIZE);
			t_memoria_cpu_resize *proceso_enviar = malloc(sizeof(t_memoria_cpu_resize));

			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->bytes = proceso_recibido->bytes;
			proceso_enviar->resultado = 1;
			proceso_enviar->motivo = strdup("OK");
			proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

			serializar_t_memoria_cpu_resize(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);
			free(proceso_enviar);
		}

		break;
	}
	case CPU_MEMORIA_PROXIMA_INSTRUCCION:
	{
		t_cpu_memoria_instruccion *instruccion_recibida = deserializar_t_cpu_memoria_instruccion(buffer);

		log_debug(argumentos->logger, "Instruccion recibida de CPU:");
		log_debug(argumentos->logger, "PID: %d", instruccion_recibida->pid);
		log_debug(argumentos->logger, "Program counter: %d", instruccion_recibida->program_counter);

		log_debug(argumentos->logger, "Se comienza a buscar el proceso solicitado por CPU en la lista de procesos globales.");

		t_proceso *proceso_encontrado = buscar_proceso(argumentos, instruccion_recibida->pid);

		if (proceso_encontrado == NULL)
		{
			log_warning(argumentos->logger, "No se encontro el proceso con PID <%d>", instruccion_recibida->pid);
			free(instruccion_recibida);
			break;
		}

		log_debug(argumentos->logger, "Proceso encontrado:");
		log_debug(argumentos->logger, "PID: %d", proceso_encontrado->pid);
		log_debug(argumentos->logger, "Ultimo PC enviado: %d", proceso_encontrado->pc);
		log_debug(argumentos->logger, "Proximo PC a enviar: %d", instruccion_recibida->program_counter);

		// Caso borde, no debería pasar nunca
		if (list_is_empty(proceso_encontrado->instrucciones))
		{
			log_warning(argumentos->logger, "No se encontraron instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);
			free(instruccion_recibida);
			break;
		}

		log_debug(argumentos->logger, "Cantidad de instrucciones: %d", list_size(proceso_encontrado->instrucciones));

		int pc_solicitado = instruccion_recibida->program_counter;

		// Esto no debería pasar nunca, porque las instrucciones tienen EXIT al final.
		// En caso que el archivo de instrucciones no tenga EXIT al final, se envía un mensaje a CPU
		if (pc_solicitado == list_size(proceso_encontrado->instrucciones))
		{

			log_warning(argumentos->logger, "Se solicito una instruccion fuera de rango para el proceso con PID <%d>", proceso_encontrado->pid);

			log_debug(argumentos->logger, "Le aviso a CPU que no hay mas instrucciones para el proceso con PID <%d>", proceso_encontrado->pid);

			t_paquete *paquete_instruccion = crear_paquete(MEMORIA_CPU_PROXIMA_INSTRUCCION);
			t_memoria_cpu_instruccion *instruccion_nula = malloc(sizeof(t_memoria_cpu_instruccion));
			instruccion_nula->pid = proceso_encontrado->pid;
			instruccion_nula->cantidad_elementos = 1;
			instruccion_nula->array = string_split("EXIT", " ");

			serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_nula);
			enviar_paquete(paquete_instruccion, argumentos->memoria.sockets.socket_cpu);

			free(instruccion_recibida);
			break;
		}

		// Actualizo la instruccion solicitada en el PC del proceso para saber cual fue la ultima solicitada por CPU
		proceso_encontrado->pc = instruccion_recibida->program_counter;

		t_memoria_cpu_instruccion *instruccion_proxima = list_get(proceso_encontrado->instrucciones, instruccion_recibida->program_counter);

		for (int i = 0; i < instruccion_proxima->cantidad_elementos; i++)
		{
			log_debug(argumentos->logger, "Instruccion en posicion %d: %s", i, instruccion_proxima->array[i]);
		}

		t_paquete *paquete_instruccion = crear_paquete(MEMORIA_CPU_PROXIMA_INSTRUCCION);
		serializar_t_memoria_cpu_instruccion(&paquete_instruccion, instruccion_proxima);
		enviar_paquete(paquete_instruccion, argumentos->memoria.sockets.socket_cpu);

		log_debug(argumentos->logger, "Instruccion con PID %d enviada a CPU", instruccion_recibida->pid);

		free(instruccion_recibida);
		eliminar_paquete(paquete_instruccion);
		break;
	}
	case CPU_MEMORIA_TAM_PAGINA:
	{
		t_paquete *paquete = crear_paquete(MEMORIA_CPU_TAM_PAGINA);
		uint32_t sizepagina = argumentos->memoria.tamPagina;

		serializar_t_memoria_cpu_tam_pagina(&paquete, sizepagina);

		enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
		eliminar_paquete(paquete);
		break;
	}
	case CPU_MEMORIA_NUMERO_FRAME:
	{
		t_cpu_memoria_numero_frame *proceso_recibido = deserializar_t_cpu_memoria_numero_frame(buffer);

		t_proceso *proceso = buscar_proceso(argumentos, proceso_recibido->pid);

		// Verifico que el proceso exista en la lista de procesos globales
		if (proceso == NULL)
		{
			log_error(argumentos->logger, "No se encontro el proceso con PID <%d> en Memoria", proceso_recibido->pid);
			free(proceso);
			break;
		}

		t_paquete *paquete = crear_paquete(MEMORIA_CPU_NUMERO_FRAME);

		t_memoria_cpu_numero_marco *proceso_enviar = malloc(sizeof(t_memoria_cpu_numero_marco));

		proceso_enviar->pid = proceso_recibido->pid;
		proceso_enviar->numero_pagina = proceso_recibido->numero_pagina;

		// Si el proceso existe, verifico si la pagina solicitada existe
		proceso_enviar->numero_marco = tabla_paginas_acceder_pagina(argumentos, proceso, proceso_recibido->numero_pagina);

		// Si la pagina no existe,
		if (proceso_enviar->numero_marco == -1)
		{
			log_error(argumentos->logger, "No se encontro la página para el proceso con PID <%d> y pagina <%d>", proceso->pid, proceso_recibido->numero_pagina);
			proceso_enviar->resultado = 0;
		}
		else // Si la pagina existe
		{
			log_debug(argumentos->logger, "Se encontro el marco <%d> para el proceso con PID <%d> y pagina <%d>", proceso_enviar->numero_marco, proceso->pid, proceso_recibido->numero_pagina);
			proceso_enviar->resultado = 1;
		}

		serializar_t_memoria_cpu_numero_marco(&paquete, proceso_enviar);
		enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);

		eliminar_paquete(paquete);
		free(proceso_recibido);
		free(proceso_enviar);
		break;
	}
	case CPU_MEMORIA_COPY_STRING_2:
	{
		t_copy_string *proceso_recibido = deserializar_t_copy_string(buffer);

		log_debug(argumentos->logger, "Se recibio una petición de busqueda de dato de la dirección física <%d> del registro SI perteneciente al proceso PID <%d> asociado a la instruccion <COPY_STRING>", proceso_recibido->direccion_si, proceso_recibido->pid);

		t_paquete *paquete = crear_paquete(MEMORIA_CPU_COPY_STRING_2);
		t_copy_string *proceso_enviar = malloc(sizeof(t_copy_string));

		// Busco el proceso en la lista de procesos globales
		t_proceso *proceso = buscar_proceso(argumentos, proceso_recibido->pid);

		// Si el proceso no existe, envio un mensaje a CPU
		if (proceso == NULL)
		{
			log_error(argumentos->logger, "No se encontro el proceso con PID <%d> en Memoria", proceso_recibido->pid);

			// Le devuelvo lo que me envio, pero con resultado 0 que indica que fallo
			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->resultado = 0;
			proceso_enviar->marco_si = proceso_recibido->marco_si;
			proceso_enviar->marco_di = proceso_recibido->marco_di;
			proceso_enviar->num_pagina_si = proceso_recibido->num_pagina_si;
			proceso_enviar->num_pagina_di = proceso_recibido->num_pagina_di;
			proceso_enviar->direccion_si = proceso_recibido->direccion_si;
			proceso_enviar->direccion_di = proceso_recibido->direccion_di;
			proceso_enviar->cant_bytes = proceso_recibido->cant_bytes;
			proceso_enviar->frase = strdup("No se encontro el proceso con PID solicitado");
			proceso_enviar->size_frase = strlen(proceso_enviar->frase) + 1;

			serializar_t_copy_string(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(proceso_enviar->frase);
			free(proceso_enviar);
			free(proceso_recibido->frase);
			free(proceso_recibido);

			break;
		}

		// Si la pagina existe y los bytes a leer pertenecen al proceso, envio el marco recuperado de la tabla de paginas a CPU
		proceso_enviar->frase = espacio_usuario_leer_char(argumentos, proceso, proceso_recibido->direccion_si, proceso_recibido->cant_bytes);

		// Si el dato no existe, envio un mensaje a CPU
		if (proceso_enviar->frase == NULL)
		{
			log_error(argumentos->logger, "No se encontro el dato registro SI perteneciente al proceso PID <%d> asociado a la instruccion <COPY_STRING>", proceso_recibido->pid);

			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->resultado = 0;
			proceso_enviar->marco_si = proceso_recibido->marco_si;
			proceso_enviar->marco_di = proceso_recibido->marco_di;
			proceso_enviar->num_pagina_si = proceso_recibido->num_pagina_si;
			proceso_enviar->num_pagina_di = proceso_recibido->num_pagina_di;
			proceso_enviar->direccion_si = proceso_recibido->direccion_si;
			proceso_enviar->direccion_di = proceso_recibido->direccion_di;
			proceso_enviar->cant_bytes = proceso_recibido->cant_bytes;
			proceso_enviar->frase = strdup("No se encontro la frase");
			proceso_enviar->size_frase = strlen(proceso_enviar->frase) + 1;

			serializar_t_copy_string(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(proceso_enviar->frase);
			free(proceso_enviar);
			free(proceso_recibido->frase);
			free(proceso_recibido);
			break;
		}

		log_debug(argumentos->logger, "Se encontro el dato <%s> en la direccion fisica <%d> del registro SI perteneciente al proceso PID <%d>", proceso_enviar->frase, proceso_recibido->direccion_si, proceso_recibido->pid);

		// Escribo en la dirección física perteneciente al registro DI del proceso PID
		int resultado = espacio_usuario_escribir_char(argumentos, proceso, proceso_recibido->direccion_di, proceso_enviar->frase);

		// Si no se pudo escribir la frase a partir de la direccion fisica del registro DI, envio un mensaje a CPU
		if (resultado == -1)
		{
			log_error(argumentos->logger, "No se pudo escribir la frase en la direccion fisica <%d> del registro DI perteneciente al proceso PID <%d>", proceso_recibido->direccion_di, proceso_recibido->pid);

			proceso_enviar->pid = proceso_recibido->pid;
			proceso_enviar->resultado = 0;
			proceso_enviar->marco_si = proceso_recibido->marco_si;
			proceso_enviar->marco_di = proceso_recibido->marco_di;
			proceso_enviar->num_pagina_si = proceso_recibido->num_pagina_si;
			proceso_enviar->num_pagina_di = proceso_recibido->num_pagina_di;
			proceso_enviar->direccion_si = proceso_recibido->direccion_si;
			proceso_enviar->direccion_di = proceso_recibido->direccion_di;
			proceso_enviar->cant_bytes = proceso_recibido->cant_bytes;
			proceso_enviar->frase = strdup("No se pudo escribir el registro DI");
			proceso_enviar->size_frase = strlen(proceso_enviar->frase) + 1;

			serializar_t_copy_string(&paquete, proceso_enviar);
			enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
			eliminar_paquete(paquete);

			free(proceso_enviar->frase);
			free(proceso_enviar);
			free(proceso_recibido->frase);
			free(proceso_recibido);

			break;
		}

		log_debug(argumentos->logger, "Se escribió el dato correctamente en la dirección fisica del registro DI perteneciente al proceso PID <%d>", proceso_recibido->pid);

		proceso_enviar->pid = proceso_recibido->pid;
		proceso_enviar->resultado = 1;
		proceso_enviar->marco_si = proceso_recibido->marco_si;
		proceso_enviar->marco_di = proceso_recibido->marco_di;
		proceso_enviar->num_pagina_si = proceso_recibido->num_pagina_si;
		proceso_enviar->num_pagina_di = proceso_recibido->num_pagina_di;
		proceso_enviar->direccion_si = proceso_recibido->direccion_si;
		proceso_enviar->direccion_di = proceso_recibido->direccion_di;
		proceso_enviar->cant_bytes = proceso_recibido->cant_bytes;
		proceso_enviar->frase = strdup(proceso_enviar->frase);
		proceso_enviar->size_frase = strlen(proceso_enviar->frase) + 1;

		serializar_t_copy_string(&paquete, proceso_enviar);
		enviar_paquete(paquete, argumentos->memoria.sockets.socket_cpu);
		eliminar_paquete(paquete);

		free(proceso_recibido->frase);
		free(proceso_recibido);
		free(proceso_enviar->frase);
		free(proceso_enviar);

		break;
	}
	default:
	{
		log_warning(argumentos->logger, "[CPU] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&argumentos->memoria.sockets.socket_cpu);
		break;
	}
	}
}