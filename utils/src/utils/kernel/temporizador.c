#include "temporizador.h"

// Interrumpe el temporizador y devuelve el quantum restante
int interrumpir_temporizador(hilos_args *args)
{
    if (determinar_algoritmo(args->kernel->algoritmoPlanificador) == FIFO)
    {
        return 0;
    }

    struct itimerspec quantum_restante;
    // Obtener el tiempo restante del temporizador
    if (timer_gettime(args->timer, &quantum_restante) == -1)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "Error al obtener el tiempo restante del temporizador");
        return -1;
    }

    if (timer_delete(args->timer) == -1)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "Error al eliminar el temporizador");
        return -1;
    }
    else
    {
        if (quantum_restante.it_value.tv_sec > 0)
        {
            kernel_log_generic(args, LOG_LEVEL_WARNING, "[QUANTUM] Al proceso en ejecución se lo ha interrumpido y le sobra QUANTUM: <%ld> milisegundos", quantum_restante.it_value.tv_sec * 1000);
        }
    }
    return quantum_restante.it_value.tv_sec * 1000;
}

void iniciar_temporizador(hilos_args *args, double milisegundos)
{
    // Crea el temporizador
    timer_create(CLOCK_REALTIME, &args->sev, &args->timer);

    // Configura el tiempo de inicio y el intervalo del temporizador
    double tiempo_total = milisegundos / 1000.0;
    time_t segundos = (time_t)tiempo_total;
    long nanosegundos = (long)((tiempo_total - segundos) * 1000000000L);

    args->its.it_value.tv_sec = segundos;
    args->its.it_value.tv_nsec = nanosegundos;
    args->its.it_interval.tv_sec = 0;
    args->its.it_interval.tv_nsec = 0;

    // Inicia el temporizador
    timer_settime(args->timer, 0, &args->its, NULL);
}

// Que hacer si me interrumpieron por señal
void signal_handler(int signum)
{
    // printf("Hilo interrumpido por señal %d\n", signum);
    return;
}