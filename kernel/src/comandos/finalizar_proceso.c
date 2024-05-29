#include <utils/kernel.h>

void finalizar_proceso(char **separar_linea, hilos_args *hiloArgs)
{
    if (separar_linea[1] == 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Falta proporcionar un numero de PID para eliminar. Vuelva a intentar.");
        return;
    }
    kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);

    int pid = atoi(separar_linea[1]);

    // Este booleano si es el momento de borrar el proceso tambien en Memoria
    bool debeBorrarMemoria = kernel_finalizar_proceso(hiloArgs, pid, INTERRUPTED_BY_USER);

    if (debeBorrarMemoria)
    {
        kernel_avisar_memoria_finalizacion_proceso(hiloArgs, pid);
    }
    return;
}