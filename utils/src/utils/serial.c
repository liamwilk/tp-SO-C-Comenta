#include "serial.h"

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *buffer = malloc(bytes);
	int desplazamiento = 0;

	memcpy(buffer + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return buffer;
};

void *deserializar_paquete(t_buffer *buffer)
{
	// TODO: Implementar
	return NULL;
};

void *recibir_stream(int socket_cliente)
{
	void *buffer_void = recibir_buffer(socket_cliente);
	t_buffer buffer_struct;
	memcpy(&buffer_struct.size, buffer_void, sizeof(int));
	buffer_struct.stream = malloc(buffer_struct.size);
	memcpy(buffer_struct.stream, buffer_void + sizeof(int), buffer_struct.size);
	free(buffer_void);
	return buffer_struct.stream;
};

void enviar_mensaje(char *mensaje, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
};

void crear_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete *crear_paquete(op_code codigo_de_operacion)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_de_operacion;
	crear_buffer(paquete);

	return paquete;
};

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
};

void *recibir_buffer(int socket_cliente)
{
	void *buffer;

	int *size = malloc(sizeof(int));

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);

	buffer = malloc(*size);

	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

// No usar, esto no funciona
t_paquete *recibir_paquete(int socket)
{
	t_paquete *ret = malloc(sizeof(t_paquete));

	// Obtengo el tamanio del paquete
	uint64_t *size = malloc(sizeof(uint64_t));
	if (recv(socket, size, sizeof(uint64_t), MSG_WAITALL) < 1)
	{
		free(ret);
		free(size);
		return NULL;
	}

	ret->size = *size;
	free(size);

	// Obtengo el opcode
	op_code *opcode = malloc(sizeof(op_code));
	if (recv(socket, opcode, sizeof(op_code), MSG_WAITALL) < 1)
	{
		free(ret);
		free(opcode);
		return NULL;
	}
	ret->codigo_operacion = *opcode;
	free(opcode);

	// Obtengo el buffer
	ret->buffer = malloc(ret->size);
	if (ret->size != 0 && recv(socket, ret->buffer, ret->size, MSG_WAITALL) < 1)
	{
		free(ret->buffer);
		free(ret);
		return NULL;
	}
	return ret;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}
