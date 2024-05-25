#include <utils/kernel.h>

void detener_planificacion(char **separar_linea, hilos_args *hiloArgs)
{
    int iniciar_algoritmo_valor;
    sem_getvalue(&hiloArgs->kernel->planificador_hilo, &iniciar_algoritmo_valor);
    if (iniciar_algoritmo_valor == -1)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "No se puede detener la planificacion si no está iniciada");
        return;
    }
    hilo_planificador_detener(hiloArgs);
    hilo_planificador_estado(hiloArgs, false);
    kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s", separar_linea[0]);
    return;
}