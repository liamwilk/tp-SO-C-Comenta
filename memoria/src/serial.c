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

void serializar_t_memoria_cpu_instruccion(t_paquete** paquete_instruccion, t_memoria_cpu_instruccion *instruccion_proxima)
{
    // Actualizo el buffer del paquete
    actualizar_buffer(*paquete_instruccion, sizeof(uint32_t) * 7 + instruccion_proxima->size_instruccion + instruccion_proxima->size_argumento_1 + instruccion_proxima->size_argumento_2 + instruccion_proxima->size_argumento_3 + instruccion_proxima->size_argumento_4 + instruccion_proxima->size_argumento_5);

    // Serializo el t_memoria_cpu_instruccion
    serializar_uint32_t(instruccion_proxima->size_instruccion, *paquete_instruccion);
    serializar_char(instruccion_proxima->instruccion, *paquete_instruccion);
    serializar_uint32_t(instruccion_proxima->cantidad_argumentos, *paquete_instruccion);
    serializar_uint32_t(instruccion_proxima->size_argumento_1, *paquete_instruccion);
    serializar_char(instruccion_proxima->argumento_1, *paquete_instruccion);
    serializar_uint32_t(instruccion_proxima->size_argumento_2, *paquete_instruccion);
    serializar_char(instruccion_proxima->argumento_2, *paquete_instruccion);
    serializar_uint32_t(instruccion_proxima->size_argumento_3, *paquete_instruccion);
    serializar_char(instruccion_proxima->argumento_3, *paquete_instruccion);
    serializar_uint32_t(instruccion_proxima->size_argumento_4, *paquete_instruccion);
    serializar_char(instruccion_proxima->argumento_4, *paquete_instruccion);
    serializar_uint32_t(instruccion_proxima->size_argumento_5, *paquete_instruccion);
    serializar_char(instruccion_proxima->argumento_5, *paquete_instruccion);
}

void serializar_t_memoria_kernel_proceso(t_paquete** paquete, t_memoria_kernel_proceso *proceso)
{
    actualizar_buffer(*paquete, sizeof(uint32_t) * 2 + sizeof(bool));
    serializar_uint32_t(proceso->pid, *paquete);
    serializar_uint32_t(proceso->cantidad_instrucciones, *paquete);
    serializar_bool(proceso->leido, *paquete);
}