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

    size_t tamano_buscado = 9;
    uint32_t direccion_fisica_con_espacio = espacio_usuario_proxima_direccion(args, tamano_buscado);
    int marco_con_espacio = espacio_usuario_proximo_marco(args, tamano_buscado);

    if (direccion_fisica_con_espacio != -1)
    {
        log_debug(args->logger, "Encontrada la dirección física %u en el marco %d con suficiente espacio para %ld bytes", direccion_fisica_con_espacio,marco_con_espacio, tamano_buscado);
    }
    else
    {
        log_error(args->logger, "No se encontró una dirección física con suficiente espacio para %zu bytes.", tamano_buscado);
    }

    log_debug(args->logger,"Tamaño de la cadena: %zu", strlen(cadena)+1);
    espacio_usuario_escribir_char(args, 39, cadena);

    char cadena_leida2[strlen(cadena) + 1];
    espacio_usuario_leer_char(args, 39, cadena_leida2, strlen(cadena) + 1);
    log_debug(args->logger,"Cadena leída: %s", cadena_leida2);

    // espacio_usuario_liberar_dato(args, 39, strlen(cadena) + 1);

    tamano_buscado = 2;
    direccion_fisica_con_espacio = espacio_usuario_proxima_direccion(args, tamano_buscado);
    marco_con_espacio = espacio_usuario_proximo_marco(args, tamano_buscado);

    if (direccion_fisica_con_espacio != -1)
    {
        log_debug(args->logger, "Encontrada la dirección física %u en el marco %d con suficiente espacio para %ld bytes", direccion_fisica_con_espacio,marco_con_espacio, tamano_buscado);
    }
    else
    {
        log_error(args->logger, "No se encontró una dirección física con suficiente espacio para %zu bytes.", tamano_buscado);
    }

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
