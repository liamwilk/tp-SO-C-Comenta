#include <utils/memoria.h>

void switch_case_memoria_entrada_salida_stdin(t_args_hilo *argumentos, char *modulo, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case ENTRADA_SALIDA_MEMORIA_IO_STDIN_READ:
	{
		t_io_memoria_stdin *paquete_recibido = deserializar_t_io_memoria_stdin(buffer);

		log_debug(argumentos->argumentos->logger, "Se solicito escribir <%ld> bytes en el espacio de usuario desde a la direccion fisica <%d>", strlen(paquete_recibido->input), paquete_recibido->direccion_fisica);

		t_proceso *proceso = buscar_proceso(argumentos->argumentos, paquete_recibido->pid);

		int resultado = espacio_usuario_escribir_char(argumentos->argumentos, paquete_recibido->direccion_fisica, paquete_recibido->input);
		proceso->bytes_usados += strlen(paquete_recibido->input);

		t_paquete *paquete = crear_paquete(MEMORIA_ENTRADA_SALIDA_IO_STDIN_READ);
		t_memoria_entrada_salida_io_stdin_read *paquete_enviar = malloc(sizeof(t_memoria_entrada_salida_io_stdin_read));

		if (resultado == -1)
		{
			log_error(argumentos->argumentos->logger, "No se pudo escribir en el espacio de usuario");
			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->resultado = 0;
		}
		else
		{
			log_debug(argumentos->argumentos->logger, "Se escribieron <%ld> bytes en el espacio de usuario desde a la direccion fisica <%d>", strlen(paquete_recibido->input), paquete_recibido->direccion_fisica);
			paquete_enviar->pid = paquete_recibido->pid;
			paquete_enviar->resultado = 1;
		}

		serializar_t_memoria_entrada_salida_io_stdin_read(&paquete, paquete_enviar);
		enviar_paquete(paquete, argumentos->entrada_salida->socket);

		eliminar_paquete(paquete);
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