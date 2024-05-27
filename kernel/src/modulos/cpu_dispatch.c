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
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Proceso PID:<%d> ejecutado completo. Transicionar a exit", proceso->pid);

            kernel_transicion_exec_exit(args);
            kernel_finalizar_proceso(args, proceso->pid, SUCCESS);
            t_paquete *paquete_finalizar = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
            t_kernel_memoria_finalizar_proceso finalizar_proceso = {.pid = proceso->pid};
            serializar_t_kernel_memoria_finalizar_proceso(&paquete_finalizar, &finalizar_proceso);
            enviar_paquete(paquete_finalizar, args->kernel->sockets.memoria);
            eliminar_paquete(paquete_finalizar);
            sem_post(&args->kernel->planificador_iniciar);
        }
        else
        {
            // Actualizo los registros del proceso en exec con los que me envia la CPU
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Actualizo registros recibidos de PID <%d> por interrupcion.", proceso->pid);

            // Envio el proceso a ready desde exec
            t_pcb *pcb = kernel_transicion_exec_ready(args);

            // Actualizo los registros del pcb por los recibios de CPU
            pcb->registros_cpu->ax = proceso->registros.ax;
            pcb->registros_cpu->bx = proceso->registros.bx;
            pcb->registros_cpu->cx = proceso->registros.cx;
            pcb->registros_cpu->dx = proceso->registros.dx;
            pcb->registros_cpu->pc = proceso->registros.pc;
            pcb->registros_cpu->eax = proceso->registros.eax;
            pcb->registros_cpu->ebx = proceso->registros.ebx;
            pcb->registros_cpu->ecx = proceso->registros.ecx;
            pcb->registros_cpu->edx = proceso->registros.edx;
            pcb->registros_cpu->si = proceso->registros.si;
            pcb->registros_cpu->di = proceso->registros.di;

            // Vuelvo a iniciar el planificador
            sem_post(&args->kernel->planificador_iniciar);
        }

        free(proceso);
        break;
    }
    default:
    {
        kernel_log_generic(args, LOG_LEVEL_WARNING, "[CPU Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_dispatch);
        break;
    }
    }
}