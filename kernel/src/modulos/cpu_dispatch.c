#include "cpu_dispatch.h"

void switch_case_cpu_dispatch(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_PROCESO:
    {
        t_cpu_kernel_proceso *proceso = deserializar_t_cpu_kernel_proceso(buffer);

        if (proceso->ejecutado)
        {
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Proceso PID:<%d> ejecutado completo. transicionar a exit", proceso->pid);

            kernel_transicion_exec_exit(args);
            kernel_finalizar_proceso(args, proceso->pid, SUCCESS);
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
            kernel_transicion_exec_ready(args);
            sem_post(&args->kernel->planificador_iniciar);
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