#include <utils/memoria.h>

void switch_case_memoria_entrada_salida_stdout(t_args_hilo *argumentos, char* modulo,  t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case MEMORIA_ENTRADA_SALIDA_IDENTIFICACION:
	{
		t_entrada_salida_identificacion *identificacion = deserializar_t_entrada_salida_identificacion(buffer);

		if (buscar_interfaz(argumentos->argumentos, identificacion->identificador) != NULL)
		{
			agregar_identificador_rechazado(argumentos, "no identificada");
			log_warning(argumentos->argumentos->logger,  "[%s/%d] Se rechazo identificacion, identificador %s ocupado. Cierro hilo.", modulo, argumentos->entrada_salida->orden, identificacion->identificador);
			
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
		log_warning(argumentos->argumentos->logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo",modulo);
		liberar_conexion(&argumentos->argumentos->memoria.sockets.socket_entrada_salida_stdin);
		break;
	}
	}
}