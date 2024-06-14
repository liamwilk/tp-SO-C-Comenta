#include <utils/kernel/main.h>

void iniciar_proceso(char **separar_linea, hilos_args *hiloArgs)
{
    if (separar_linea[1] == 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "No se proporciono un path. Vuelva a intentar");
        return;
    }
    else
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
        char *pathInstrucciones = separar_linea[1];
        kernel_nuevo_proceso(hiloArgs, hiloArgs->estados, hiloArgs->logger, pathInstrucciones);
        return;
    }
}