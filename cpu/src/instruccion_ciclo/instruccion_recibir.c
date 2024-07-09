#include <utils/cpu.h>

int instruccion_recibir(t_cpu *args, t_buffer *buffer)
{
    log_info(args->logger, "PID : <%d> - FETCH - Program Counter : <%d>", args->proceso.pid, args->proceso.registros.pc);

    // Libero la instruccion anterior
    if (args->cantidad_elementos > 0 && args->instruccion != NULL) {
        for (int i = 0; i < args->cantidad_elementos; i++) {
            free(args->instruccion[i]);
        }
        free(args->instruccion);
    }

    args->cantidad_elementos = 0;

    // Recibo la instruccion nueva
    t_memoria_cpu_instruccion *instruccion = deserializar_t_memoria_cpu_instruccion(buffer);

    args->tipo_instruccion = determinar_codigo_instruccion(instruccion->array[0]);

    if (args->tipo_instruccion == -1) {
        // Borro la instruccion recibida
        for (int i = 0; i < instruccion->cantidad_elementos; i++) {
            free(instruccion->array[i]);
        }
        free(instruccion->array);
        free(instruccion);
        return 1;
    }

    args->cantidad_elementos = instruccion->cantidad_elementos;
    args->instruccion = malloc(sizeof(char *) * instruccion->cantidad_elementos);

    // Copio la instruccion recibida en la estructura de la CPU
    for (int i = 0; i < instruccion->cantidad_elementos; i++) {
        args->instruccion[i] = malloc(strlen(instruccion->array[i]) + 1);
        strcpy(args->instruccion[i], instruccion->array[i]);
    }

    // Borro la instruccion recibida
    for (int i = 0; i < instruccion->cantidad_elementos; i++) {
        free(instruccion->array[i]);
    }
    free(instruccion->array);
    free(instruccion);

    args->proceso.registros.pc += 1;

    // TODO: Ver esto
    // args->flag_interrupt = 0;

    return 0;
}