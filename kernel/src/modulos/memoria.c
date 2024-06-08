#include "memoria.h"
void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case MEMORIA_KERNEL_NUEVO_PROCESO:
    {
        t_memoria_kernel_proceso *proceso = deserializar_t_memoria_kernel_proceso(buffer);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Recibí la respuesta de Memoria acerca de la solicitud de nuevo proceso");
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "PID: %d", proceso->pid);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Cantidad de instrucciones: %d", proceso->cantidad_instrucciones);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Leido: %d", proceso->leido);

        char *estado = proceso_estado(args->estados, proceso->pid);

        if (strcmp(estado, "EXIT") == 0)
        {
            kernel_log_generic(args, LOG_LEVEL_WARNING, "Proceso PID:<%d> ya ha sido eliminado de kernel, no se lo planifica", proceso->pid);
            break;
        }
        if (proceso->leido)
        {
            // Enviar a cpu los registros
            // t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
            t_pcb *pcb = proceso_buscar_new(args->estados, proceso->pid);
            if (pcb == NULL)
            {

                break;
            }
            // serializar_t_registros_cpu(&paquete, pcb->pid, pcb->registros_cpu);
            // enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            // Buscar proceso
            pcb->memoria_aceptado = true;
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Proceso PID:<%d> aceptado en memoria", pcb->pid);
            sem_post(&args->kernel->planificador_iniciar);
        }
        else
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "Proceso PID:<%d> rechazado en memoria", proceso->pid);
            // Eliminar proceso de la cola de new
            proceso_revertir(args->estados, proceso->pid);
            kernel_log_generic(args, LOG_LEVEL_WARNING, "Proceso PID:<%d> eliminado de kernel", proceso->pid);
        };
        free(proceso);
        break;
    }
    default:
    {
        kernel_log_generic(args, LOG_LEVEL_WARNING, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
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