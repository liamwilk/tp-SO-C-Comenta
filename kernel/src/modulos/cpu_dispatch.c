#include "cpu_dispatch.h"

void switch_case_cpu_dispatch(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_RESIZE:
    {
        t_cpu_kernel_resize *proceso_recibido = deserializar_t_cpu_kernel_resize(buffer);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se recibio respuesta de Memoria sobre el pedido de RESIZE para el proceso <%d>", proceso_recibido->pid);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo redimensionar el proceso <%d>", proceso_recibido->pid);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Motivo: <%s>", proceso_recibido->motivo);

        /*
        RESIZE (Tamaño): Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
        */

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Registros del proceso <%d>:", proceso_recibido->pid);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "PC: %d", proceso_recibido->registros.pc);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "AX: %d", proceso_recibido->registros.ax);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "BX: %d", proceso_recibido->registros.bx);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "CX: %d", proceso_recibido->registros.cx);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "DX: %d", proceso_recibido->registros.dx);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "EAX: %d", proceso_recibido->registros.eax);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "EBX: %d", proceso_recibido->registros.ebx);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "ECX: %d", proceso_recibido->registros.ecx);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "EDX: %d", proceso_recibido->registros.edx);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "SI: %d", proceso_recibido->registros.si);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "DI: %d", proceso_recibido->registros.di);

        // TODO: Preguntar que se hace en este caso? El proceso va a exit?
        kernel_transicion_exec_exit(args);

        free(proceso_recibido->motivo);
        free(proceso_recibido);
        break;
    }
    case CPU_KERNEL_PROCESO:
    {
        t_cpu_kernel_proceso *proceso = deserializar_t_cpu_kernel_proceso(buffer);

        // Este caso se da cuando el usuario interrumpio a CPU para finalizar un proceso
        t_pcb *proceso_en_exit = proceso_buscar_exit(args->estados, proceso->pid);
        if (proceso_en_exit != NULL)
        {
            // Detener QUANTUM si es RR o VRR
            interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> - Motivo: <INTERRUPTED_BY_USER>", proceso_en_exit->pid);
            proceso_matar(args->estados, string_itoa(proceso_en_exit->pid));
            free(proceso);
            avisar_planificador(args);
            break;
        }

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso->pid);
        if (proceso->ejecutado)
        {
            // Checkeo que ese proceso se encuentre en exec antes de finalizarlo
            if (pcb != NULL)
            {
                interrumpir_temporizador(args);
            }
            kernel_finalizar_proceso(args, proceso->pid, SUCCESS);
            kernel_avisar_memoria_finalizacion_proceso(args, proceso->pid);
        }
        else
        {
            // Actualizo los registros del pcb por los recibidos de CPU
            proceso_actualizar_registros(pcb, proceso->registros);
        }

        avisar_planificador(args);
        free(proceso);
        break;
    }
    case CPU_KERNEL_WAIT:
    {
        t_cpu_kernel_solicitud_recurso *solicitud_recurso = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        solicitud_recurso = deserializar_t_cpu_kernel_solicitud_recurso(buffer);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Recurso solicitado (WAIT) por CPU para el proceso <PID: %d>: %s", solicitud_recurso->pid, solicitud_recurso->nombre_recurso);

        // TODO: Implementar logica del manejo de WAIT en kernel.

        free(solicitud_recurso->nombre_recurso);
        free(solicitud_recurso);

        break;
    }
    case CPU_KERNEL_SIGNAL:
    {
        t_cpu_kernel_solicitud_recurso *solicitud_recurso = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        solicitud_recurso = deserializar_t_cpu_kernel_solicitud_recurso(buffer);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Recurso liberado (SIGNAL) por CPU para el proceso <PID: %d>: %s", solicitud_recurso->pid, solicitud_recurso->nombre_recurso);

        // TODO: Implementar logica del manejo de SIGNAL en kernel.

        free(solicitud_recurso->nombre_recurso);
        free(solicitud_recurso);
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