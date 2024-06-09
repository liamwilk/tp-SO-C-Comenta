#include "serial.h"

t_kernel_memoria_proceso *deserializar_t_kernel_memoria(t_buffer *buffer)
{
	t_kernel_memoria_proceso *dato = malloc(sizeof(t_kernel_memoria_proceso));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(dato->size_path));
	deserializar_char(&stream, &(dato->path_instrucciones), dato->size_path);
	deserializar_uint32_t(&stream, &(dato->program_counter));
	deserializar_uint32_t(&stream, &(dato->pid));

	return dato;
}

t_cpu_memoria_instruccion *deserializar_t_cpu_memoria_instruccion(t_buffer *buffer)
{
	t_cpu_memoria_instruccion *dato = malloc(sizeof(t_cpu_memoria_instruccion));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(dato->program_counter));
	deserializar_uint32_t(&stream, &(dato->pid));
	return dato;
}

t_kernel_memoria_finalizar_proceso *deserializar_t_kernel_memoria_finalizar_proceso(t_buffer *buffer)
{
	t_kernel_memoria_finalizar_proceso *dato = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(dato->pid));

	return dato;
}

void serializar_t_memoria_kernel_proceso(t_paquete **paquete, t_memoria_kernel_proceso *proceso)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 2 + sizeof(bool));
	serializar_uint32_t(proceso->pid, *paquete);
	serializar_uint32_t(proceso->cantidad_instrucciones, *paquete);
	serializar_bool(proceso->leido, *paquete);
}

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

t_buffer *recibir_buffer(t_log *logger, int *socket_cliente)
{
	uint32_t size;
	uint32_t offset;
	ssize_t bytes_recibidos;

	bytes_recibidos = recv(*socket_cliente, &size, sizeof(uint32_t), MSG_WAITALL);

	if (bytes_recibidos == -1)
	{
		log_trace(logger, "El cliente se desconecto del socket %d mientras recibia el tamaño del stream.", *socket_cliente);
		*socket_cliente = -1;
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_trace(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el tamaño del stream.", *socket_cliente);
		*socket_cliente = -1;
		return NULL;
	}

	bytes_recibidos = recv(*socket_cliente, &offset, sizeof(uint32_t), MSG_WAITALL);
	if (bytes_recibidos == -1)
	{
		log_trace(logger, "El cliente se desconecto del socket %d mientras recibia el offset del stream.", *socket_cliente);
		*socket_cliente = -1;
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_trace(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el offset del stream.", *socket_cliente);
		*socket_cliente = -1;
		return NULL;
	}

	void *stream = malloc(size);

	bytes_recibidos = recv(*socket_cliente, stream, size, MSG_WAITALL);

	if (bytes_recibidos == -1)
	{
		free(stream);
		log_trace(logger, "El cliente se desconecto del socket %d mientras recibia el stream.", *socket_cliente);
		close(*socket_cliente);
		*socket_cliente = -1;
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		free(stream);
		log_trace(logger, "Conexion por socket %d cerrada en el otro extremo.", *socket_cliente);
		close(*socket_cliente);
		*socket_cliente = -1;
		return NULL;
	}

	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = size;
	buffer->offset = offset;
	buffer->stream = stream;

	return buffer;
}

t_paquete *recibir_paquete(t_log *logger, int *socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	ssize_t bytes_recibidos;

	bytes_recibidos = recv(*socket_cliente, &(paquete->codigo_operacion), sizeof(t_op_code), MSG_WAITALL);
	if (bytes_recibidos == -1)
	{
		log_trace(logger, "El cliente se desconecto del socket %d mientras recibia el codigo de operacion.", *socket_cliente);
		*socket_cliente = -1;
		free(paquete);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_trace(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el codigo de operacion.", *socket_cliente);
		*socket_cliente = -1;
		free(paquete);
		return NULL;
	}

	bytes_recibidos = recv(*socket_cliente, &(paquete->size_buffer), sizeof(uint32_t), MSG_WAITALL);
	if (bytes_recibidos == -1)
	{
		log_trace(logger, "El cliente se desconecto del socket %d mientras recibia el tamaño total del buffer.", *socket_cliente);
		*socket_cliente = -1;
		free(paquete);
		return NULL;
	}
	else if (bytes_recibidos == 0)
	{
		log_trace(logger, "Conexion por socket %d cerrada en el otro extremo mientras recibia el tamaño total del buffer.", *socket_cliente);
		*socket_cliente = -1;
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
		log_debug(logger, "Paquete recibido de modulo %s", modulo);
		// log_trace(logger, "Deserializado del paquete:");
		log_debug(logger, "Codigo de operacion: %d", paquete->codigo_operacion);
		// log_trace(logger, "Size del buffer en paquete: %d", paquete->size_buffer);
		// log_trace(logger, "Deserializado del buffer:");
		// log_trace(logger, "Size del stream: %d", paquete->buffer->size);
		// log_trace(logger, "Offset del stream: %d", paquete->buffer->offset);

		if (paquete->size_buffer != paquete->buffer->size + (2 * sizeof(uint32_t)))
		{
			log_error(logger, "Error en el tamaño del buffer. Se esperaba %d y se recibio %ld", paquete->size_buffer, paquete->buffer->size + (2 * sizeof(uint32_t)));
		}
	}
	else
	{
		log_debug(logger, "Kernel solicito el apagado del modulo.");
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
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(proceso->ejecutado));
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

t_kernel_entrada_salida_unidad_de_trabajo *deserializar_t_kernel_entrada_salida_unidad_de_trabajo(t_buffer *buffer)
{
	t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(unidad->pid));
	deserializar_uint32_t(&stream, &(unidad->unidad_de_trabajo));

	return unidad;
}

t_entrada_salida_kernel_unidad_de_trabajo *deserializar_t_entrada_salida_kernel_unidad_de_trabajo(t_buffer *buffer)
{
	t_entrada_salida_kernel_unidad_de_trabajo *unidad = malloc(sizeof(t_entrada_salida_kernel_unidad_de_trabajo));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(unidad->pid));
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
	serializar_uint32_t(proceso->registros.pc, *paquete);
	serializar_uint32_t(proceso->registros.eax, *paquete);
	serializar_uint32_t(proceso->registros.ebx, *paquete);
	serializar_uint32_t(proceso->registros.ecx, *paquete);
	serializar_uint32_t(proceso->registros.edx, *paquete);
	serializar_uint32_t(proceso->registros.si, *paquete);
	serializar_uint32_t(proceso->registros.di, *paquete);
	serializar_uint8_t(proceso->registros.ax, *paquete);
	serializar_uint8_t(proceso->registros.bx, *paquete);
	serializar_uint8_t(proceso->registros.cx, *paquete);
	serializar_uint8_t(proceso->registros.dx, *paquete);
}

void serializar_t_kernel_entrada_salida_unidad_de_trabajo(t_paquete **paquete, t_kernel_entrada_salida_unidad_de_trabajo *unidad)
{
	actualizar_buffer(*paquete, 2 * sizeof(uint32_t));
	serializar_uint32_t(unidad->pid, *paquete);
	serializar_uint32_t(unidad->unidad_de_trabajo, *paquete);
}

void serializar_t_cpu_kernel_io_gen_sleep(t_paquete **paquete, t_cpu_kernel_io_gen_sleep *unidad)
{

	/* typedef struct
	{
		uint32_t pid;
		uint32_t size_interfaz;
		char *interfaz;
		uint32_t tiempo;
	} t_cpu_kernel_io_gen_sleep; */

	actualizar_buffer(*paquete, unidad->size_interfaz + sizeof(uint32_t) * 3 + sizeof(uint32_t) * 7 + sizeof(uint8_t) * 4);
	serializar_uint32_t(unidad->pid, *paquete);
	serializar_uint32_t(unidad->size_interfaz, *paquete);
	serializar_char(unidad->interfaz, *paquete);
	serializar_uint32_t(unidad->tiempo, *paquete);
	serializar_uint32_t(unidad->registros.pc, *paquete);
	serializar_uint32_t(unidad->registros.eax, *paquete);
	serializar_uint32_t(unidad->registros.ebx, *paquete);
	serializar_uint32_t(unidad->registros.ecx, *paquete);
	serializar_uint32_t(unidad->registros.edx, *paquete);
	serializar_uint32_t(unidad->registros.si, *paquete);
	serializar_uint32_t(unidad->registros.di, *paquete);
	serializar_uint8_t(unidad->registros.ax, *paquete);
	serializar_uint8_t(unidad->registros.bx, *paquete);
	serializar_uint8_t(unidad->registros.cx, *paquete);
	serializar_uint8_t(unidad->registros.dx, *paquete);
}

t_cpu_kernel_io_gen_sleep *deserializar_t_cpu_kernel_io_gen_sleep(t_buffer *buffer)
{
	t_cpu_kernel_io_gen_sleep *dato = malloc(sizeof(t_cpu_kernel_io_gen_sleep));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(dato->pid));
	deserializar_uint32_t(&stream, &(dato->size_interfaz));
	deserializar_char(&stream, &(dato->interfaz), dato->size_interfaz);
	deserializar_uint32_t(&stream, &(dato->tiempo));
	deserializar_uint32_t(&stream, &(dato->registros.pc));
	deserializar_uint32_t(&stream, &(dato->registros.eax));
	deserializar_uint32_t(&stream, &(dato->registros.ebx));
	deserializar_uint32_t(&stream, &(dato->registros.ecx));
	deserializar_uint32_t(&stream, &(dato->registros.edx));
	deserializar_uint32_t(&stream, &(dato->registros.si));
	deserializar_uint32_t(&stream, &(dato->registros.di));
	deserializar_uint8_t(&stream, &(dato->registros.ax));
	deserializar_uint8_t(&stream, &(dato->registros.bx));
	deserializar_uint8_t(&stream, &(dato->registros.cx));
	deserializar_uint8_t(&stream, &(dato->registros.dx));

	return dato;
}

void serializar_t_entrada_salida_kernel_unidad_de_trabajo(t_paquete **paquete, t_entrada_salida_kernel_unidad_de_trabajo *unidad)
{
	actualizar_buffer(*paquete, sizeof(bool) + sizeof(uint32_t));
	serializar_uint32_t(unidad->pid, *paquete);
	serializar_bool(unidad->terminado, *paquete);
}

void serializar_t_entrada_salida_kernel_finalizar(t_paquete **paquete, t_entrada_salida_kernel_finalizar *unidad)
{
	actualizar_buffer(*paquete, sizeof(bool));
	serializar_bool(unidad->terminado, *paquete);
}

void serializar_t_cpu_memoria_resize(t_paquete **paquete, t_cpu_memoria_resize *resize)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 2);
	serializar_uint32_t(resize->pid, *paquete);
	serializar_uint32_t(resize->bytes, *paquete);
}

t_cpu_memoria_resize *deserializar_t_cpu_memoria_resize(t_buffer *buffer)
{
	t_cpu_memoria_resize *dato = malloc(sizeof(t_cpu_memoria_resize));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(dato->pid));
	deserializar_uint32_t(&stream, &(dato->bytes));

	return dato;
}

void serializar_t_memoria_cpu_resize(t_paquete **paquete, t_memoria_cpu_resize *resize)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 4 + resize->size_motivo);
	serializar_uint32_t(resize->pid, *paquete);
	serializar_uint32_t(resize->bytes, *paquete);
	serializar_uint32_t(resize->resultado, *paquete);
	serializar_uint32_t(resize->size_motivo, *paquete);
	serializar_char(resize->motivo, *paquete);
}

t_memoria_cpu_resize *deserializar_t_memoria_cpu_resize(t_buffer *buffer)
{
	t_memoria_cpu_resize *dato = malloc(sizeof(t_memoria_cpu_resize));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(dato->pid));
	deserializar_uint32_t(&stream, &(dato->bytes));
	deserializar_uint32_t(&stream, &(dato->resultado));
	deserializar_uint32_t(&stream, &(dato->size_motivo));
	deserializar_char(&stream, &(dato->motivo), dato->size_motivo);

	return dato;
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

void serializar_t_kernel_cpu_interrupcion(t_paquete **paquete, t_kernel_cpu_interrupcion *interrupcion)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) + sizeof(uint32_t) + interrupcion->len_motivo);
	serializar_uint32_t(interrupcion->pid, *paquete);
	serializar_uint32_t(interrupcion->len_motivo, *paquete);
	serializar_char(interrupcion->motivo, *paquete);
}

void serializar_t_cpu_kernel_resize(t_paquete **paquete, t_cpu_kernel_resize *resize)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 11 + sizeof(uint8_t) * 4 + resize->size_motivo);
	serializar_uint32_t(resize->pid, *paquete);
	serializar_uint32_t(resize->resultado, *paquete);
	serializar_uint32_t(resize->size_motivo, *paquete);
	serializar_char(resize->motivo, *paquete);
	serializar_uint32_t(resize->pid, *paquete);
	serializar_uint32_t(resize->registros.pc, *paquete);
	serializar_uint32_t(resize->registros.eax, *paquete);
	serializar_uint32_t(resize->registros.ebx, *paquete);
	serializar_uint32_t(resize->registros.ecx, *paquete);
	serializar_uint32_t(resize->registros.edx, *paquete);
	serializar_uint32_t(resize->registros.si, *paquete);
	serializar_uint32_t(resize->registros.di, *paquete);
	serializar_uint8_t(resize->registros.ax, *paquete);
	serializar_uint8_t(resize->registros.bx, *paquete);
	serializar_uint8_t(resize->registros.cx, *paquete);
	serializar_uint8_t(resize->registros.dx, *paquete);
}

t_cpu_kernel_resize *deserializar_t_cpu_kernel_resize(t_buffer *buffer)
{
	t_cpu_kernel_resize *resize = malloc(sizeof(t_cpu_kernel_resize));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(resize->pid));
	deserializar_uint32_t(&stream, &(resize->resultado));
	deserializar_uint32_t(&stream, &(resize->size_motivo));
	deserializar_char(&stream, &(resize->motivo), resize->size_motivo);
	deserializar_uint32_t(&stream, &(resize->pid));
	deserializar_uint32_t(&stream, &(resize->registros.pc));
	deserializar_uint32_t(&stream, &(resize->registros.eax));
	deserializar_uint32_t(&stream, &(resize->registros.ebx));
	deserializar_uint32_t(&stream, &(resize->registros.ecx));
	deserializar_uint32_t(&stream, &(resize->registros.edx));
	deserializar_uint32_t(&stream, &(resize->registros.si));
	deserializar_uint32_t(&stream, &(resize->registros.di));
	deserializar_uint8_t(&stream, &(resize->registros.ax));
	deserializar_uint8_t(&stream, &(resize->registros.bx));
	deserializar_uint8_t(&stream, &(resize->registros.cx));
	deserializar_uint8_t(&stream, &(resize->registros.dx));

	return resize;
}

void serializar_t_entrada_salida_kernel_io_stdout_write(t_paquete **paquete, t_entrada_salida_kernel_io_stdout_write *write)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 2);
	serializar_uint32_t(write->pid, *paquete);
	serializar_uint32_t(write->resultado, *paquete);
}

t_entrada_salida_kernel_io_stdout_write *deserializar_t_entrada_salida_kernel_io_stdout_write(t_buffer *buffer)
{
	t_entrada_salida_kernel_io_stdout_write *write = malloc(sizeof(t_entrada_salida_kernel_io_stdout_write));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(write->pid));
	deserializar_uint32_t(&stream, &(write->resultado));
	return write;
}

void serializar_t_kernel_cpu_io_stdout_write(t_paquete **paquete, t_kernel_cpu_io_stdout_write *write)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 4 + write->size_motivo);
	serializar_uint32_t(write->pid, *paquete);
	serializar_uint32_t(write->resultado, *paquete);
	serializar_uint32_t(write->size_motivo, *paquete);
	serializar_char(write->motivo, *paquete);
}

t_kernel_cpu_io_stdout_write *deserializar_t_kernel_cpu_io_stdout_write(t_buffer *buffer)
{
	t_kernel_cpu_io_stdout_write *write = malloc(sizeof(t_kernel_cpu_io_stdout_write));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(write->pid));
	deserializar_uint32_t(&stream, &(write->resultado));
	deserializar_uint32_t(&stream, &(write->size_motivo));
	deserializar_char(&stream, &(write->motivo), write->size_motivo);
	return write;
}

void serializar_t_memoria_io_stdout(t_paquete **paquete, t_memoria_io_stdout *write)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 4 + write->size_dato);

	serializar_uint32_t(write->pid, *paquete);
	serializar_uint32_t(write->direccion_fisica, *paquete);
	serializar_uint32_t(write->tamanio, *paquete);
	serializar_uint32_t(write->size_dato, *paquete);
	serializar_char(write->dato, *paquete);
}

t_memoria_io_stdout *deserializar_t_memoria_io_stdout(t_buffer *buffer)
{
	t_memoria_io_stdout *write = malloc(sizeof(t_memoria_io_stdout));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(write->pid));
	deserializar_uint32_t(&stream, &(write->direccion_fisica));
	deserializar_uint32_t(&stream, &(write->tamanio));
	deserializar_uint32_t(&stream, &(write->size_dato));
	deserializar_char(&stream, &(write->dato), write->size_dato);
	return write;
}

void serializar_t_io_memoria_stdout(t_paquete **paquete, t_io_memoria_stdout *write)
{

	// typedef struct
	// {
	// 	uint32_t pid;
	// 	uint32_t direccion_fisica;
	// 	uint32_t tamanio;
	// } t_io_memoria_stdout;

	actualizar_buffer(*paquete, sizeof(uint32_t) * 3);
	serializar_uint32_t(write->pid, *paquete);
	serializar_uint32_t(write->direccion_fisica, *paquete);
	serializar_uint32_t(write->tamanio, *paquete);
}

t_io_memoria_stdout *deserializar_t_io_memoria_stdout(t_buffer *buffer)
{
	t_io_memoria_stdout *write = malloc(sizeof(t_io_memoria_stdout));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(write->pid));
	deserializar_uint32_t(&stream, &(write->direccion_fisica));
	deserializar_uint32_t(&stream, &(write->tamanio));
	return write;
}

void serializar_t_io_stdout_write(t_paquete **paquete, t_io_stdout_write *write)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 16 + sizeof(uint8_t) * 4 + write->size_interfaz);
	serializar_uint32_t(write->pid, *paquete);
	serializar_uint32_t(write->resultado, *paquete);
	serializar_uint32_t(write->registro_direccion, *paquete);
	serializar_uint32_t(write->registro_tamanio, *paquete);
	serializar_uint32_t(write->marco, *paquete);
	serializar_uint32_t(write->numero_pagina, *paquete);
	serializar_uint32_t(write->direccion_fisica, *paquete);
	serializar_uint32_t(write->desplazamiento, *paquete);
	serializar_uint32_t(write->size_interfaz, *paquete);

	// Registros de CPU

	serializar_uint32_t(write->registros.pc, *paquete);
	serializar_uint32_t(write->registros.eax, *paquete);
	serializar_uint32_t(write->registros.ebx, *paquete);
	serializar_uint32_t(write->registros.ecx, *paquete);
	serializar_uint32_t(write->registros.edx, *paquete);
	serializar_uint32_t(write->registros.si, *paquete);
	serializar_uint32_t(write->registros.di, *paquete);
	serializar_uint8_t(write->registros.ax, *paquete);
	serializar_uint8_t(write->registros.bx, *paquete);
	serializar_uint8_t(write->registros.cx, *paquete);
	serializar_uint8_t(write->registros.dx, *paquete);

	serializar_char(write->interfaz, *paquete);
}

t_io_stdout_write *deserializar_t_io_stdout_write(t_buffer *buffer)
{
	t_io_stdout_write *write = malloc(sizeof(t_io_stdout_write));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(write->pid));
	deserializar_uint32_t(&stream, &(write->resultado));
	deserializar_uint32_t(&stream, &(write->registro_direccion));
	deserializar_uint32_t(&stream, &(write->registro_tamanio));
	deserializar_uint32_t(&stream, &(write->marco));
	deserializar_uint32_t(&stream, &(write->numero_pagina));
	deserializar_uint32_t(&stream, &(write->direccion_fisica));
	deserializar_uint32_t(&stream, &(write->desplazamiento));
	deserializar_uint32_t(&stream, &(write->size_interfaz));

	// Registros CPU
	deserializar_uint32_t(&stream, &(write->registros.pc));
	deserializar_uint32_t(&stream, &(write->registros.eax));
	deserializar_uint32_t(&stream, &(write->registros.ebx));
	deserializar_uint32_t(&stream, &(write->registros.ecx));
	deserializar_uint32_t(&stream, &(write->registros.edx));
	deserializar_uint32_t(&stream, &(write->registros.si));
	deserializar_uint32_t(&stream, &(write->registros.di));
	deserializar_uint8_t(&stream, &(write->registros.ax));
	deserializar_uint8_t(&stream, &(write->registros.bx));
	deserializar_uint8_t(&stream, &(write->registros.cx));
	deserializar_uint8_t(&stream, &(write->registros.dx));

	deserializar_char(&stream, &(write->interfaz), write->size_interfaz);
	return write;
}

void serializar_t_mov_out(t_paquete **paquete, t_mov_out *mov)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 9 + sizeof(uint8_t));
	serializar_uint32_t(mov->pid, *paquete);
	serializar_uint32_t(mov->tamanio_registro_datos, *paquete);
	serializar_uint32_t(mov->registro_direccion, *paquete);
	serializar_uint32_t(mov->registro_datos, *paquete);
	serializar_uint32_t(mov->numero_pagina, *paquete);
	serializar_uint32_t(mov->numero_marco, *paquete);
	serializar_uint32_t(mov->resultado, *paquete);
	serializar_uint32_t(mov->direccion_fisica, *paquete);
	serializar_uint32_t(mov->dato_32, *paquete);
	serializar_uint8_t(mov->dato_8, *paquete);
}

t_mov_out *deserializar_t_mov_out(t_buffer *buffer)
{
	t_mov_out *paquete = malloc(sizeof(t_mov_out));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(paquete->pid));
	deserializar_uint32_t(&stream, &(paquete->tamanio_registro_datos));
	deserializar_uint32_t(&stream, &(paquete->registro_direccion));
	deserializar_uint32_t(&stream, &(paquete->registro_datos));
	deserializar_uint32_t(&stream, &(paquete->numero_pagina));
	deserializar_uint32_t(&stream, &(paquete->numero_marco));
	deserializar_uint32_t(&stream, &(paquete->resultado));
	deserializar_uint32_t(&stream, &(paquete->direccion_fisica));
	deserializar_uint32_t(&stream, &(paquete->dato_32));
	deserializar_uint8_t(&stream, &(paquete->dato_8));
	return paquete;
}

void serializar_t_mov_in(t_paquete **paquete, t_mov_in *mov)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 9 + sizeof(uint8_t));
	serializar_uint32_t(mov->pid, *paquete);
	serializar_uint32_t(mov->tamanio_registro_datos, *paquete);
	serializar_uint32_t(mov->registro_direccion, *paquete);
	serializar_uint32_t(mov->registro_datos, *paquete);
	serializar_uint32_t(mov->numero_pagina, *paquete);
	serializar_uint32_t(mov->numero_marco, *paquete);
	serializar_uint32_t(mov->resultado, *paquete);
	serializar_uint32_t(mov->direccion_fisica, *paquete);
	serializar_uint32_t(mov->dato_32, *paquete);
	serializar_uint8_t(mov->dato_8, *paquete);
}

t_mov_in *deserializar_t_mov_in(t_buffer *buffer)
{
	t_mov_in *paquete = malloc(sizeof(t_mov_in));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(paquete->pid));
	deserializar_uint32_t(&stream, &(paquete->tamanio_registro_datos));
	deserializar_uint32_t(&stream, &(paquete->registro_direccion));
	deserializar_uint32_t(&stream, &(paquete->registro_datos));
	deserializar_uint32_t(&stream, &(paquete->numero_pagina));
	deserializar_uint32_t(&stream, &(paquete->numero_marco));
	deserializar_uint32_t(&stream, &(paquete->resultado));
	deserializar_uint32_t(&stream, &(paquete->direccion_fisica));
	deserializar_uint32_t(&stream, &(paquete->dato_32));
	deserializar_uint8_t(&stream, &(paquete->dato_8));
	return paquete;
}

void serializar_t_kernel_io_interrupcion(t_paquete **paquete, t_kernel_io_interrupcion *interrupcion)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) + sizeof(uint32_t) + interrupcion->len_motivo);
	serializar_uint32_t(interrupcion->pid, *paquete);
	serializar_uint32_t(interrupcion->len_motivo, *paquete);
	serializar_char(interrupcion->motivo, *paquete);
};

void serializar_t_entrada_salida_identificacion(t_paquete **paquete, t_entrada_salida_identificacion *identificacion)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) + identificacion->size_identificador);
	serializar_uint32_t(identificacion->size_identificador, *paquete);
	serializar_char(identificacion->identificador, *paquete);
}

t_entrada_salida_identificacion *deserializar_t_entrada_salida_identificacion(t_buffer *buffer)
{
	t_entrada_salida_identificacion *dato = malloc(sizeof(t_kernel_memoria_proceso));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(dato->size_identificador));
	deserializar_char(&stream, &(dato->identificador), dato->size_identificador);

	return dato;
}

t_kernel_cpu_interrupcion *deserializar_t_kernel_cpu_interrupcion(t_buffer *buffer)
{
	t_kernel_cpu_interrupcion *interrupcion = malloc(sizeof(t_kernel_cpu_interrupcion));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(interrupcion->pid));
	deserializar_uint32_t(&stream, &(interrupcion->len_motivo));
	deserializar_char(&stream, &(interrupcion->motivo), interrupcion->len_motivo);
	return interrupcion;
}

void remover_salto_linea(char *argumento_origen)
{
	if (argumento_origen[strlen(argumento_origen) - 1] == '\n')
	{
		argumento_origen[strlen(argumento_origen) - 1] = '\0';
	}
}

void serializar_t_memoria_cpu_tam_pagina(t_paquete **paquete, uint32_t tam_pagina)
{
	actualizar_buffer(*paquete, sizeof(uint32_t));
	serializar_uint32_t(tam_pagina, *paquete);
}

uint32_t *deserializar_t_memoria_cpu_tam_pagina(t_buffer *buffer)
{
	uint32_t *tam_pagina = malloc(sizeof(uint32_t));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, tam_pagina);

	return tam_pagina;
}

void serializar_t_memoria_cpu_numero_marco(t_paquete **paquete, uint32_t numero_marco)
{
	actualizar_buffer(*paquete, sizeof(uint32_t));
	serializar_uint32_t(numero_marco, *paquete);
}

uint32_t *deserializar_t_memoria_cpu_numero_marco(t_buffer *buffer)
{
	uint32_t *numero_marco = malloc(sizeof(uint32_t));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, numero_marco);

	return numero_marco;
}

void serializar_t_cpu_memoria_numero_marco(t_paquete **paquete, uint32_t pid, int numero_pagina)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) + sizeof(int));
	serializar_uint32_t(pid, *paquete);
	serializar_uint32_t(numero_pagina, *paquete);
}

t_cpu_memoria_numero_frame *deserializar_t_cpu_memoria_numero_frame(t_buffer *buffer)
{
	t_cpu_memoria_numero_frame *proceso = malloc(sizeof(t_cpu_memoria_numero_frame));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(proceso->pid));
	deserializar_uint32_t(&stream, &(proceso->numero_pagina));

	return proceso;
}

void serializar_t_cpu_kernel_solicitud_recurso(t_paquete **paquete, t_cpu_kernel_solicitud_recurso *contexto)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 9 + sizeof(uint8_t) * 4 + contexto->size_nombre_recurso);

	// PCB:
	serializar_uint32_t(contexto->pid, *paquete);
	serializar_uint32_t(contexto->registros->pc, *paquete);
	serializar_uint32_t(contexto->registros->eax, *paquete);
	serializar_uint32_t(contexto->registros->ebx, *paquete);
	serializar_uint32_t(contexto->registros->ecx, *paquete);
	serializar_uint32_t(contexto->registros->edx, *paquete);
	serializar_uint32_t(contexto->registros->si, *paquete);
	serializar_uint32_t(contexto->registros->di, *paquete);
	serializar_uint8_t(contexto->registros->ax, *paquete);
	serializar_uint8_t(contexto->registros->bx, *paquete);
	serializar_uint8_t(contexto->registros->cx, *paquete);
	serializar_uint8_t(contexto->registros->dx, *paquete);

	serializar_uint32_t(contexto->size_nombre_recurso, *paquete);
	serializar_char(contexto->nombre_recurso, *paquete);
}

t_cpu_kernel_solicitud_recurso *deserializar_t_cpu_kernel_solicitud_recurso(t_buffer *buffer)
{
	t_cpu_kernel_solicitud_recurso *ret = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
	ret->registros = malloc(sizeof(t_registros_cpu));

	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(ret->pid));
	deserializar_uint32_t(&stream, &(ret->registros->pc));
	deserializar_uint32_t(&stream, &(ret->registros->eax));
	deserializar_uint32_t(&stream, &(ret->registros->ebx));
	deserializar_uint32_t(&stream, &(ret->registros->ecx));
	deserializar_uint32_t(&stream, &(ret->registros->edx));
	deserializar_uint32_t(&stream, &(ret->registros->si));
	deserializar_uint32_t(&stream, &(ret->registros->di));
	deserializar_uint8_t(&stream, &(ret->registros->ax));
	deserializar_uint8_t(&stream, &(ret->registros->bx));
	deserializar_uint8_t(&stream, &(ret->registros->cx));
	deserializar_uint8_t(&stream, &(ret->registros->dx));

	deserializar_uint32_t(&stream, &(ret->size_nombre_recurso));
	deserializar_char(&stream, &(ret->nombre_recurso), ret->size_nombre_recurso);

	return ret;
}

t_kernel_io_interrupcion *deserializar_t_kernel_io_interrupcion(t_buffer *buffer)
{
	t_kernel_io_interrupcion *interrupcion = malloc(sizeof(t_kernel_io_interrupcion));
	void *stream = buffer->stream;
	deserializar_uint32_t(&stream, &(interrupcion->pid));
	deserializar_uint32_t(&stream, &(interrupcion->len_motivo));
	deserializar_char(&stream, &(interrupcion->motivo), interrupcion->len_motivo);
	return interrupcion;
}

void serializar_t_io_stdin_read(t_paquete **paquete, t_io_stdin_read *read)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 17 + sizeof(uint8_t) * 4 + read->size_interfaz);
	serializar_uint32_t(read->pid, *paquete);
	serializar_uint32_t(read->resultado, *paquete);
	serializar_uint32_t(read->registro_direccion, *paquete);
	serializar_uint32_t(read->registro_tamanio, *paquete);
	serializar_uint32_t(read->marco_inicial, *paquete);
	serializar_uint32_t(read->marco_final, *paquete);
	serializar_uint32_t(read->numero_pagina, *paquete);
	serializar_uint32_t(read->direccion_fisica, *paquete);
	serializar_uint32_t(read->desplazamiento, *paquete);
	serializar_uint32_t(read->size_interfaz, *paquete);

	// Registros de CPU

	serializar_uint32_t(read->registros.pc, *paquete);
	serializar_uint32_t(read->registros.eax, *paquete);
	serializar_uint32_t(read->registros.ebx, *paquete);
	serializar_uint32_t(read->registros.ecx, *paquete);
	serializar_uint32_t(read->registros.edx, *paquete);
	serializar_uint32_t(read->registros.si, *paquete);
	serializar_uint32_t(read->registros.di, *paquete);
	serializar_uint8_t(read->registros.ax, *paquete);
	serializar_uint8_t(read->registros.bx, *paquete);
	serializar_uint8_t(read->registros.cx, *paquete);
	serializar_uint8_t(read->registros.dx, *paquete);

	serializar_char(read->interfaz, *paquete);
}

t_io_stdin_read *deserializar_t_io_stdin_read(t_buffer *buffer)
{
	t_io_stdin_read *read = malloc(sizeof(t_io_stdin_read));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(read->pid));
	deserializar_uint32_t(&stream, &(read->resultado));
	deserializar_uint32_t(&stream, &(read->registro_direccion));
	deserializar_uint32_t(&stream, &(read->registro_tamanio));
	deserializar_uint32_t(&stream, &(read->marco_inicial));
	deserializar_uint32_t(&stream, &(read->marco_final));
	deserializar_uint32_t(&stream, &(read->numero_pagina));
	deserializar_uint32_t(&stream, &(read->direccion_fisica));
	deserializar_uint32_t(&stream, &(read->desplazamiento));
	deserializar_uint32_t(&stream, &(read->size_interfaz));

	// Registros CPU
	deserializar_uint32_t(&stream, &(read->registros.pc));
	deserializar_uint32_t(&stream, &(read->registros.eax));
	deserializar_uint32_t(&stream, &(read->registros.ebx));
	deserializar_uint32_t(&stream, &(read->registros.ecx));
	deserializar_uint32_t(&stream, &(read->registros.edx));
	deserializar_uint32_t(&stream, &(read->registros.si));
	deserializar_uint32_t(&stream, &(read->registros.di));
	deserializar_uint8_t(&stream, &(read->registros.ax));
	deserializar_uint8_t(&stream, &(read->registros.bx));
	deserializar_uint8_t(&stream, &(read->registros.cx));
	deserializar_uint8_t(&stream, &(read->registros.dx));

	deserializar_char(&stream, &(read->interfaz), read->size_interfaz);
	return read;
}

void serializar_t_kernel_cpu_io_stdin_read(t_paquete **paquete, t_kernel_cpu_io_stdin_read *read)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 4 + read->size_motivo);
	serializar_uint32_t(read->pid, *paquete);
	serializar_uint32_t(read->resultado, *paquete);
	serializar_uint32_t(read->size_motivo, *paquete);
	serializar_char(read->motivo, *paquete);
}

t_kernel_cpu_io_stdin_read *deserializar_t_kernel_cpu_io_stdin_read(t_buffer *buffer)
{
	t_kernel_cpu_io_stdin_read *read = malloc(sizeof(t_kernel_cpu_io_stdin_read));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(read->pid));
	deserializar_uint32_t(&stream, &(read->resultado));
	deserializar_uint32_t(&stream, &(read->size_motivo));
	deserializar_char(&stream, &(read->motivo), read->size_motivo);
	return read;
}

void serializar_t_kernel_io_stdin_read(t_paquete **paquete, t_kernel_io_stdin_read *read)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 17 + sizeof(uint8_t) * 4 + read->size_interfaz);
	serializar_uint32_t(read->pid, *paquete);
	serializar_uint32_t(read->resultado, *paquete);
	serializar_uint32_t(read->registro_direccion, *paquete);
	serializar_uint32_t(read->registro_tamanio, *paquete);
	serializar_uint32_t(read->marco_inicial, *paquete);
	serializar_uint32_t(read->marco_final, *paquete);
	serializar_uint32_t(read->numero_pagina, *paquete);
	serializar_uint32_t(read->direccion_fisica, *paquete);
	serializar_uint32_t(read->desplazamiento, *paquete);
	serializar_uint32_t(read->size_interfaz, *paquete);

	// Registros de CPU

	serializar_uint32_t(read->registros.pc, *paquete);
	serializar_uint32_t(read->registros.eax, *paquete);
	serializar_uint32_t(read->registros.ebx, *paquete);
	serializar_uint32_t(read->registros.ecx, *paquete);
	serializar_uint32_t(read->registros.edx, *paquete);
	serializar_uint32_t(read->registros.si, *paquete);
	serializar_uint32_t(read->registros.di, *paquete);
	serializar_uint8_t(read->registros.ax, *paquete);
	serializar_uint8_t(read->registros.bx, *paquete);
	serializar_uint8_t(read->registros.cx, *paquete);
	serializar_uint8_t(read->registros.dx, *paquete);

	serializar_char(read->interfaz, *paquete);
}

t_kernel_io_stdin_read *deserializar_t_kernel_io_stdin_read(t_buffer *buffer)
{
	t_kernel_io_stdin_read *read = malloc(sizeof(t_kernel_io_stdin_read));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(read->pid));
	deserializar_uint32_t(&stream, &(read->resultado));
	deserializar_uint32_t(&stream, &(read->registro_direccion));
	deserializar_uint32_t(&stream, &(read->registro_tamanio));
	deserializar_uint32_t(&stream, &(read->marco_inicial));
	deserializar_uint32_t(&stream, &(read->marco_final));
	deserializar_uint32_t(&stream, &(read->numero_pagina));
	deserializar_uint32_t(&stream, &(read->direccion_fisica));
	deserializar_uint32_t(&stream, &(read->desplazamiento));
	deserializar_uint32_t(&stream, &(read->size_interfaz));

	// Registros CPU
	deserializar_uint32_t(&stream, &(read->registros.pc));
	deserializar_uint32_t(&stream, &(read->registros.eax));
	deserializar_uint32_t(&stream, &(read->registros.ebx));
	deserializar_uint32_t(&stream, &(read->registros.ecx));
	deserializar_uint32_t(&stream, &(read->registros.edx));
	deserializar_uint32_t(&stream, &(read->registros.si));
	deserializar_uint32_t(&stream, &(read->registros.di));
	deserializar_uint8_t(&stream, &(read->registros.ax));
	deserializar_uint8_t(&stream, &(read->registros.bx));
	deserializar_uint8_t(&stream, &(read->registros.cx));
	deserializar_uint8_t(&stream, &(read->registros.dx));

	deserializar_char(&stream, &(read->interfaz), read->size_interfaz);
	return read;
}

void serializar_t_io_memoria_stdin(t_paquete **paquete, t_io_memoria_stdin *read)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 4 + read->size_input);
	serializar_uint32_t(read->pid, *paquete);
	serializar_uint32_t(read->direccion_fisica, *paquete);
	serializar_uint32_t(read->size_input, *paquete);
	serializar_char(read->input, *paquete);
}

t_io_memoria_stdin *deserializar_t_io_memoria_stdin(t_buffer *buffer)
{
	t_io_memoria_stdin *read = malloc(sizeof(t_io_memoria_stdin));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(read->pid));
	deserializar_uint32_t(&stream, &(read->direccion_fisica));
	deserializar_uint32_t(&stream, &(read->size_input));
	deserializar_char(&stream, &(read->input), read->size_input);
	return read;
}

void serializar_t_entrada_salida_kernel_io_stdin_read(t_paquete **paquete, t_entrada_salida_kernel_io_stdin_read *read)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 2);
	serializar_uint32_t(read->pid, *paquete);
	serializar_uint32_t(read->resultado, *paquete);
}

t_entrada_salida_kernel_io_stdin_read *deserializar_t_entrada_salida_kernel_io_stdin_read(t_buffer *buffer)
{
	t_entrada_salida_kernel_io_stdin_read *read = malloc(sizeof(t_entrada_salida_kernel_io_stdin_read));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(read->pid));
	deserializar_uint32_t(&stream, &(read->resultado));
	return read;
}

void serializar_t_memoria_entrada_salida_io_stdin_read(t_paquete **paquete, t_memoria_entrada_salida_io_stdin_read *read)
{
	actualizar_buffer(*paquete, sizeof(uint32_t) * 2);
	serializar_uint32_t(read->pid, *paquete);
	serializar_uint32_t(read->resultado, *paquete);
}

t_memoria_entrada_salida_io_stdin_read *deserializar_t_memoria_entrada_salida_io_stdin_read(t_buffer *buffer)
{
	t_memoria_entrada_salida_io_stdin_read *read = malloc(sizeof(t_memoria_entrada_salida_io_stdin_read));
	void *stream = buffer->stream;

	deserializar_uint32_t(&stream, &(read->pid));
	deserializar_uint32_t(&stream, &(read->resultado));
	return read;
}