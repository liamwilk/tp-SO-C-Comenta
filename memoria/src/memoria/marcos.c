#include <utils/memoria.h>

void memoria_inicializar_marcos(t_args *args) {

    // Pido memoria para los marcos
    args->memoria.marcos = malloc(args->memoria.tamMemoria);

    // Reviso que se haya podido asignar memoria
    if (args->memoria.marcos == NULL) {
        log_error(args->logger, "Malloc falló al asignar %d bytes para los marcos de memoria.", args->memoria.tamMemoria);
        exit(1);
    }

    log_debug(args->logger, "Memoria asignada correctamente %d bytes para los marcos de memoria.", args->memoria.tamMemoria);

    // Inicializo toda la memoria en 0
    memset(args->memoria.marcos, 0, args->memoria.tamMemoria);
}
