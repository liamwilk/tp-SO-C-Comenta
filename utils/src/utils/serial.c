#include "serial.h"

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
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

t_paquete *crear_paquete(void)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
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

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
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

t_buffer *buffer_create(uint32_t size)
{
	t_buffer *buffer = malloc(sizeof(t_buffer)); // reservamos memoria para almacenar en algún lugar los datos del buffer
	buffer->stream = NULL;
	buffer->size = size;
	return buffer;
}

void buffer_destroy(t_buffer *buffer)
{
	free(buffer->stream);
	free(buffer);
}

void buffer_add(t_buffer *buffer, void *data, uint32_t size)
{
	buffer->stream = realloc(buffer->stream, (size + sizeof(uint32_t)));				 // expandimos el tamaño del stream que almacena el buffer
	memcpy(buffer->stream + buffer->size, &size, sizeof(uint32_t));						 // copiamos el tamaño del próximo dato que almacena
	memcpy(buffer->stream + buffer->size + buffer->size + sizeof(uint32_t), data, size); // copiamos el dato en cuestión
}

void buffer_read(t_buffer *buffer, void *data, uint32_t size)
{

	if (buffer == NULL || buffer->stream == NULL)
	{
		printf("ERROR: el puntero a buffer no apunta a ningun buffer \n");
		return; // de esta forma se evitan los condicionales anidados
	}

	if (buffer->size == 0 || size == 0)
	{
		printf("ERROR: no hay datos para leer en el buffer \n");
		return;
	}

	if (buffer->size < size)
	{
		printf("ERROR: el buffer es menor a la cantidad de memoria que se desea leer \n");
		return;
	}

	memcpy(data, buffer->stream, size);
}
