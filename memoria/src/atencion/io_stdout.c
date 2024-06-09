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

		char *lectura = espacio_usuario_leer_char(argumentos->argumentos, paquete_recibido->direccion_fisica, paquete_recibido->tamanio);

		if (lectura == NULL)
		{
			log_error(argumentos->argumentos->logger, "No se pudo leer del espacio de usuario");

			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->tamanio = paquete_recibido->tamanio;
			paquete_enviar->dato = strdup("ERROR");
			paquete_enviar->resultado = 0;
			paquete_enviar->size_dato = strlen(lectura) + 1;
			paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;
		}
		else
		{
			log_debug(argumentos->argumentos->logger, "Se leyo del espacio de usuario asociado a la direccion fisica %d la siguiente cadena: %s", paquete_recibido->direccion_fisica, lectura);

			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->tamanio = paquete_recibido->tamanio;
			paquete_enviar->dato = strdup(lectura);
			paquete_enviar->resultado = 1;
			paquete_enviar->size_dato = strlen(lectura) + 1;
			paquete_enviar->direccion_fisica = paquete_recibido->direccion_fisica;
			
			free(lectura);
		}

		serializar_t_memoria_io_stdout(&paquete, paquete_enviar);
		enviar_paquete(paquete, argumentos->entrada_salida->socket);
		eliminar_paquete(paquete);
		free(paquete_enviar->dato);
		free(paquete_enviar);
		free(paquete_recibido);
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