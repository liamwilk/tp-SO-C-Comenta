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

            // Busco el proceso en exec o en ready ya que hay variantes y puede estar en uno o el otro.
            // Es un tema de sincronizacion, porque estoy recibiendo de CPU que el proceso fue desalojado, asi que
            // tecnicamente no está ejecutando.
            // TODO: Hay que revisar esto.

            t_pcb *pcb1 = proceso_buscar_exec(args->estados, proceso->pid);
            t_pcb *pcb2 = proceso_buscar_ready(args->estados, proceso->pid);
            t_pcb *pcb = pcb1;

            if (pcb2 == NULL && pcb1 == NULL)
            {
                kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro el proceso PID <%d> en exec ni en ready", proceso->pid);
                break;
            }

            // Si no se encontro en exec, se encontro en ready
            if (pcb1 == NULL)
            {
                pcb = pcb2;
            }

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