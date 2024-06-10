#include "cpu_dispatch.h"

void switch_case_cpu_dispatch(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_IO_STDOUT_WRITE:
    {
        t_io_stdout_write *proceso_recibido = deserializar_t_io_stdout_write(buffer);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "Aviso a CPU que no se pudo enviar el paquete a la interfaz <%s> porque no está conectada", proceso_recibido->interfaz);

            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDOUT_WRITE);
            t_kernel_cpu_io_stdout_write *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdout_write));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("La interfaz solicitada no se encuentra conectada a Kernel");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdout_write(&paquete, proceso_enviar);
            enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo stdout
        if (entrada_salida->tipo != ENTRADA_SALIDA_STDOUT)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque no es del tipo IO_STDOUT.", proceso_recibido->interfaz);

            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDOUT_WRITE);
            t_kernel_cpu_io_stdout_write *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdout_write));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDOUT_WRITE porque no es del tipo IO_STDOUT");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdout_write(&paquete, proceso_enviar);
            enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida esta ocupada
        if (entrada_salida->ocupado)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque esta ocupada con el proceso %d.", proceso_recibido->interfaz, entrada_salida->pid);

            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDOUT_WRITE);
            t_kernel_cpu_io_stdout_write *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdout_write));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDOUT_WRITE porque se encuentra ocupada con otro proceso");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdout_write(&paquete, proceso_enviar);
            enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Actualizo la interfaz de entrada salida
        entrada_salida->ocupado = 1;
        entrada_salida->pid = proceso_recibido->pid;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_STDOUT_WRITE del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_STDOUT_WRITE);
        t_io_stdout_write *proceso_completo = malloc(sizeof(t_io_stdout_write));

        proceso_completo->pid = proceso_recibido->pid;
        proceso_completo->resultado = proceso_recibido->resultado;
        proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
        proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
        proceso_completo->marco = proceso_recibido->marco;
        proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
        proceso_completo->direccion_fisica = proceso_recibido->direccion_fisica;
        proceso_completo->desplazamiento = proceso_recibido->desplazamiento;
        proceso_completo->size_interfaz = proceso_recibido->size_interfaz;
        proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
        proceso_completo->registros = proceso_recibido->registros;

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        // Actualizo los registros del proceso en Kernel
        proceso_actualizar_registros(pcb, proceso_recibido->registros);

        serializar_t_io_stdout_write(&paquete, proceso_completo);
        enviar_paquete(paquete, entrada_salida->socket);
        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio la instruccion de IO_STDOUT_WRITE a la interfaz %s", entrada_salida->interfaz);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_STDOUT_WRITE.", proceso_recibido->pid);
        kernel_transicion_exec_block(args);

        free(proceso_recibido->interfaz);
        free(proceso_recibido);

        break;
    }
    case CPU_KERNEL_IO_STDIN_READ:
    {
        t_io_stdin_read *proceso_recibido = deserializar_t_io_stdin_read(buffer);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "Aviso a CPU que no se pudo enviar el paquete a la interfaz <%s> porque no esta conectada", proceso_recibido->interfaz);

            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDIN_READ);
            t_kernel_cpu_io_stdin_read *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdin_read));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("La interfaz solicitada no se encuentra conectada a Kernel");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdin_read(&paquete, proceso_enviar);
            enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo STDIN
        if (entrada_salida->tipo != ENTRADA_SALIDA_STDIN)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque no es del tipo IO_STDIN.", proceso_recibido->interfaz);

            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDIN_READ);
            t_kernel_cpu_io_stdin_read *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdin_read));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDIN_READ porque no es del tipo IO_STDIN");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdin_read(&paquete, proceso_enviar);
            enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida esta ocupada
        if (entrada_salida->ocupado)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque esta ocupada con el proceso %d.", proceso_recibido->interfaz, entrada_salida->pid);

            t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDIN_READ);
            t_kernel_cpu_io_stdin_read *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdin_read));

            proceso_enviar->pid = proceso_recibido->pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDIN_READ porque se encuentra ocupada con otro proceso");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            serializar_t_kernel_cpu_io_stdin_read(&paquete, proceso_enviar);
            enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_enviar->motivo);
            free(proceso_enviar);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Actualizo la interfaz de entrada salida
        entrada_salida->ocupado = 1;
        entrada_salida->pid = proceso_recibido->pid;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_STDIN_READ del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_STDIN_READ);
        t_kernel_io_stdin_read *proceso_completo = malloc(sizeof(t_kernel_io_stdin_read));

        proceso_completo->pid = proceso_recibido->pid;
        proceso_completo->resultado = proceso_recibido->resultado;
        proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
        proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
        proceso_completo->marco_inicial = proceso_recibido->marco_inicial;
        proceso_completo->marco_final = proceso_recibido->marco_final;
        proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
        proceso_completo->direccion_fisica = proceso_recibido->direccion_fisica;
        proceso_completo->desplazamiento = proceso_recibido->desplazamiento;
        proceso_completo->size_interfaz = proceso_recibido->size_interfaz;
        proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
        proceso_completo->registros = proceso_recibido->registros;

        proceso_actualizar_registros(proceso_buscar_exec(args->estados, proceso_recibido->pid), proceso_recibido->registros);

        serializar_t_kernel_io_stdin_read(&paquete, proceso_completo);
        enviar_paquete(paquete, entrada_salida->socket);
        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio la instruccion de IO_STDIN_READ a la interfaz %s", entrada_salida->interfaz);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_STDIN_READ.", proceso_recibido->pid);

        kernel_transicion_exec_block(args);

        free(proceso_recibido->interfaz);
        free(proceso_recibido);

        break;
    }
    case CPU_KERNEL_RESIZE:
    {
        t_cpu_kernel_resize *proceso_recibido = deserializar_t_cpu_kernel_resize(buffer);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se recibio respuesta de Memoria sobre el pedido de RESIZE para el proceso <%d>", proceso_recibido->pid);

        t_pcb *proceso = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        if (proceso == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "[CPU Dispatch/RESIZE] Posible condiciones de carrera, el proceso <%d> no se encuentra en EXEC", proceso_recibido->pid);
            interrumpir_temporizador(args);
            kernel_finalizar_proceso(args, proceso_recibido->pid, OUT_OF_MEMORY);
            break;
        }

        if (proceso_recibido->resultado)
        {
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se pudo redimensionar el proceso <%d>", proceso_recibido->pid);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Motivo: <%s>", proceso_recibido->motivo);
            avisar_planificador(args);
        }
        else
        {
            proceso->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo redimensionar el proceso <%d>", proceso_recibido->pid);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "Motivo: <%s>", proceso_recibido->motivo);

            proceso_actualizar_registros(proceso, proceso_recibido->registros);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Registros del proceso <%d>:", proceso->pid);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "PC: %d", proceso->registros_cpu->pc);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "AX: %d", proceso->registros_cpu->ax);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "BX: %d", proceso->registros_cpu->bx);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "CX: %d", proceso->registros_cpu->cx);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "DX: %d", proceso->registros_cpu->dx);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "EAX: %d", proceso->registros_cpu->eax);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "EBX: %d", proceso->registros_cpu->ebx);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "ECX: %d", proceso->registros_cpu->ecx);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "EDX: %d", proceso->registros_cpu->edx);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "SI: %d", proceso->registros_cpu->si);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "DI: %d", proceso->registros_cpu->di);
            kernel_finalizar_proceso(args, proceso_recibido->pid, OUT_OF_MEMORY);
            avisar_planificador(args);
        }

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
            kernel_log_generic(args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> - Motivo: <INTERRUPTED_BY_USER>", proceso_en_exit->pid);
            proceso_matar(args->estados, string_itoa(proceso_en_exit->pid));
            free(proceso);
            avisar_planificador(args);
            break;
        }

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso->pid);

        if (proceso->ejecutado == 1)
        {
            // Checkeo que ese proceso se encuentre en exec antes de finalizarlo
            if (pcb != NULL)
            {
                interrumpir_temporizador(args);
                kernel_finalizar_proceso(args, proceso->pid, SUCCESS);
            }
        }
        else if (proceso->ejecutado == 2) // El proceso se ejecuto parcialmente por interrupcion
        {
            if (pcb == NULL)
            {
                kernel_log_generic(args, LOG_LEVEL_ERROR, "[CPU Dispatch] Posible condiciones de carrera, el proceso <%d> no se encuentra en EXEC", proceso->pid);
                break;
            }
            // Actualizo los registros del pcb por los recibidos de CPU
            proceso_actualizar_registros(pcb, proceso->registros);
            kernel_manejar_ready(args, pcb->pid, EXEC_READY);
        }
        else if (proceso->ejecutado == 0) // La ejecucion del proceso fallo
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "Proceso PID:<%d> ejecutado fallido. Transicionar a exit", proceso->pid);

            kernel_finalizar_proceso(args, proceso->pid, SUCCESS);

            sem_post(&args->kernel->planificador_iniciar);
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

        t_pcb *proceso_en_exec = proceso_buscar_exec(args->estados, solicitud_recurso->pid);

        proceso_actualizar_registros(proceso_en_exec, *solicitud_recurso->registros);
        kernel_recurso_wait(args, args->recursos, solicitud_recurso->pid, solicitud_recurso->nombre_recurso);

        avisar_planificador(args);

        free(solicitud_recurso->nombre_recurso);
        free(solicitud_recurso);

        break;
    }
    case CPU_KERNEL_SIGNAL:
    {
        t_cpu_kernel_solicitud_recurso *solicitud_recurso = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        solicitud_recurso = deserializar_t_cpu_kernel_solicitud_recurso(buffer);

        t_pcb *proceso_en_exec = proceso_buscar_exec(args->estados, solicitud_recurso->pid);
        proceso_actualizar_registros(proceso_en_exec, *solicitud_recurso->registros);
        kernel_recurso_signal(args, args->recursos, solicitud_recurso->pid, solicitud_recurso->nombre_recurso, SIGNAL_RECURSO);
        avisar_planificador(args);
        free(solicitud_recurso->nombre_recurso);
        free(solicitud_recurso);
        break;
    }
    case CPU_KERNEL_COPY_STRING:
    {
        t_copy_string *proceso_completo = deserializar_t_copy_string(buffer);

        t_pcb *pcb = buscar_proceso(args->estados, proceso_completo->pid);

        if (pcb != NULL)
        {
            if (proceso_completo->resultado)
            {
                pcb->quantum = interrumpir_temporizador(args);
                avisar_planificador(args);
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se copio el string correctamente del proceso <%d>", proceso_completo->pid);
            }
            else
            {
                kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo copiar el string del proceso <%d>", proceso_completo->pid);
                kernel_finalizar_proceso(args, pcb->pid, INVALID_RESOURCE);
            }
        }
        else
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "[CPU Dispatch] Posible condiciones de carrera, el proceso <%d> no se encuentra en EXEC", proceso_completo->pid);
        }
        free(proceso_completo->frase);
        free(proceso_completo);
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