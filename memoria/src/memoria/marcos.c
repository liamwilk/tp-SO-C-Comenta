#include <utils/memoria.h>

void memoria_inicializar_marcos(t_args *args)
{

    // Pido memoria para los marcos
    args->memoria.marcos = malloc(args->memoria.tamMemoria);

    // Reviso que se haya podido asignar memoria
    if (args->memoria.marcos == NULL) {
        log_error(args->logger, "Malloc falló al asignar %d bytes para los marcos de memoria.", args->memoria.tamMemoria);
        exit(1);
    }

    log_debug(args->logger, "Memoria asignada correctamente %d bytes para los marcos de memoria.", args->memoria.tamMemoria);

    memoria_inicializar_bitmap(args);
    memoria_imprimir_bitmap(args);

    // Inicializo toda la memoria en 0
    memset(args->memoria.marcos, 0, args->memoria.tamMemoria);
}

void memoria_liberar_marcos(t_args *args)
{
    free(args->memoria.marcos);
    log_debug(args->logger, "Se liberaron %d bytes reservados para el espacio contiguo de memoria", args->memoria.tamMemoria);
}

void memoria_liberar_bitmap(t_args *args)
{
    list_destroy_and_destroy_elements(args->memoria.bitmap, free);
    log_debug(args->logger, "Se liberaron los elementos del bitmap");
}

void memoria_inicializar_bitmap(t_args *args)
{
    args->memoria.bitmap = list_create();

    for (int i = 0; i < args->memoria.tamMemoria / args->memoria.tamPagina; i++) {
        int *bit = malloc(sizeof(int));
        *bit = 0;
        list_add(args->memoria.bitmap, bit);
    }
}

void memoria_imprimir_bitmap(t_args *args)
{
    char *bitmap = string_new();
    string_append(&bitmap, "\n[ ");

    for (int i = 0; i < list_size(args->memoria.bitmap); i++) {
        string_append(&bitmap, string_itoa(*(int *)list_get(args->memoria.bitmap, i)));
        if (i < list_size(args->memoria.bitmap) - 1) {
            string_append(&bitmap, " | ");
        }
        // Imprimo el bitmap cada 10 valores
        if ((i + 1) % 20 == 0) {
            string_append(&bitmap, "\n  ");
        }
    }
    string_append(&bitmap, " ]");
    log_debug(args->logger, "[MEMORIA] Se imprime el Bitmap %s", bitmap);
    free(bitmap);
}