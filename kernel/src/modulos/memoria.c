#include "memoria.h"
void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case MEMORIA_KERNEL_NUEVO_PROCESO:
    {
        t_memoria_kernel_proceso *proceso = deserializar_t_memoria_kernel_proceso(buffer);

        log_generic(args, LOG_LEVEL_DEBUG, "Recibí la respuesta de Memoria acerca de la solicitud de nuevo proceso");
        log_generic(args, LOG_LEVEL_DEBUG, "PID: %d", proceso->pid);
        log_generic(args, LOG_LEVEL_DEBUG, "Cantidad de instrucciones: %d", proceso->cantidad_instrucciones);
        log_generic(args, LOG_LEVEL_DEBUG, "Leido: %d", proceso->leido);
        if (proceso->leido)
        {
            // Enviar a cpu los registros
            // t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
            t_pcb *pcb = proceso_buscar_new(args->estados, proceso->pid);
            // serializar_t_registros_cpu(&paquete, pcb->pid, pcb->registros_cpu);
            // enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            // Buscar proceso
            pcb->memoria_aceptado = true;
            log_generic(args, LOG_LEVEL_DEBUG, "Proceso PID:<%d> aceptado en memoria", pcb->pid);
            log_generic(args, LOG_LEVEL_INFO, "Proceso PID:<%d> aceptado en memoria", pcb->pid);
        }
        else
        {
            log_generic(args, LOG_LEVEL_ERROR, "Proceso PID:<%d> rechazado en memoria", proceso->pid);
            // Eliminar proceso de la cola de new
            proceso_revertir(args->estados, proceso->pid);
            log_generic(args, LOG_LEVEL_WARNING, "Proceso PID:<%d> eliminado de kernel", proceso->pid);
        };
        sem_post(&args->kernel->memoria_consola_nuevo_proceso);
        free(proceso);
        break;
    }
    default:
    {
        log_warning(args->logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.memoria);
        break;
    }
    }
}

bool hilo_planificador_obtener_estado(hilos_args *args)
{
    bool i;
    pthread_mutex_lock(&args->kernel->lock);
    i = args->kernel->detener_planificador;
    pthread_mutex_unlock(&args->kernel->lock);
    return i;
}