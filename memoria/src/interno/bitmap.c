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

        char *cadena = "CURSADA DE SISTEMAS OPERATIVOS 1c 2024";

        t_char_framentado *cadenas = espacio_usuario_fragmentar_char(cadena, args->memoria.tamPagina);

        log_debug(args->logger, "Escribiendo en espacio de usuario: %s%s%s (%d bytes)", cadenas->fragmentos[0], cadenas->fragmentos[1], cadenas->fragmentos[2], cadenas->tamanos[0] + cadenas->tamanos[1] + cadenas->tamanos[2]);

        log_debug(args->logger, "Particiono la cadena en %d partes, porque el tamaño de la cadena es mayor al tamaño de un frame.", cadenas->cantidad);

        espacio_usuario_fragmentos_imprimir(args, cadenas);

        t_frame_disponible *frame_cadena_1, *frame_cadena_2, *frame_cadena_3;

        frame_cadena_1 = espacio_usuario_buscar_frame(args, cadenas->tamanos[0]);

        if (frame_cadena_1 != NULL)
        {
            espacio_usuario_escribir_char(args, frame_cadena_1->direccion_fisica, cadenas->fragmentos[0]);

            frame_cadena_2 = espacio_usuario_buscar_frame(args, cadenas->tamanos[1]);

            if (frame_cadena_2 != NULL)
            {
                espacio_usuario_escribir_char(args, frame_cadena_2->direccion_fisica, cadenas->fragmentos[1]);

                frame_cadena_3 = espacio_usuario_buscar_frame(args, cadenas->tamanos[2]);

                if (frame_cadena_3 != NULL)
                {
                    espacio_usuario_escribir_char(args, frame_cadena_3->direccion_fisica, cadenas->fragmentos[2]);

                    char *cadena_leida_1 = espacio_usuario_leer_char(args, frame_cadena_1->direccion_fisica, cadenas->tamanos[0]);
                    char *cadena_leida_2 = espacio_usuario_leer_char(args, frame_cadena_2->direccion_fisica, cadenas->tamanos[1]);
                    char *cadena_leida_3 = espacio_usuario_leer_char(args, frame_cadena_3->direccion_fisica, cadenas->tamanos[2]);

                    log_debug(args->logger, "Cadena leida de espacio de usuario: %s%s%s", cadena_leida_1, cadena_leida_2, cadena_leida_3);

                    log_debug(args->logger, "Libero recursos del test");

                    espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, cadenas->tamanos[0]);
                    espacio_usuario_liberar_dato(args, frame_cadena_2->direccion_fisica, cadenas->tamanos[1]);
                    espacio_usuario_liberar_dato(args, frame_cadena_3->direccion_fisica, cadenas->tamanos[2]);

                    free(frame_cadena_1);
                    free(frame_cadena_2);
                    free(frame_cadena_3);
                }
                else
                {
                    log_error(args->logger, "No se pudo encontrar un frame disponible para la cadena 3.");
                    espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, cadenas->tamanos[0]);
                    espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, cadenas->tamanos[1]);

                    free(frame_cadena_1);
                    free(frame_cadena_2);
                    free(frame_cadena_3);
                }
            }
            else
            {
                log_error(args->logger, "No se pudo encontrar un frame disponible para la cadena 2.");
                espacio_usuario_liberar_dato(args, frame_cadena_1->direccion_fisica, cadenas->tamanos[0]);
                free(frame_cadena_1);
                free(frame_cadena_2);
            }
        }
        else
        {
            log_error(args->logger, "No se pudo encontrar un frame disponible para la cadena 1. Detengo el test.");
            free(frame_cadena_1);
        }

        espacio_usuario_fragmentos_liberar(args, cadenas);
    }
}

// Libero el bitmap
void bitmap_liberar(t_args *args)
{
    bitarray_destroy(args->memoria.bitmap);
    log_debug(args->logger, "Se libero el bitmap del espacio de usuario.");
}

// Marca un frame como usado en el bitmap
void bitmap_marcar_ocupado(t_args *args, uint32_t frame)
{
    bitarray_set_bit(args->memoria.bitmap, frame);
}

// Marca un frame como libre en el bitmap
void bitmap_marcar_libre(t_args *args, uint32_t frame)
{
    bitarray_clean_bit(args->memoria.bitmap, frame);
}

// Verifica si un frame está libre en el bitmap
bool bitmap_frame_libre(t_bitarray *bitmap, uint32_t frame)
{
    return !bitarray_test_bit(bitmap, frame);
}
