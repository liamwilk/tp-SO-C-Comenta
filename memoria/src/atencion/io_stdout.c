#include <utils/memoria.h>

void switch_case_memoria_entrada_salida_stdout(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case ENTRADA_SALIDA_MEMORIA_IO_STDOUT_WRITE:
	{
		t_io_memoria_stdout *paquete_recibido = deserializar_t_io_memoria_stdout(buffer);

		log_debug(argumentos->argumentos->logger, "Se recibio la solicitud de lectura de %d bytes en la direccion fisica %d", paquete_recibido->tamanio, paquete_recibido->direccion_fisica);

		t_paquete *paquete = crear_paquete(MEMORIA_ENTRADA_SALIDA_IO_STDOUT_WRITE);
		t_memoria_io_stdout *paquete_enviar = malloc(sizeof(t_memoria_io_stdout));
		t_proceso *proceso = buscar_proceso(argumentos->argumentos, paquete_recibido->pid);

		/*
		Para leer de memoria, debo buscar en que marco se encuentra la direccion fisica de referencia.

		Los pasos correctos son:

		A partir de la direccion fisica recibida, calcular:
		 - El marco en el que se encuentra
		 - El desplazamiento dentro del primer marco

		Luego, buscar en que pagina se encuentra ese marco y obtener el marco de pagina correspondiente.

		Una vez que tengo el numero de pagina, ya se cuantas paginas contiguas debo leer, itero sobre ellas y leo el contenido de cada una.
		*/

		// Calculo el marco en el que se encuentra la direccion fisica
		int marco = paquete_recibido->direccion_fisica / argumentos->argumentos->memoria.tamPagina;

		// Calculo el desplazamiento dentro del marco
		int desplazamiento = paquete_recibido->direccion_fisica % argumentos->argumentos->memoria.tamPagina;

		int pagina = -1;

		// Busco la pagina en la que se encuentra el marco
		for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
		{
			t_pagina *pagina_recibida = list_get(proceso->tabla_paginas, i);

			if (pagina_recibida->validez == 1 && pagina_recibida->marco == marco)
			{
				pagina = i;
				break;
			}
		}

		if (pagina == -1)
		{
			log_error(argumentos->argumentos->logger, "No se encontro la pagina asociada al marco <%d>", marco);
			break;
		}

		// Calculo la cantidad de paginas que debo leer, menos la primera que se lee fuera del bucle
		int paginas_a_leer = ceil((double)(paquete_recibido->tamanio) / argumentos->argumentos->memoria.tamPagina);

		char *lectura = string_new();

		// Leo el marco de la primera pagina

		uint32_t offset_size_lectura = argumentos->argumentos->memoria.tamPagina - desplazamiento;

		char *primer_lectura = espacio_usuario_leer_char(argumentos->argumentos, proceso, paquete_recibido->direccion_fisica, offset_size_lectura);

		string_append_with_format(&lectura, "%s", primer_lectura);


		int size_a_leer = paquete_recibido->tamanio;

		log_warning(argumentos->argumentos->logger, "Tamaño a leer: %d", paquete_recibido->tamanio);

		if (paginas_a_leer > 1)
		{
			// Leo el marco de pagina correspondiente
			for (int i = 1; i < paginas_a_leer; i++)
			{

				int size_lectura = argumentos->argumentos->memoria.tamPagina;

				if (size_a_leer < size_lectura)
				{
					size_lectura = size_a_leer;
				}

				t_pagina *pagina_obtenida = list_get(proceso->tabla_paginas, pagina + i);

				// Calculo la dirección fisica del marco de pagina
				int direccion_fisica = pagina_obtenida->marco * argumentos->argumentos->memoria.tamPagina;

				size_a_leer -= size_lectura;

				char *ciclo_lectura = espacio_usuario_leer_char(argumentos->argumentos, proceso, direccion_fisica, size_lectura);

				log_warning(argumentos->argumentos->logger, "Se leyo de la direccion fisica %d la siguiente cadena: %s", direccion_fisica, ciclo_lectura);

				string_append_with_format(&lectura, "%s", ciclo_lectura);

				free(ciclo_lectura);
			}
		}

		if (lectura == NULL)
		{
			log_error(argumentos->argumentos->logger, "No se pudo leer del espacio de usuario");

			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->tamanio = paquete_recibido->tamanio;
			paquete_enviar->dato = strdup("ERROR");
			paquete_enviar->resultado = 0;
			paquete_enviar->size_dato = strlen(paquete_enviar->dato) + 1;
			paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;
		}
		else
		{
			log_debug(argumentos->argumentos->logger, "Se leyo del espacio de usuario asociado a la direccion fisica %d la siguiente cadena: %s", paquete_recibido->direccion_fisica, lectura);

			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->tamanio = paquete_recibido->tamanio;
			paquete_enviar->resultado = 1;
			paquete_enviar->dato = strdup(lectura);
			paquete_enviar->size_dato = strlen(paquete_enviar->dato) + 1;
			paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;

			free(lectura);
		}

		serializar_t_memoria_io_stdout(&paquete, paquete_enviar);
		enviar_paquete(paquete, argumentos->entrada_salida->socket);
		eliminar_paquete(paquete);
		free(paquete_enviar->dato);
		free(paquete_enviar);
		free(paquete_recibido);
		free(primer_lectura);
		break;
	}
	case MEMORIA_ENTRADA_SALIDA_IDENTIFICACION:
	{
		t_entrada_salida_identificacion *identificacion = deserializar_t_entrada_salida_identificacion(buffer);

		if (buscar_interfaz(argumentos->argumentos, identificacion->identificador) != NULL)
		{
			agregar_identificador_rechazado(argumentos, "no identificada");
			log_warning(argumentos->argumentos->logger, "[%s/%d] Se rechazo identificacion, identificador %s ocupado. Cierro hilo.", modulo, argumentos->entrada_salida->orden, identificacion->identificador);

			argumentos->entrada_salida->valido = false;
			argumentos->argumentos->memoria.sockets.id_entrada_salida--;

			avisar_rechazo_identificador_memoria(argumentos->entrada_salida->socket);
			liberar_conexion(&argumentos->entrada_salida->socket);
			break;
		}

		agregar_identificador(argumentos, identificacion->identificador);

		log_debug(argumentos->argumentos->logger, "[%s/%s/%d] Se recibio identificador válido.", modulo, identificacion->identificador, argumentos->entrada_salida->orden);

		free(identificacion->identificador);
		free(identificacion);
		break;
	}
	default:
	{
		log_warning(argumentos->argumentos->logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo);
		liberar_conexion(&argumentos->argumentos->memoria.sockets.socket_entrada_salida_stdin);
		break;
	}
	}
}