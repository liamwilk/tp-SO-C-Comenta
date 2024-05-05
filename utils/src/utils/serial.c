#include "serial.h"

void *serializar_paquete(t_paquete *paquete, uint32_t bytes)
{
	void *buffer = malloc(bytes);
	int desplazamiento = 0;

	memcpy(buffer + desplazamiento, &(paquete->codigo_operacion), sizeof(t_op_code));
	desplazamiento += sizeof(t_op_code);
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

t_paquete *crear_paquete(t_op_code codigo_de_operacion)
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
	uint32_t bytes = paquete->buffer->size + (3 * sizeof(uint32_t)) + sizeof(t_op_code);
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
	ssize_t bytes_recibidos;

	bytes_recibidos = recv(socket_cliente, &size, sizeof(uint32_t), MSG_WAITALL);

	if (bytes_recibidos == -1)
	{
		printf("Error al recibir el buffer por socket %d dentro de recibir_buffer, paso 1.\n", socket_cliente);
		close(socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		printf("Conexion por socket %d cerrada en el otro extremo.\n", socket_cliente);
		close(socket_cliente);
		return NULL;
	}

	bytes_recibidos = recv(socket_cliente, &offset, sizeof(uint32_t), MSG_WAITALL) == -1;

	if (bytes_recibidos == -1)
	{
		printf("Error al recibir el buffer por socket %d dentro de recibir_buffer, paso 2.\n", socket_cliente);
		close(socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		printf("Conexion por socket %d cerrada en el otro extremo.\n", socket_cliente);
		close(socket_cliente);
		return NULL;
	}

	void *stream = malloc(size);

	bytes_recibidos = recv(socket_cliente, stream, size, MSG_WAITALL);

	if (bytes_recibidos == -1)
	{
		free(stream);
		printf("Error al recibir el buffer por socket %d dentro de recibir_buffer, paso 3.\n", socket_cliente);
		close(socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		free(stream);
		printf("Conexion por socket %d cerrada en el otro extremo.\n", socket_cliente);
		close(socket_cliente);
		return NULL;
	}

	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = size;
	buffer->offset = offset;
	buffer->stream = stream;

	return buffer;
}

t_paquete *recibir_paquete(t_log *logger, int socket_cliente)
{

	t_paquete *paquete = malloc(sizeof(t_paquete));

	ssize_t bytes_recibidos;

	bytes_recibidos = recv(socket_cliente, &(paquete->codigo_operacion), sizeof(t_op_code), MSG_WAITALL);

	if (bytes_recibidos == -1)
	{
		log_error(logger, "Error al recibir el codigo de operacion: %s por socket %d en recibir_paquete, paso 1.", strerror(errno), socket_cliente);
		free(paquete);
		close(socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_error(logger, "Conexion por socket %d cerrada en el otro extremo", socket_cliente);
		free(paquete);
		close(socket_cliente);
		return NULL;
	}

	bytes_recibidos = recv(socket_cliente, &(paquete->size_buffer), sizeof(uint32_t), MSG_WAITALL) == -1;

	if (bytes_recibidos == -1)
	{
		log_error(logger, "Error al recibir el codigo de operacion: %s por socket %d en recibir_paquete, paso 2.", strerror(errno), socket_cliente);
		free(paquete);
		close(socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_error(logger, "Conexion por socket %d cerrada en el otro extremo", socket_cliente);
		free(paquete);
		close(socket_cliente);
		return NULL;
	}

	// Lo guardo porque recibir_buffer me lo cierra si da error y quiero mostrarlo en el log
	int socket = socket_cliente;

	paquete->buffer = recibir_buffer(socket_cliente);

	if (paquete->buffer == NULL)
	{
		log_error(logger, "Error al recibir el buffer. Se cierra la conexion por socket %d", socket);
		return NULL;
	}

	return paquete;
}

void revisar_paquete(t_paquete *paquete, t_log *logger, int flag, char *modulo)
{
	if (flag != 0)
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
	t_op_code cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(t_op_code), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void serializar_uint64_t(uint64_t valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, &valor, sizeof(uint64_t));
	paquete->buffer->offset += sizeof(uint64_t);
}

void serializar_uint32_t(uint32_t valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, &valor, sizeof(uint32_t));
	paquete->buffer->offset += sizeof(uint32_t);
}

void serializar_uint16_t(uint16_t valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, &valor, sizeof(uint16_t));
	paquete->buffer->offset += sizeof(uint16_t);
}

void serializar_uint8_t(uint8_t valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, &valor, sizeof(uint8_t));
	paquete->buffer->offset += sizeof(uint8_t);
}

void serializar_bool(bool valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, &valor, sizeof(bool));
	paquete->buffer->offset += sizeof(bool);
}

void deserializar_bool(void **flujo, bool *destino_del_dato)
{
	memcpy(destino_del_dato, *flujo, sizeof(bool));
	*flujo += sizeof(bool);
}

void serializar_op_code(t_op_code valor, t_paquete *paquete)
{
	memcpy(paquete->buffer->stream + paquete->buffer->offset, &valor, sizeof(t_op_code));
	paquete->buffer->offset += sizeof(t_op_code);
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

void deserializar_char(void **flujo, char **destino_del_dato, uint32_t size_del_dato)
{
	*destino_del_dato = malloc(size_del_dato);
	memcpy(*destino_del_dato, *flujo, size_del_dato);
	*flujo += size_del_dato * sizeof(char);
}

void deserializar_op_code(void **flujo, t_op_code *destino_del_dato)
{
	memcpy(destino_del_dato, *flujo, sizeof(t_op_code));
	*flujo += sizeof(t_op_code);
}

void deserializar_uint8_t(void **flujo, uint8_t *destino_del_dato)
{
	memcpy(destino_del_dato, *flujo, sizeof(uint8_t));
	*flujo += sizeof(uint8_t);
}

void deserializar_uint16_t(void **flujo, uint16_t *destino_del_dato)
{
	memcpy(destino_del_dato, *flujo, sizeof(uint16_t));
	*flujo += sizeof(uint16_t);
}

void deserializar_uint32_t(void **flujo, uint32_t *destino_del_dato)
{
	memcpy(destino_del_dato, *flujo, sizeof(uint32_t));
	*flujo += sizeof(uint32_t);
}

void deserializar_uint64_t(void **flujo, uint64_t *destino_del_dato)
{
	memcpy(destino_del_dato, *flujo, sizeof(uint64_t));
	*flujo += sizeof(uint64_t);
}

void serializar_t_kernel_memoria_proceso(t_paquete **paquete, t_kernel_memoria_proceso *proceso)
{
	actualizar_buffer(*paquete, proceso->size_path + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
	serializar_uint32_t(proceso->size_path, *paquete);
	serializar_char(proceso->path_instrucciones, *paquete);
	serializar_uint32_t(proceso->program_counter, *paquete);
	serializar_uint32_t(proceso->pid, *paquete);
}

t_memoria_kernel_proceso *deserializar_t_memoria_kernel_proceso(t_buffer *buffer)
{
	t_memoria_kernel_proceso *proceso = malloc(sizeof(t_memoria_kernel_proceso));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(proceso->pid));
	deserializar_uint32_t(&stream, &(proceso->cantidad_instruccions));
	deserializar_bool(&stream, &(proceso->leido));

	return proceso;
}

void serializar_t_registros_cpu(t_paquete **paquete, uint32_t pid, t_registros_cpu *registros)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint8_t));
	serializar_uint32_t(pid, *paquete);
	serializar_uint32_t(registros->pc, *paquete);
	serializar_uint32_t(registros->eax, *paquete);
	serializar_uint32_t(registros->ebx, *paquete);
	serializar_uint8_t(registros->ax, *paquete);
	serializar_uint8_t(registros->bx, *paquete);
	serializar_uint8_t(registros->cx, *paquete);
	serializar_uint8_t(registros->dx, *paquete);
}

void serializar_t_kernel_memoria_finalizar_proceso(t_paquete **paquete, t_kernel_memoria_finalizar_proceso *proceso)
{
	actualizar_buffer(*paquete, sizeof(uint32_t));
	serializar_uint32_t(proceso->pid, *paquete);
}
