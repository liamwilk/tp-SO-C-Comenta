#include <utils/kernel/main.h>

void iniciar_planificacion(char **separar_linea, hilos_args *hiloArgs)
{
    kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s", separar_linea[0]);
    hilo_planificador_iniciar(hiloArgs);
    hilo_planificador_estado(hiloArgs, true);
    sem_post(&hiloArgs->kernel->planificador_iniciar);
}