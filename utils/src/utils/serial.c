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

t_buffer *recibir_buffer(t_log *logger, int socket_cliente)
{
	uint32_t size;
	uint32_t offset;
	ssize_t bytes_recibidos;

	bytes_recibidos = recv(socket_cliente, &size, sizeof(uint32_t), MSG_WAITALL);

	if (bytes_recibidos == -1)
	{
		log_warning(logger, "El cliente se desconecto del socket %d mientras recibia el tamaño del stream.", socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_warning(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el tamaño del stream.", socket_cliente);
		return NULL;
	}

	bytes_recibidos = recv(socket_cliente, &offset, sizeof(uint32_t), MSG_WAITALL);
	if (bytes_recibidos == -1)
	{
		log_warning(logger, "El cliente se desconecto del socket %d mientras recibia el offset del stream.", socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_warning(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el offset del stream.", socket_cliente);
		return NULL;
	}

	void *stream = malloc(size);

	bytes_recibidos = recv(socket_cliente, stream, size, MSG_WAITALL);

	if (bytes_recibidos == -1)
	{
		free(stream);
		log_warning(logger, "El cliente se desconecto del socket %d mientras recibia el stream.", socket_cliente);
		close(socket_cliente);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		free(stream);
		log_warning(logger, "Conexion por socket %d cerrada en el otro extremo.", socket_cliente);
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
		log_warning(logger, "El cliente se desconecto del socket %d mientras recibia el codigo de operacion.", socket_cliente);
		free(paquete);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_warning(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el codigo de operacion.", socket_cliente);
		free(paquete);
		return NULL;
	}

	bytes_recibidos = recv(socket_cliente, &(paquete->size_buffer), sizeof(uint32_t), MSG_WAITALL);
	if (bytes_recibidos == -1)
	{
		log_warning(logger, "El cliente se desconecto del socket %d mientras recibia el tamaño total del buffer.", socket_cliente);
		free(paquete);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_warning(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el tamaño total del buffer.", socket_cliente);
		free(paquete);
		return NULL;
	}

	paquete->buffer = recibir_buffer(logger, socket_cliente);

	if (paquete->buffer == NULL)
	{
		return NULL;
	}

	return paquete;
}

void revisar_paquete(t_paquete *paquete, t_log *logger, char *modulo)
{
	if (paquete->codigo_operacion != FINALIZAR_SISTEMA)
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
	}
	else
	{
		log_info(logger, "Kernel solicito el apagado del modulo.");
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

	// Verifico si la memoria fue asignada correctamente
	if (paquete->buffer->stream == NULL)
	{
		perror("Error al asignar memoria para el stream del buffer\n");
	}
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
	deserializar_uint32_t(&stream, &(proceso->cantidad_instrucciones));
	deserializar_bool(&stream, &(proceso->leido));

	return proceso;
}

t_cpu_kernel_proceso *deserializar_t_cpu_kernel_proceso(t_buffer *buffer)
{
	t_cpu_kernel_proceso *proceso = malloc(sizeof(t_cpu_kernel_proceso));
	proceso->registros = malloc(sizeof(t_registros_cpu));

	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(proceso->ejecutado));
	deserializar_uint32_t(&stream, &(proceso->pid));
	deserializar_uint32_t(&stream, &(proceso->registros->pc));
	deserializar_uint32_t(&stream, &(proceso->registros->eax));
	deserializar_uint32_t(&stream, &(proceso->registros->ebx));
	deserializar_uint32_t(&stream, &(proceso->registros->ecx));
	deserializar_uint32_t(&stream, &(proceso->registros->edx));
	deserializar_uint32_t(&stream, &(proceso->registros->si));
	deserializar_uint32_t(&stream, &(proceso->registros->di));
	deserializar_uint8_t(&stream, &(proceso->registros->ax));
	deserializar_uint8_t(&stream, &(proceso->registros->bx));
	deserializar_uint8_t(&stream, &(proceso->registros->cx));
	deserializar_uint8_t(&stream, &(proceso->registros->dx));

	return proceso;
}

t_kernel_entrada_salida_unidad_de_trabajo *deserializar_t_kernel_entrada_salida_unidad_de_trabajo(t_buffer *buffer)
{
	t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(unidad->unidad_de_trabajo));

	return unidad;
}

t_entrada_salida_kernel_unidad_de_trabajo *deserializar_t_entrada_salida_kernel_unidad_de_trabajo(t_buffer *buffer)
{
	t_entrada_salida_kernel_unidad_de_trabajo *unidad = malloc(sizeof(t_entrada_salida_kernel_unidad_de_trabajo));
	void *stream = buffer->stream;

	deserializar_bool(&stream, &(unidad->terminado));

	return unidad;
}

t_entrada_salida_kernel_finalizar *deserializar_t_entrada_salida_kernel_finalizar(t_buffer *buffer)
{
	t_entrada_salida_kernel_finalizar *unidad = malloc(sizeof(t_entrada_salida_kernel_finalizar));
	void *stream = buffer->stream;

	deserializar_bool(&stream, &(unidad->terminado));

	return unidad;
}

void serializar_t_registros_cpu(t_paquete **paquete, uint32_t pid, t_registros_cpu *registros)
{
	actualizar_buffer(*paquete, sizeof(uint8_t) * 4 + sizeof(uint32_t) * 8);
	serializar_uint32_t(pid, *paquete);
	serializar_uint32_t(registros->pc, *paquete);
	serializar_uint32_t(registros->eax, *paquete);
	serializar_uint32_t(registros->ebx, *paquete);
	serializar_uint32_t(registros->ecx, *paquete);
	serializar_uint32_t(registros->edx, *paquete);
	serializar_uint32_t(registros->si, *paquete);
	serializar_uint32_t(registros->di, *paquete);
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

void serializar_t_cpu_kernel_proceso(t_paquete **paquete, t_cpu_kernel_proceso *proceso)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 13);

	serializar_uint32_t(proceso->ejecutado, *paquete);
	serializar_uint32_t(proceso->pid, *paquete);
	serializar_uint32_t(proceso->registros->pc, *paquete);
	serializar_uint32_t(proceso->registros->eax, *paquete);
	serializar_uint32_t(proceso->registros->ebx, *paquete);
	serializar_uint32_t(proceso->registros->ecx, *paquete);
	serializar_uint32_t(proceso->registros->edx, *paquete);
	serializar_uint32_t(proceso->registros->si, *paquete);
	serializar_uint32_t(proceso->registros->di, *paquete);
	serializar_uint8_t(proceso->registros->ax, *paquete);
	serializar_uint8_t(proceso->registros->bx, *paquete);
	serializar_uint8_t(proceso->registros->cx, *paquete);
	serializar_uint8_t(proceso->registros->dx, *paquete);
}

void serializar_t_kernel_entrada_salida_unidad_de_trabajo(t_paquete **paquete, t_kernel_entrada_salida_unidad_de_trabajo *unidad)
{
	actualizar_buffer(*paquete, sizeof(uint32_t));
	serializar_uint32_t(unidad->unidad_de_trabajo, *paquete);
}

void serializar_t_entrada_salida_kernel_unidad_de_trabajo(t_paquete **paquete, t_entrada_salida_kernel_unidad_de_trabajo *unidad)
{
	actualizar_buffer(*paquete, sizeof(bool));
	serializar_bool(unidad->terminado, *paquete);
}

void serializar_t_entrada_salida_kernel_finalizar(t_paquete **paquete, t_entrada_salida_kernel_finalizar *unidad)
{
	actualizar_buffer(*paquete, sizeof(bool));
	serializar_bool(unidad->terminado, *paquete);
}

t_memoria_cpu_instruccion *deserializar_t_memoria_cpu_instruccion(t_buffer *buffer)
{
	t_memoria_cpu_instruccion *dato = malloc(sizeof(t_memoria_cpu_instruccion));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(dato->pid));
	deserializar_uint32_t(&stream, &(dato->cantidad_elementos));
	deserializar_char_array(&(dato->array), &(dato->cantidad_elementos), &stream);

	return dato;
}

void serializar_t_memoria_cpu_instruccion(t_paquete **paquete, t_memoria_cpu_instruccion *proceso)
{

	// Reservo tamaño para el pid,la cantidad de elementos y el array
	uint32_t buffer_size = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(char *);

	for (int i = 0; i < proceso->cantidad_elementos; i++)
	{
		// Reservo tamaño para el elemento que tiene el tamaño de la cadena
		buffer_size += sizeof(uint32_t);
		// Reservo tamaño para la cadena
		buffer_size += strlen(proceso->array[i]) + 1;
	}

	actualizar_buffer(*paquete, buffer_size);
	serializar_uint32_t(proceso->pid, *paquete);
	serializar_uint32_t(proceso->cantidad_elementos, *paquete);
	serializar_char_array(proceso->array, proceso->cantidad_elementos, *paquete);
}

void serializar_char_array(char **array, uint32_t cantidad_elementos, t_paquete *paquete)
{
	// Serializo la cantidad de elementos del array
	serializar_uint32_t(cantidad_elementos, paquete);

	// Serializo de cada elemento: el tamaño de la cadena, y luego la cadena
	for (int i = 0; i < cantidad_elementos; i++)
	{
		// Calculo el tamaño de la cadena
		uint32_t len = strlen(array[i]) + 1;

		// Serializo el tamaño de la cadena
		serializar_uint32_t(len, paquete);

		// Serializo la cadena
		serializar_char(array[i], paquete);
	}
}

void serializar_uint32_t_array(uint32_t *array, uint32_t cantidad_elementos, t_paquete *paquete)
{

	// Serializo la cantidad de elementos del array
	serializar_uint32_t(cantidad_elementos, paquete);

	// Serializo cada elemento del array
	for (int i = 0; i < cantidad_elementos; i++)
	{
		// Serializo el valor uint32_t en la posición actual del array
		serializar_uint32_t(array[i], paquete);
	}
}

void deserializar_uint32_t_array(uint32_t **array, uint32_t *cantidad_elementos, void **buffer)
{
	// Deserializo la cantidad de elementos del array
	deserializar_uint32_t(buffer, cantidad_elementos);

	// Reservo memoria para el array
	*array = malloc(*cantidad_elementos * sizeof(uint32_t));

	// Verifico que la memoria se haya asignado correctamente
	if (*array == NULL)
	{
		perror("Error al asignar memoria para el array de uint32_t");
	}

	// Deserializo cada elemento del array
	for (int i = 0; i < *cantidad_elementos; i++)
	{
		deserializar_uint32_t(buffer, &((*array)[i]));
	}
}

void deserializar_char_array(char ***array, uint32_t *cantidad_elementos, void **buffer)
{
	// Deserializo la cantidad de elementos dentro del array
	deserializar_uint32_t(buffer, cantidad_elementos);

	// Reservamos memoria para los elementos del array
	*array = malloc(*cantidad_elementos * sizeof(char *));

	// Deserializo de cada elemento: el tamaño de la cadena, y luego la cadena
	for (int i = 0; i < *cantidad_elementos; i++)
	{
		uint32_t len;

		// Deserializo el tamaño de la cadena
		deserializar_uint32_t(buffer, &len);

		// Deserializo la cadena
		deserializar_char(buffer, &((*array)[i]), len);
	}
}

void serializar_uint8_t_array(uint8_t *array, uint32_t cantidad_elementos, t_paquete *paquete)
{
	// Serializo la cantidad de elementos del array
	serializar_uint8_t(cantidad_elementos, paquete);

	// Serializo cada elemento del array
	for (int i = 0; i < cantidad_elementos; i++)
	{
		// Serializo el valor uint8_t en la posición actual del array
		serializar_uint8_t(array[i], paquete);
	}
}

void deserializar_uint8_t_array(uint8_t **array, uint8_t *cantidad_elementos, void **buffer)
{
	// Deserializo la cantidad de elementos del array
	deserializar_uint8_t(buffer, cantidad_elementos);

	// Reservo memoria para el array
	*array = malloc(*cantidad_elementos * sizeof(uint8_t));

	// Verifico que la memoria se haya asignado correctamente
	if (*array == NULL)
	{
		perror("Error al asignar memoria para el array de uint8_t");
	}

	// Deserializo cada elemento del array
	for (int i = 0; i < *cantidad_elementos; i++)
	{
		deserializar_uint8_t(buffer, &((*array)[i]));
	}
}

void serializar_uint16_t_array(uint16_t *array, uint32_t cantidad_elementos, t_paquete *paquete)
{
	// Serializo la cantidad de elementos del array
	serializar_uint16_t(cantidad_elementos, paquete);

	// Serializo cada elemento del array
	for (int i = 0; i < cantidad_elementos; i++)
	{
		// Serializo el valor uint16_t en la posición actual del array
		serializar_uint16_t(array[i], paquete);
	}
}

void deserializar_uint16_t_array(uint16_t **array, uint16_t *cantidad_elementos, void **buffer)
{
	// Deserializo la cantidad de elementos del array
	deserializar_uint16_t(buffer, cantidad_elementos);

	// Reservo memoria para el array
	*array = malloc(*cantidad_elementos * sizeof(uint16_t));

	// Verifico que la memoria se haya asignado correctamente
	if (*array == NULL)
	{
		perror("Error al asignar memoria para el array de uint16_t");
	}

	// Deserializo cada elemento del array
	for (int i = 0; i < *cantidad_elementos; i++)
	{
		deserializar_uint16_t(buffer, &((*array)[i]));
	}
}

void serializar_uint64_t_array(uint64_t *array, uint32_t cantidad_elementos, t_paquete *paquete)
{
	// Serializo la cantidad de elementos del array
	serializar_uint64_t(cantidad_elementos, paquete);

	// Serializo cada elemento del array
	for (int i = 0; i < cantidad_elementos; i++)
	{
		// Serializo el valor uint64_t en la posición actual del array
		serializar_uint64_t(array[i], paquete);
	}
}

void deserializar_uint64_t_array(uint64_t **array, uint64_t *cantidad_elementos, void **buffer)
{
	// Deserializo la cantidad de elementos del array
	deserializar_uint64_t(buffer, cantidad_elementos);

	// Reservo memoria para el array
	*array = malloc(*cantidad_elementos * sizeof(uint64_t));

	// Verifico que la memoria se haya asignado correctamente
	if (*array == NULL)
	{
		perror("Error al asignar memoria para el array de uint64_t");
	}

	// Deserializo cada elemento del array
	for (int i = 0; i < *cantidad_elementos; i++)
	{
		deserializar_uint64_t(buffer, &((*array)[i]));
	}
}

void serializar_t_cpu_memoria_instruccion(t_paquete **paquete, t_cpu_memoria_instruccion *proceso)
{
	actualizar_buffer(*paquete, (2 * sizeof(uint32_t)));
	serializar_uint32_t(proceso->program_counter, *paquete);
	serializar_uint32_t(proceso->pid, *paquete);
}

t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer)
{
	t_kernel_cpu_proceso *proceso = malloc(sizeof(t_kernel_cpu_proceso));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(proceso->pid));
	deserializar_uint32_t(&stream, &(proceso->registros.pc));
	deserializar_uint32_t(&stream, &(proceso->registros.eax));
	deserializar_uint32_t(&stream, &(proceso->registros.ebx));
	deserializar_uint32_t(&stream, &(proceso->registros.ecx));
	deserializar_uint32_t(&stream, &(proceso->registros.edx));
	deserializar_uint32_t(&stream, &(proceso->registros.si));
	deserializar_uint32_t(&stream, &(proceso->registros.di));
	deserializar_uint8_t(&stream, &(proceso->registros.ax));
	deserializar_uint8_t(&stream, &(proceso->registros.bx));
	deserializar_uint8_t(&stream, &(proceso->registros.cx));
	deserializar_uint8_t(&stream, &(proceso->registros.dx));

	return proceso;
}