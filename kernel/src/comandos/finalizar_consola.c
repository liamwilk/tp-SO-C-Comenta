#include <utils/kernel.h>

void finalizar_consola(char **separar_linea, hilos_args *hiloArgs)
{
    kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s", separar_linea[0]);

    pthread_mutex_lock(&hiloArgs->kernel->lock);
    hiloArgs->kernel_orden_apagado = 0;
    pthread_mutex_unlock(&hiloArgs->kernel->lock);

    // Detengo el hilo planificador
    hilo_planificador_detener(hiloArgs);

    kernel_finalizar(hiloArgs);

    return;
}