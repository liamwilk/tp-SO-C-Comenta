#include "handshake.h"

t_handshake crear_handshake(t_log *logger, int socket_servidor, t_handshake codigo_a_recibir, char *modulo)
{
	t_handshake respuesta;

	send(socket_servidor, &codigo_a_recibir, sizeof(t_handshake), 0);
	recv(socket_servidor, &respuesta, sizeof(t_handshake), MSG_WAITALL);

	if (respuesta == CORRECTO)
	{
		log_info(logger, "[%s] Conexion por handshake realizada y establecida.", modulo);
	}
	else
	{
		log_error(logger, "[%s] Error al crear handshake.", modulo);
	}

	return respuesta;
}

t_handshake conexion_handshake_recibir(t_log *logger, int socket_cliente, t_handshake codigo_esperado, char *modulo)
{
	t_handshake respuesta;
	t_handshake ok = CORRECTO;
	t_handshake error = ERROR;

	recv(socket_cliente, &respuesta, sizeof(t_handshake), MSG_WAITALL);

	if (respuesta == codigo_esperado)
	{
		send(socket_cliente, &ok, sizeof(t_handshake), 0);
		log_info(logger, "[%s] Conexion por handshake recibida y establecida.", modulo);
		respuesta = ok;
	}
	else
	{
		send(socket_cliente, &error, sizeof(t_handshake), 0);
		log_error(logger, "[%s] Error al recibir handshake.", modulo);
		respuesta = error;
	}

	return respuesta;
}

t_handshake esperar_handshake_entrada_salida(t_log *logger, int socket_cliente)
{
	t_handshake respuesta;
	t_handshake ok = CORRECTO;
	t_handshake error = ERROR;

	recv(socket_cliente, &respuesta, sizeof(t_handshake), MSG_WAITALL);

	char *modulo;

	switch (respuesta)
	{
	case KERNEL_ENTRADA_SALIDA_GENERIC:
		modulo = "Entrada/Salida Generico";
		break;
	case KERNEL_ENTRADA_SALIDA_STDIN:
		modulo = "Entrada/Salida STDIN";
		break;
	case KERNEL_ENTRADA_SALIDA_STDOUT:
		modulo = "Entrada/Salida STDOUT";
		break;
	case KERNEL_ENTRADA_SALIDA_DIALFS:
		modulo = "Entrada/Salida DIALFS";
		break;
	default:
		respuesta = ERROR;
		modulo = "Desconocido";
		log_error(logger, "Error al recibir handshake. Modulo desconocido");
		break;
	}

	if (respuesta != error)
	{
		send(socket_cliente, &ok, sizeof(t_handshake), 0);
		log_info(logger, "[%s] Conexion por handshake recibida y establecida.", modulo);
	}
	else
	{
		send(socket_cliente, &error, sizeof(t_handshake), 0);
		log_error(logger, "[%s] Error al recibir handshake.", modulo);
		respuesta = error;
	}

	return respuesta;
}