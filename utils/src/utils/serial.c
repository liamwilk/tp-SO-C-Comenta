#include "serial.h"

void *serializar_paquete(t_paquete *paquete, uint32_t bytes)
{
	void *buffer = malloc(bytes);
	int desplazamiento = 0;

	memcpy(buffer + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento += sizeof(op_code);
	memcpy(buffer + desplazamiento, &(paquete->size_buffer), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(buffer + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(buffer + desplazamiento, &(paquete->buffer->offset), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
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
	// Esto va a dar segmentation fault si le envian un paquete que tiene NULL en el stream. No lo hagan :)
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
	paquete->buffer->offset = 0;
	paquete->buffer->stream = NULL;
}

t_paquete *crear_paquete(op_code codigo_de_operacion)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_de_operacion;
	paquete->size_buffer = 0;
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
	uint32_t bytes = paquete->buffer->size + (3 * sizeof(uint32_t)) + sizeof(op_code);
	void *buffer_intermedio = serializar_paquete(paquete, bytes);
	send(socket_cliente, buffer_intermedio, bytes, 0);
	free(buffer_intermedio);
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
};

t_buffer *recibir_buffer(int socket_cliente)
{
	uint32_t size;
	uint32_t offset;

	recv(socket_cliente, &size, sizeof(uint32_t), MSG_WAITALL);
	recv(socket_cliente, &offset, sizeof(uint32_t), MSG_WAITALL);

	void *buffer = malloc(size);
	recv(socket_cliente, buffer, size, MSG_WAITALL);

	t_buffer *ret = malloc(sizeof(t_buffer));
	ret->size = size;
	ret->offset = offset;
	ret->stream = buffer;

	return ret;
}

t_paquete *recibir_paquete(int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	recv(socket_cliente, &(paquete->codigo_operacion), sizeof(op_code), MSG_WAITALL);
	recv(socket_cliente, &(paquete->size_buffer), sizeof(uint32_t), MSG_WAITALL);

	paquete->buffer = recibir_buffer(socket_cliente);

	return paquete;
}

// No usar, esto no funciona
// t_paquete *recibir_paquete(int socket)
// {
// 	t_paquete *ret = malloc(sizeof(t_paquete));

// 	// Obtengo el tamanio del paquete
// 	uint64_t *size = malloc(sizeof(uint64_t));
// 	if (recv(socket, size, sizeof(uint64_t), MSG_WAITALL) < 1)
// 	{
// 		free(ret);
// 		free(size);
// 		return NULL;
// 	}

// 	ret->size_buffer = *size;
// 	free(size);

// 	// Obtengo el opcode
// 	op_code *opcode = malloc(sizeof(op_code));
// 	if (recv(socket, opcode, sizeof(op_code), MSG_WAITALL) < 1)
// 	{
// 		free(ret);
// 		free(opcode);
// 		return NULL;
// 	}
// 	ret->codigo_operacion = *opcode;
// 	free(opcode);

// 	// Obtengo el buffer
// 	ret->buffer = malloc(ret->size_buffer);
// 	if (ret->size_buffer != 0 && recv(socket, ret->buffer, ret->size_buffer, MSG_WAITALL) < 1)
// 	{
// 		free(ret->buffer);
// 		free(ret);
// 		return NULL;
// 	}
// 	return ret;
// }

int recibir_operacion(int socket_cliente)
{
	op_code cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(op_code), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}
