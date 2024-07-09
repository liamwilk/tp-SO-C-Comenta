#include <utils/cpu.h>

int instruccion_recibir(t_cpu *args, t_buffer *buffer)
{
    log_info(args->logger, "PID : <%d> - FETCH - Program Counter : <%d>", args->proceso.pid, args->proceso.registros.pc);

    t_memoria_cpu_instruccion *dato = deserializar_t_memoria_cpu_instruccion(buffer);
    args->tipo_instruccion = determinar_codigo_instruccion(dato->array[0]);
    args->proceso.registros.pc += 1;
    // TODO: Ver esto
    //  args->flag_interrupt = 0;
    if (args->tipo_instruccion == -1)
    {
        return 1;
    }
    args->instruccion = *dato;
    free(dato);
    return 0;
}