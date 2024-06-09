#include <utils/kernel/main.h>

void multiprogramacion(char **separar_linea, hilos_args *hiloArgs)
{
    if (separar_linea[1] == 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "No se proporciono un numero para el grado de multiprogramacion. Vuelva a intentar");
        return;
    }
    else
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
        int grado_multiprogramacion = atoi(separar_linea[1]);
        hiloArgs->kernel->gradoMultiprogramacion = grado_multiprogramacion;
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se cambio el grado de multiprogramacion a %d", grado_multiprogramacion);
        return;
    }
}