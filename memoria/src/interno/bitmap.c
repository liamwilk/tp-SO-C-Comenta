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

    { // Test basico de funcionamiento

        /* TODO: Quitar este caso de prueba.

        Caso de prueba pedido en el TP.

        Escribir "CURSADA DE SISTEMAS OPERATIVOS 1 2024" en el espacio de usuario, y que ocupe 3 frames.

        Lo hardcodeo, pero esta logica se va a encargar el gestor de tabla de paginas de realizarla, es decir, de iterativamente cortar los strings hasta el tamaño maximo de la pagina (sacando el caracter nulo del final) y escribirlos en memoria.

        Memoria automaticamente cuando lo lee del espacio de usuario, te lo devuelve con el caracter nulo al final, es decir, ya formateado. */

        char *cadena_1 = "CURSADA DE SISTE";
        char *cadena_2 = "MAS OPERATIVOS 1";
        char *cadena_3 = "c 2024";

        size_t tamano_cadena_1 = strlen(cadena_1);
        size_t tamano_cadena_2 = strlen(cadena_2);
        size_t tamano_cadena_3 = strlen(cadena_3);

        log_debug(args->logger, "Escribiendo en espacio de usuario: %s%s%s (%ld bytes)", cadena_1, cadena_2, cadena_3, tamano_cadena_1 + tamano_cadena_2 + tamano_cadena_3);

        log_debug(args->logger, "Particiono la cadena en 3 partes, porque el tamaño de la cadena es mayor al tamaño de un frame.");

        log_debug(args->logger, "Cadena 1: %s (%ld bytes)", cadena_1, tamano_cadena_1);
        log_debug(args->logger, "Cadena 2: %s (%ld bytes)", cadena_2, tamano_cadena_2);
        log_debug(args->logger, "Cadena 3: %s (%ld bytes)", cadena_3, tamano_cadena_3);

        t_frame_disponible *frame_cadena_1, *frame_cadena_2, *frame_cadena_3;

        frame_cadena_1 = espacio_usuario_buscar_frame(args, tamano_cadena_1);

        if (frame_cadena_1 != NULL)
        {
            espacio_usuario_escribir_char(args, frame_cadena_1->direccion_fisica, cadena_1);

            frame_cadena_2 = espacio_usuario_buscar_frame(args, tamano_cadena_2);

            if (frame_cadena_2 != NULL)
            {
                espacio_usuario_escribir_char(args, frame_cadena_2->direccion_fisica, cadena_2);

                frame_cadena_3 = espacio_usuario_buscar_frame(args, tamano_cadena_3);

                if (frame_cadena_3 != NULL)
                {
                    espacio_usuario_escribir_char(args, frame_cadena_3->direccion_fisica, cadena_3);

                    char *cadena_leida_1 = espacio_usuario_leer_char(args, frame_cadena_1->direccion_fisica, tamano_cadena_1);
                    char *cadena_leida_2 = espacio_usuario_leer_char(args, frame_cadena_2->direccion_fisica, tamano_cadena_2);
                    char *cadena_leida_3 = espacio_usuario_leer_char(args, frame_cadena_3->direccion_fisica, tamano_cadena_3);

                    log_debug(args->logger, "Cadena leida de espacio de usuario: %s%s%s", cadena_leida_1, cadena_leida_2, cadena_leida_3);

                    // Libero los recursos de los tests

                    log_debug(args->logger, "Libero recursos del test");

                    espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, tamano_cadena_1);
                    espacio_usuario_liberar_dato(args, frame_cadena_2->direccion_fisica, tamano_cadena_2);
                    espacio_usuario_liberar_dato(args, frame_cadena_3->direccion_fisica, tamano_cadena_3);

                    free(cadena_leida_1);
                    free(cadena_leida_2);
                    free(cadena_leida_3);

                    free(frame_cadena_1);
                    free(frame_cadena_2);
                    free(frame_cadena_3);
                }
                else
                {
                    log_error(args->logger, "No se pudo encontrar un frame disponible para la cadena 3.");
                    espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, tamano_cadena_1);
                    espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, tamano_cadena_2);

                    free(frame_cadena_1);
                    free(frame_cadena_2);
                    free(frame_cadena_3);
                }
            }
            else
            {
                log_error(args->logger, "No se pudo encontrar un frame disponible para la cadena 2.");
                espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, tamano_cadena_1);
                free(frame_cadena_1);
                free(frame_cadena_2);
            }
        }
        else
        {
            log_error(args->logger, "No se pudo encontrar un frame disponible para la cadena 1. Detengo el test.");
            free(frame_cadena_1);
        }
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
        log_debug(args->logger, "Se marco el frame %d como ocupado en el bitmap.", frame);
    }
}

// Marca un frame como libre en el bitmap
void bitmap_marcar_libre(t_args *args, t_bitarray *bitmap, uint32_t frame)
{
    bitarray_clean_bit(bitmap, frame);
    log_debug(args->logger, "Se marco el frame %d como libre en el bitmap.", frame);
}

// Verifica si un frame está libre en el bitmap
bool bitmap_frame_libre(t_bitarray *bitmap, uint32_t frame)
{
    return !bitarray_test_bit(bitmap, frame);
}
