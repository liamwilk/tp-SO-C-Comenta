#include "cpu_dispatch.h"

void switch_case_cpu_dispatch(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_PROCESO:
    {
        t_cpu_kernel_proceso *proceso = deserializar_t_cpu_kernel_proceso(buffer);

        log_debug(logger, "Recibí la respuesta de CPU acerca de la ejecucion de un proceso");
        log_debug(logger, "PID: %d", proceso->pid);
        log_debug(logger, "Cantidad de instrucciones ejecutadas: %d", proceso->registros->pc);
        log_debug(logger, "Ejecutado completo: %d", proceso->ejecutado);

        // TODO: En CPU implementar lo siguiente: ejecutado = 1 para ejecutado completo, 0 para interrumpido y pendiente, -1 para error.
        if (proceso->ejecutado)
        {
            log_debug(logger, "Proceso PID:<%d> ejecutado completo.", proceso->pid);

            log_debug(logger, "Le doy notificacion a Memoria para que elimine el proceso PID:<%d>", proceso->pid);

            t_paquete *paquete_finalizar = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
            t_kernel_memoria_finalizar_proceso *finalizar_proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
            finalizar_proceso->pid = proceso->pid;
            serializar_t_kernel_memoria_finalizar_proceso(&paquete_finalizar, finalizar_proceso);
            enviar_paquete(paquete_finalizar, args->kernel->sockets.memoria);
            eliminar_paquete(paquete_finalizar);

            sem_post(&args->kernel->planificador_iniciar);
        }
        else
        {
            // // Mover proceso a ready
            // t_pcb *pcb = proceso_buscar_new(args->estados, proceso->pid);
            // pcb->memoria_aceptado = false;
            // proceso_push_ready(args->estados, pcb);
            // log_info(logger, "Se mueve el proceso <%d> a READY", proceso->pid);
            log_warning(logger, "Mandar proceso PID: %d a READY", proceso->pid);
        }

        free(proceso);
        break;
    }
    default:
    {
        log_warning(args->logger, "[CPU Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_dispatch);
        break;
    }
    }
}