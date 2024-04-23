#include "handshake.h"

uint32_t handshake(t_log *logger, int conexion, uint32_t envio, char *modulo)
{
	uint32_t result;

	send(conexion, &envio, sizeof(uint32_t), 0);
	recv(conexion, &result, sizeof(uint32_t), MSG_WAITALL);

	if (result == 0)
	{
		log_info(logger, "[%s] Conexion por handshake establecida.", modulo);
	}
	else
	{
		log_error(logger, "[%s] Error en la conexión.", modulo);
		return -1;
	}
	return result;
}

int esperar_handshake(int nuevoSocket)
{
	uint32_t ok = 0;
	uint32_t error = -1;
	int respuesta;
	recv(nuevoSocket, &respuesta, sizeof(uint32_t), MSG_WAITALL);
	if (respuesta == 1)
	{
		send(nuevoSocket, &ok, sizeof(uint32_t), 0);
	}
	else
	{
		send(nuevoSocket, &error, sizeof(uint32_t), 0);
	}
	return nuevoSocket;
}