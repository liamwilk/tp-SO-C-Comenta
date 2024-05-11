#include "cpu_interrupt.h"
void switch_case_cpu_interrupt(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
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
        }
        else
        {
            // Mover proceso a ready
            t_pcb *pcb = proceso_buscar_new(args->estados, proceso->pid);
            pcb->memoria_aceptado = false;
            proceso_push_ready(args->estados, pcb, logger);
            log_info(logger, "Se mueve el proceso <%d> a READY", proceso->pid);
        }

        free(proceso);
        break;
    }
    case CPU_KERNEL_IO_GEN_SLEEP:
    {
        t_cpu_kernel_io_gen_sleep *sleep = deserializar_t_cpu_kernel_io_gen_sleep(buffer);

        log_debug(logger, "Recibí la solicitud de CPU para activar IO_GEN_SLEEP en la Interfaz %s por %d unidades de trabajo asociado al PID %d", sleep->interfaz, sleep->tiempo, sleep->pid);

        if (args->kernel->sockets.entrada_salida_generic == 0)
        {
            log_error(args->logger, "No se pudo enviar el paquete a IO Generic porque aun no se conecto.");
            break;
        }

        // TODO: Implementar mapeo de logica de IDs de IO a sockets de IO. Actualmente solo se envia a IO Generic.

        log_debug(args->logger, "Se envia un sleep de 10 segundos a IO Generic ID %s", sleep->interfaz);
        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);

        t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
        unidad->pid = sleep->pid;
        unidad->unidad_de_trabajo = sleep->tiempo;

        serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);

        enviar_paquete(paquete, args->kernel->sockets.entrada_salida_generic);
        log_debug(args->logger, "Se envio el paquete a IO Generic");

        eliminar_paquete(paquete);
        free(unidad);

        break;
    }
    default:
    {
        log_warning(args->logger, "[CPU Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_interrupt);
        break;
    }
    }
}
