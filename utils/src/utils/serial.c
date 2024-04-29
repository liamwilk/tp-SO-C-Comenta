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

	void *stream = malloc(size);
	recv(socket_cliente, stream, size, MSG_WAITALL);

	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = size;
	buffer->offset = offset;
	buffer->stream = stream;

	return buffer;
}

t_paquete *recibir_paquete(t_log *logger, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	recv(socket_cliente, &(paquete->codigo_operacion), sizeof(op_code), MSG_WAITALL);
	recv(socket_cliente, &(paquete->size_buffer), sizeof(uint32_t), MSG_WAITALL);
	paquete->buffer = recibir_buffer(socket_cliente);
	return paquete;
}

void revisar_paquete(t_paquete *paquete, t_log *logger, int flag, char *modulo)
{
	// Si el paquete es distinto de TERMINAR y flag es 1, entonces se loguea porque indica que se recibio un paquete de un modulo.
	if (paquete->codigo_operacion != TERMINAR && flag == 1)
	{
		log_debug(logger, "Paquete recibido de modulo %s\n", modulo);
		log_debug(logger, "Deserializado del paquete:");
		log_info(logger, "Codigo de operacion: %d", paquete->codigo_operacion);
		log_debug(logger, "Size del buffer en paquete: %d", paquete->size_buffer);
		log_debug(logger, "Deserializado del buffer:");
		log_debug(logger, "Size del stream: %d", paquete->buffer->size);
		log_debug(logger, "Offset del stream: %d", paquete->buffer->offset);

		if (paquete->size_buffer != paquete->buffer->size + (2 * sizeof(uint32_t)))
		{
			log_warning(logger, "Error en el tamaño del buffer. Se esperaba %d y se recibio %ld", paquete->size_buffer, paquete->buffer->size + (2 * sizeof(uint32_t)));
		}

		if (paquete->buffer->offset != paquete->buffer->size)
		{
			log_warning(logger, "Error en el offset del buffer. Se esperaba %d y se recibio %d. Esto indica que serializaste mal", paquete->buffer->size, paquete->buffer->offset);
		}
	}
}

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

void serializar_uint32_t(uint32_t valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, &valor, sizeof(uint32_t));
	paquete->buffer->offset += sizeof(uint32_t);
}

void serializar_char(char *valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, valor, strlen(valor) + 1);
	paquete->buffer->offset += strlen(valor) + 1;
}

void actualizar_buffer(t_paquete *paquete, uint32_t size)
{
	// Estos dos uint32_t son por el offset y el size del stream en el buffer.
	paquete->size_buffer = size + (sizeof(uint32_t) * 2);
	// El tamaño del stream es el que nos dan
	paquete->buffer->size = size;
	// Reservo el espacio necesario para el stream
	paquete->buffer->stream = malloc(paquete->buffer->size);
}