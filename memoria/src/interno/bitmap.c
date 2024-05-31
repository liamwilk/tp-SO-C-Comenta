#include <utils/memoria.h>

// Inicializo el bitmap
void bitmap_inicializar(t_args *args)
{
    // Calcular el tamaño del bitmap
    size_t tamano_bitmap = (args->memoria.tamMemoria / args->memoria.tamPagina + 7) / 8;

    // Asignar memoria para el bitmap
    args->memoria.bitmap_data = malloc(tamano_bitmap);

    // Reviso que se haya podido asignar memoria para el bitmap
    if (args->memoria.bitmap_data == NULL)
    {
        log_error(args->logger, "Malloc falló al asignar %zu bytes para el bitmap.", tamano_bitmap);
        espacio_usuario_liberar(args);
        exit(1);
    }

    // Inicializar el bitmap en 0
    memset(args->memoria.bitmap_data, 0, tamano_bitmap);

    // Crear el bitmap
    args->memoria.bitmap = bitarray_create_with_mode(args->memoria.bitmap_data, tamano_bitmap, LSB_FIRST);

    // Reviso que se haya podido crear el bitmap
    if (args->memoria.bitmap == NULL)
    {
        log_error(args->logger, "No se pudo crear el bitmap para el espacio de usuario.");
        free(args->memoria.bitmap_data);
        espacio_usuario_liberar(args);
        exit(1);
    }

    log_debug(args->logger, "Se creo el bitmap para el espacio de usuario.");

    // TODO: Quitar estos casos de prueba.

    char *cadena = "CURSADA DE SISTEMAS OPERATIVOS 1c 2024";
    log_debug(args->logger, "Tamaño de la cadena: %zu", strlen(cadena) + 1);
    espacio_usuario_escribir_char(args, 0, cadena);

    char cadena_leida1[strlen(cadena) + 1];
    espacio_usuario_leer_char(args, 0, cadena_leida1, strlen(cadena) + 1);
    log_debug(args->logger,"Cadena leída: %s", cadena_leida1);
    
    // espacio_usuario_liberar_dato(args, 0, strlen(cadena) + 1);

    // log_debug(args->logger,"Tamaño de la cadena: %zu", strlen(cadena)+1);
    // espacio_usuario_escribir_char(args, 39, cadena);

    // char cadena_leida2[strlen(cadena) + 1];
    // espacio_usuario_leer_char(args, 39, cadena_leida2, strlen(cadena) + 1);
    // log_debug(args->logger,"Cadena leída: %s", cadena_leida2);

    // espacio_usuario_liberar_dato(args, 39, strlen(cadena) + 1);

    t_frame_disponible *frame = espacio_usuario_buscar_frame(args, 122);

    if (frame != NULL)
    {
        // Aca tendría que hacer algo con el frame
    }

    log_debug(args->logger,"Tamaño de la cadena: %zu", strlen(cadena)+1);
    espacio_usuario_escribir_char(args, 39, cadena);

    char cadena_leida2[strlen(cadena) + 1];
    espacio_usuario_leer_char(args, 39, cadena_leida2, strlen(cadena) + 1);
    log_debug(args->logger,"Cadena leída: %s", cadena_leida2);

    // espacio_usuario_liberar_dato(args, 39, strlen(cadena) + 1);

    frame = espacio_usuario_buscar_frame(args, 2);

}

// Libero el bitmap
void bitmap_liberar(t_args *args)
{
    bitarray_destroy(args->memoria.bitmap);
    log_debug(args->logger, "Se libero el bitmap del espacio de usuario.");
}

// Marca un frame como usado en el bitmap
void bitmap_marcar_ocupado(t_args *args, t_bitarray *bitmap, uint32_t frame)
{
    if (!bitarray_test_bit(bitmap, frame))
    {
        bitarray_set_bit(bitmap, frame);
        log_debug(args->logger, "Se marcó el frame %d como ocupado en el bitmap.", frame);
    }
}

// Marca un frame como libre en el bitmap
void bitmap_marcar_libre(t_args *args, t_bitarray *bitmap, uint32_t frame)
{
    bitarray_clean_bit(bitmap, frame);
    log_debug(args->logger, "Se marcó el frame %d como libre en el bitmap.", frame);
}

// Verifica si un frame está libre en el bitmap
bool bitmap_frame_libre(t_bitarray *bitmap, uint32_t frame)
{
    return !bitarray_test_bit(bitmap, frame);
}
