#include <utils/memoria.h>

// Función para escribir datos con verificación del bitmap y la dirección específica
int espacio_usuario_escribir_dato(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, void *dato, size_t tamano)
{
    // Verifico que la escritura no supere los limites de la memoria
    if (direccion_fisica + tamano > args->memoria.tamMemoria)
    {
        log_error(args->logger, "Se intento escribir en la direccion fisica %d, pero el limite del espacio de usuario es %d.", direccion_fisica, args->memoria.tamMemoria);
        return -1;
    }

    // Determino los frames de inicio y fin
    uint32_t frame_inicio = espacio_usuario_obtener_frame(direccion_fisica, args->memoria.tamPagina);
    uint32_t frame_fin = espacio_usuario_obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);

    // Verifica que todos los frames pertenecen al proceso
    for (uint32_t frame = frame_inicio; frame <= frame_fin; frame++) {
        bool frame_pertenece = false;

        // Recorre todas las páginas del proceso para verificar si el frame está asignado
        for (int i = 0; i < list_size(proceso->tabla_paginas); i++) {
            t_pagina *pagina = list_get(proceso->tabla_paginas, i);
            int marco = -1;

            if (pagina != NULL && pagina->validez == 1) {
                marco = pagina->marco;
            }

            if (marco == frame) {
                frame_pertenece = true;
                break;
            }
        }

        if (!frame_pertenece) {
            log_error(args->logger, "El frame <%d> no pertenece al proceso PID <%d>. Escritura abortada.", frame, proceso->pid);
            return -1; 
        }
    }

    // Verificar si ya hay datos en la dirección física
    uint8_t *destino = (uint8_t *)args->memoria.espacio_usuario + direccion_fisica;

    // Alarma por si se sobreescriben datos
    for (size_t i = 0; i < tamano; i++)
    {
        if (destino[i] != '\n')
        {
            log_warning(args->logger, "Sobrescribiendo datos en la dirección física %ld.", direccion_fisica + i);
        }
    }

    // Marco todos los frames del intervalo de inicio a fin como ocupados y actualizo el array de bytes usados en cada frame
    for (uint32_t frame = frame_inicio; frame <= frame_fin; frame++)
    {
        args->memoria.bitmap_array[frame] = 1;

        // Calculo los bytes a actualizar en este frame
        uint32_t offset_inicio, offset_fin;

        if (frame == frame_inicio)
        {
            offset_inicio = direccion_fisica % args->memoria.tamPagina;
        }
        else
        {
            offset_inicio = 0;
        }

        if (frame == frame_fin)
        {
            offset_fin = (direccion_fisica + tamano - 1) % args->memoria.tamPagina;
        }
        else
        {
            offset_fin = args->memoria.tamPagina - 1;
        }

        args->memoria.bytes_usados[frame] += (offset_fin - offset_inicio + 1);
    }

    // Escribo los datos
    memcpy(destino, dato, tamano);

    // Actualizo los bytes usados en el proceso
    proceso->bytes_usados += tamano;

    // Notifico que se escribio el dato
    log_debug(args->logger, "Se escribio el dato de %ld bytes partiendo de la dirección física %d (%d -> %ld) [Frame %d -> %d]", tamano, direccion_fisica, direccion_fisica, direccion_fisica + tamano - 1, frame_inicio, frame_fin);

    log_info(args->logger, "PID: %d - Accion: ESCRIBIR - Direccion fisica: <%d> - Tamaño <%ld>", proceso->pid, direccion_fisica, tamano);

    return 0;
}

// Retorna el frame desde el cual se iniciaria a escribir un dato de tamaño especificado desde una dirección física
uint32_t espacio_usuario_escribir_dato_frame_inicio(t_args *args, uint32_t direccion_fisica, size_t tamano)
{
    return espacio_usuario_obtener_frame(direccion_fisica, args->memoria.tamPagina);
}

// Retorna el frame desde el cual se finalizaria la escritura de un dato de tamaño especificado desde una dirección física
uint32_t espacio_usuario_escribir_dato_frame_fin(t_args *args, uint32_t direccion_fisica, size_t tamano)
{
    return espacio_usuario_obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);
}

// Escribir un uint32_t
void espacio_usuario_escribir_uint32_t(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, uint32_t valor)
{
    espacio_usuario_escribir_dato(args, proceso, direccion_fisica, &valor, sizeof(uint32_t));
}

// Escribir un uint8_t
void espacio_usuario_escribir_uint8_t(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, uint8_t valor)
{
    espacio_usuario_escribir_dato(args, proceso, direccion_fisica, &valor, sizeof(uint8_t));
}

// Escribir una cadena
int espacio_usuario_escribir_char(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, const char *cadena)
{
    return espacio_usuario_escribir_dato(args, proceso, direccion_fisica, (void *)cadena, strlen(cadena));
}
