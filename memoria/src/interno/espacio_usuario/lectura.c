#include <utils/memoria.h>

// Leer un dato genérico
int espacio_usuario_leer_dato(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, void *destino, size_t tamano)
{
    // Reviso que la dirección física esté dentro de los límites del espacio de usuario
    if (direccion_fisica + tamano > args->memoria.tamMemoria)
    {
        log_error(args->logger, "Se intento leer fuera de los límites del espacio de usuario.");
        return -1;
    }

    // Obtengo la dirección de origen
    uint8_t *origen = (uint8_t *)args->memoria.espacio_usuario + direccion_fisica;

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
            log_error(args->logger, "El frame <%d> no pertenece al proceso PID <%d>. Lectura abortada.", frame, proceso->pid);
            return -1; 
        }
    }

    // Alerto si es que la lectura abarca más de un frame
    if (frame_inicio != frame_fin)
    {
        log_debug(args->logger, "La lectura de %ld bytes desde la direccion fisica %d (%d -> %ld) comienza en el frame %d hasta el %d.", tamano, direccion_fisica, direccion_fisica, tamano + direccion_fisica, frame_inicio, frame_fin);
    }

    // Copio los datos
    memcpy(destino, origen, tamano);

    log_info(args->logger, "PID: %d - Accion: LEER - Direccion fisica: <%d> - Tamaño <%ld>", proceso->pid, direccion_fisica, tamano);

    return 0;
}

// Leer un uint8_t
uint8_t espacio_usuario_leer_uint8(t_args *args, t_proceso *proceso, uint8_t direccion_fisica)
{
    uint8_t valor;
    espacio_usuario_leer_dato(args, proceso, direccion_fisica, &valor, sizeof(uint8_t));
    return valor;
}

// Leer un uint32_t
uint32_t espacio_usuario_leer_uint32(t_args *args, t_proceso *proceso, uint32_t direccion_fisica)
{
    uint32_t valor;
    espacio_usuario_leer_dato(args, proceso, direccion_fisica, &valor, sizeof(uint32_t));
    return valor;
}

// Leer un char*
char *espacio_usuario_leer_char(t_args *args, t_proceso *proceso, uint32_t direccion_fisica, size_t tamano_max)
{
    int bytes_totales = tamano_max + 1;
    char *destino = malloc(bytes_totales);
    int resultado = espacio_usuario_leer_dato(args, proceso, direccion_fisica, destino, tamano_max);

    if (resultado == -1)
    {
        free(destino);
        return NULL;
    }

    destino[bytes_totales - 1] = '\0';
    return destino;
}
