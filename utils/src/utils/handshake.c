#include "handshake.h"

handshake_code crear_handshake(t_log *logger, int socket, handshake_code codigo_a_recibir, char *modulo)
{
	handshake_code respuesta;

	send(socket, &codigo_a_recibir, sizeof(handshake_code), 0);
	recv(socket, &respuesta, sizeof(handshake_code), MSG_WAITALL);

	if (respuesta == CORRECTO)
	{
		log_info(logger, "[%s] Conexion por handshake establecida.", modulo);
	}
	else
	{
		log_error(logger, "[%s] Error en la conexión.", modulo);
	}

	return respuesta;
}

handshake_code esperar_handshake(t_log *logger, int socket, handshake_code codigo_esperado, char *modulo)
{
	handshake_code respuesta;

	handshake_code ok = CORRECTO;
	handshake_code error = ERROR;

	recv(socket, &respuesta, sizeof(handshake_code), MSG_WAITALL);

	if (respuesta == codigo_esperado)
	{
		send(socket, &ok, sizeof(handshake_code), 0);
		log_info(logger, "[%s] Conexion por handshake establecida.", modulo);
		respuesta = ok;
	}
	else
	{
		send(socket, &error, sizeof(handshake_code), 0);
		log_error(logger, "[%s] Error en la conexión.", modulo);
		respuesta = error;
	}

	return respuesta;
}