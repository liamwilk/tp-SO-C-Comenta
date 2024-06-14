#include "../hilos.h"

void switch_case_kernel_entrada_salida_stdin(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case ENTRADA_SALIDA_KERNEL_IO_STDIN_READ:
    {
        t_entrada_salida_kernel_io_stdin_read *proceso_recibido = deserializar_t_entrada_salida_kernel_io_stdin_read(buffer);

        // Verifico si este proceso no ha ya sido marcado como eliminado  en kernel
        if (kernel_verificar_proceso_en_exit(io_args->args, proceso_recibido->pid))
        {
            break;
        };

        if (proceso_recibido->resultado)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%s/%d] Se completó la operación de IO_STDIN_READ para el proceso PID <%d>", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, proceso_recibido->pid);

            io_args->entrada_salida->ocupado = 0;
            kernel_manejar_ready(io_args->args, proceso_recibido->pid, BLOCK_READY);

            // Aviso a CPU que termino la ejecucion del proceso opcode KERNEL_CPU_IO_STDIN_READ
            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDIN_READ);
            t_kernel_cpu_io_stdin_read *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdin_read));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 1;
            proceso_enviar->motivo = strdup("Se completó la operación de IO_STDIN_READ");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdin_read(&paquete, proceso_enviar);
            enviar_paquete(paquete, io_args->args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            // Inicio el planificador nuevamente
            avisar_planificador(io_args->args);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
        }
        else
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_ERROR, "[%s/%d] Ocurrió un error en la operación de escritura en consola para el proceso PID %d", modulo, io_args->entrada_salida->orden, proceso_recibido->pid);
            t_pcb *pcb = proceso_buscar(io_args->args->estados, proceso_recibido->pid);
            pcb->quantum = interrumpir_temporizador(io_args->args);
            // Aviso a CPU que termino la ejecucion del proceso opcode KERNEL_CPU_IO_STDOUT_WRITE
            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDOUT_WRITE);
            t_kernel_cpu_io_stdout_write *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdout_write));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("Error en la operación de escritura en consola de IO_STDOUT");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdout_write(&paquete, proceso_enviar);
            enviar_paquete(paquete, io_args->args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            // Elimino el proceso de la cola de bloqueados
            // TODO: Preguntar que pasa en este caso? Lo elimino? Lo mando a ready de nuevo??
            kernel_finalizar_proceso(io_args->args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
        }

        free(proceso_recibido);
        break;
    }
    case ENTRADA_SALIDA_KERNEL_IDENTIFICACION:
    {
        t_entrada_salida_identificacion *identificacion = deserializar_t_entrada_salida_identificacion(buffer);

        if (kernel_entrada_salida_buscar_interfaz(io_args->args, identificacion->identificador) != NULL)
        {
            kernel_entradasalida_rechazo(io_args, modulo, identificacion->identificador);
            liberar_conexion(&io_args->entrada_salida->socket);
            break;
        }

        entrada_salida_agregar_identificador(io_args, identificacion->identificador);

        kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/%s/%d] Se recibio identificador válido.", modulo, identificacion->identificador, io_args->entrada_salida->orden);
        free(identificacion->identificador);
        free(identificacion);
        break;
    }
    default:
    {
        kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/%s/%d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden);
        liberar_conexion(&io_args->entrada_salida->socket);
        break;
    }
    }
}