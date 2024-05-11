#include "memoria.h"
void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case MEMORIA_KERNEL_NUEVO_PROCESO:
    {
        t_memoria_kernel_proceso *proceso = deserializar_t_memoria_kernel_proceso(buffer);

        log_debug(logger, "Recibí la respuesta de Memoria acerca de la solicitud de nuevo proceso");
        log_debug(logger, "PID: %d", proceso->pid);
        log_debug(logger, "Cantidad de instrucciones: %d", proceso->cantidad_instrucciones);
        log_debug(logger, "Leido: %d", proceso->leido);
        if (proceso->leido)
        {
            // Enviar a cpu los registros
            // t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
            t_pcb *pcb = proceso_buscar_new(args->estados, proceso->pid);
            // serializar_t_registros_cpu(&paquete, pcb->pid, pcb->registros_cpu);
            // enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            // Buscar proceso
            pcb->memoria_aceptado = true;
            log_debug(logger, "Proceso PID:<%d> aceptado en memoria", pcb->pid);
            log_info(logger, "Se crea el proceso <%d> en NEW", pcb->pid);
        }
        else
        {
            log_error(logger, "Proceso PID:<%d> rechazado en memoria", proceso->pid);
            // Eliminar proceso de la cola de new
            proceso_revertir(args->estados, proceso->pid);
            log_warning(logger, "Proceso PID:<%d> eliminado de kernel", proceso->pid);
        };
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