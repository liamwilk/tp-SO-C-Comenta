#include "cpu_dispatch.h"

void switch_case_cpu_dispatch(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_IO_STDOUT_WRITE:
    {
        t_io_stdout_write *proceso_recibido = deserializar_t_io_stdout_write(buffer);
        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        if (pcb == NULL)
        {
            free(proceso_recibido);
            break;
        }

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_STDOUT_WRITE> del PID <%d> a la interfaz <%s> porque no está conectada.", proceso_recibido->pid, proceso_recibido->interfaz);

            // Aviso a cpu
            kernel_cpu_entradasalida_no_conectada(args, CPU_IO_STDOUT_WRITE, proceso_recibido->interfaz, proceso_recibido->pid);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo stdout
        if (entrada_salida->tipo != ENTRADA_SALIDA_STDOUT)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_STDOUT_WRITE> del PID <%d> a la interfaz <%s> porque no es IO_STDOUT.", proceso_recibido->pid, proceso_recibido->interfaz);
            kernel_cpu_entradasalida_distinto_tipo(args, CPU_IO_STDOUT_WRITE, proceso_recibido->interfaz, proceso_recibido->pid);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida esta ocupada
        if (entrada_salida->ocupado)
        {
            // kernel_cpu_entradasalida_ocupada(args, CPU_IO_STDOUT_WRITE, proceso_recibido->interfaz, proceso_recibido->pid);

            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo enviar la instrucción <IO_STDOUT_WRITE> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", proceso_recibido->pid, proceso_recibido->interfaz, entrada_salida->pid);

            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);
            kernel_transicion_exec_block(args);

            // Actualizar campo tiene_proxima_io
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se actualiza el campo tiene_proxima_io del proceso <%d> a true", pcb->pid);
                pcb->proxima_io->tiene_proxima_io = true;
            }
            pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
            pcb->proxima_io->tipo = ENTRADA_SALIDA_STDOUT;
            pcb->proxima_io->args = list_create();

            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->pid));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->resultado));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registro_direccion));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registro_tamanio));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->marco));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->numero_pagina));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->direccion_fisica));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->desplazamiento));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_interfaz));
            list_add(pcb->proxima_io->args, strdup(proceso_recibido->interfaz));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.ax));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.bx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.cx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.dx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.eax));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.ebx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.ecx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.edx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.si));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.di));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.pc));

            avisar_planificador(args);

            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        pcb->quantum = interrumpir_temporizador(args);

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

        proceso_actualizar_registros(pcb, proceso_recibido->registros);

        serializar_t_io_stdout_write(&paquete, proceso_completo);
        enviar_paquete(paquete, entrada_salida->socket);
        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio la instruccion de IO_STDOUT_WRITE a la interfaz %s", entrada_salida->interfaz);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_STDOUT_WRITE.", proceso_recibido->pid);

        kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        kernel_transicion_exec_block(args);

        avisar_planificador(args);

        free(proceso_recibido->interfaz);
        free(proceso_recibido);

        break;
    }
    case CPU_KERNEL_IO_STDIN_READ:
    {
        t_io_stdin_read *proceso_recibido = deserializar_t_io_stdin_read(buffer);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        if (pcb == NULL)
        {
            break;
        }

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_STDIN_READ> del PID <%d> a la interfaz <%s> porque no está conectada.", proceso_recibido->pid, proceso_recibido->interfaz);
            kernel_cpu_entradasalida_no_conectada(args, CPU_IO_STDIN_READ, proceso_recibido->interfaz, proceso_recibido->pid);
            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo STDIN
        if (entrada_salida->tipo != ENTRADA_SALIDA_STDIN)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_STDIN_READ> del PID <%d> a la interfaz <%s> porque no es IO_STDIN.", proceso_recibido->pid, proceso_recibido->interfaz);
            kernel_cpu_entradasalida_distinto_tipo(args, CPU_IO_STDIN_READ, proceso_recibido->interfaz, proceso_recibido->pid);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida esta ocupada
        if (entrada_salida->ocupado)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo enviar la instrucción <IO_STDIN_READ> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", proceso_recibido->pid, proceso_recibido->interfaz, entrada_salida->pid);

            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);
            kernel_transicion_exec_block(args);

            // Actualizar campo tiene_proxima_io
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se actualiza el campo tiene_proxima_io del proceso <%d> a true", pcb->pid);
                pcb->proxima_io->tiene_proxima_io = true;
            }
            pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
            pcb->proxima_io->tipo = ENTRADA_SALIDA_STDIN;
            pcb->proxima_io->args = list_create();

            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->pid));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->resultado));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registro_direccion));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registro_tamanio));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->marco_inicial));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->marco_final));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->numero_pagina));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->direccion_fisica));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->desplazamiento));

            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->cantidad_marcos));
            list_add(pcb->proxima_io->args, strdup(proceso_recibido->marcos));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_marcos));

            list_add(pcb->proxima_io->args, strdup(proceso_recibido->interfaz));
            list_add(pcb->proxima_io->args, string_itoa((strlen(proceso_recibido->interfaz) + 1)));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.ax));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.bx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.cx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.dx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.eax));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.ebx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.ecx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.edx));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.si));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.di));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registros.pc));

            avisar_planificador(args);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        pcb->quantum = interrumpir_temporizador(args);

        // Actualizo la interfaz de entrada salida
        entrada_salida->ocupado = 1;
        entrada_salida->pid = proceso_recibido->pid;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_STDIN_READ del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_STDIN_READ);
        t_kernel_io_stdin_read *proceso_completo = malloc(sizeof(t_kernel_io_stdin_read));

        proceso_completo->marco_inicial = 0;  // FIXME: Borrar
        proceso_completo->marco_final = 0;    // FIXME: Borrar
        proceso_completo->desplazamiento = 0; // FIXME: Borrar

        proceso_completo->pid = proceso_recibido->pid;
        proceso_completo->resultado = proceso_recibido->resultado;
        proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
        proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
        proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
        proceso_completo->direccion_fisica = proceso_recibido->direccion_fisica;
        proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
        proceso_completo->size_interfaz = strlen(proceso_completo->interfaz) + 1;
        proceso_completo->registros = proceso_recibido->registros;
        proceso_completo->cantidad_marcos = proceso_recibido->cantidad_marcos;
        proceso_completo->marcos = strdup(proceso_recibido->marcos);
        proceso_completo->size_marcos = strlen(proceso_completo->marcos) + 1;
        proceso_actualizar_registros(pcb, proceso_recibido->registros);

        kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        kernel_transicion_exec_block(args);

        serializar_t_kernel_io_stdin_read(&paquete, proceso_completo);
        enviar_paquete(paquete, entrada_salida->socket);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio la instruccion de IO_STDIN_READ a la interfaz %s", entrada_salida->interfaz);

        avisar_planificador(args);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_STDIN_READ.", proceso_recibido->pid);

        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo);

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
        char *estado = proceso_estado(args->estados, proceso->pid);

        if (strcmp(estado, "EXIT") == 0)
        {
            avisar_planificador(args);
            free(proceso);
            break;
        }

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso->pid);

        if (proceso->ejecutado == PROCESO_EJECUTANDO)
        {
            // Checkeo que ese proceso se encuentre en exec antes de finalizarlo
            if (pcb != NULL)
            {
                interrumpir_temporizador(args);
                kernel_finalizar_proceso(args, proceso->pid, SUCCESS);
            }
        }
        else if (proceso->ejecutado == PROCESO_PARCIAL_INTERRUPCION) // El proceso se ejecuto parcialmente por interrupcion
        {
            if (pcb == NULL)
            {
                free(proceso);
                break;
            }

            // Actualizo los registros del pcb por los recibidos de CPU
            proceso_actualizar_registros(pcb, proceso->registros);
            kernel_manejar_ready(args, pcb->pid, EXEC_READY);
        }
        else if (proceso->ejecutado == PROCESO_ERROR) // La ejecucion del proceso fallo
        {
            // Lo vuelvo a mandar a ready
            if (pcb == NULL)
            {
                free(proceso);
                break;
            }
            proceso_actualizar_registros(pcb, proceso->registros);
            kernel_manejar_ready(args, pcb->pid, EXEC_READY);
        }

        avisar_planificador(args);
        free(proceso);
        break;
    }
    case CPU_KERNEL_WAIT:
    {
        t_cpu_kernel_solicitud_recurso *solicitud_recurso = deserializar_t_cpu_kernel_solicitud_recurso(buffer);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Recurso solicitado (WAIT) por CPU para el proceso <PID: %d>: %s", solicitud_recurso->pid, solicitud_recurso->nombre_recurso);

        t_pcb *proceso_en_exec = proceso_buscar_exec(args->estados, solicitud_recurso->pid);

        proceso_actualizar_registros(proceso_en_exec, *solicitud_recurso->registros);
        kernel_wait(args, solicitud_recurso->pid, solicitud_recurso->nombre_recurso);

        avisar_planificador(args);

        free(solicitud_recurso->nombre_recurso);
        free(solicitud_recurso->registros);
        free(solicitud_recurso);

        break;
    }
    case CPU_KERNEL_SIGNAL:
    {
        t_cpu_kernel_solicitud_recurso *solicitud_recurso = deserializar_t_cpu_kernel_solicitud_recurso(buffer);

        t_pcb *proceso_en_exec = proceso_buscar_exec(args->estados, solicitud_recurso->pid);
        proceso_actualizar_registros(proceso_en_exec, *solicitud_recurso->registros);
        kernel_signal(args, solicitud_recurso->pid, solicitud_recurso->nombre_recurso, SIGNAL_RECURSO);
        avisar_planificador(args);
        free(solicitud_recurso->nombre_recurso);
        free(solicitud_recurso->registros);
        free(solicitud_recurso);
        break;
    }
    case CPU_KERNEL_COPY_STRING:
    {
        t_copy_string *proceso_completo = deserializar_t_copy_string(buffer);

        t_pcb *pcb = proceso_buscar(args->estados, proceso_completo->pid);

        if (pcb != NULL)
        {
            if (proceso_completo->resultado)
            {
                avisar_planificador(args);
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se copio el string correctamente del proceso <%d>", proceso_completo->pid);
            }
            else
            {
                kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo copiar el string del proceso <%d>", proceso_completo->pid);
                kernel_finalizar_proceso(args, pcb->pid, SEGMENTATION_FAULT);
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
    case CPU_KERNEL_IO_FS_CREATE:
    {
        t_entrada_salida_fs_create *proceso_recibido = deserializar_t_entrada_salida_fs_create(buffer);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        if (pcb == NULL)
        {
            break;
        }

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_CREATE> del PID <%d> a la interfaz <%s> porque no está conectada.", proceso_recibido->pid, proceso_recibido->interfaz);
            kernel_cpu_entradasalida_no_conectada(args, CPU_IO_FS_CREATE, proceso_recibido->interfaz, proceso_recibido->pid);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo DIALFS
        if (entrada_salida->tipo != ENTRADA_SALIDA_DIALFS)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_CREATE> del PID <%d> a la interfaz <%s> porque no es DIALFS.", proceso_recibido->pid, proceso_recibido->interfaz);
            kernel_cpu_entradasalida_distinto_tipo(args, CPU_IO_FS_CREATE, proceso_recibido->interfaz, proceso_recibido->pid);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida esta ocupada
        if (entrada_salida->ocupado)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo enviar la instrucción <IO_FS_CREATE> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", proceso_recibido->pid, proceso_recibido->interfaz, entrada_salida->pid);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

            kernel_transicion_exec_block(args);

            // Actualizar campo tiene_proxima_io
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se actualiza el campo tiene_proxima_io del proceso <%d> a true", pcb->pid);
                pcb->proxima_io->tiene_proxima_io = true;
            }
            pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
            pcb->proxima_io->tipo = ENTRADA_SALIDA_DIALFS_CREATE;
            pcb->proxima_io->args = list_create();

            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->pid));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->resultado));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_interfaz));
            list_add(pcb->proxima_io->args, strdup(proceso_recibido->nombre_archivo));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_nombre_archivo));

            avisar_planificador(args);
            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido);

            break;
        }
        pcb->quantum = interrumpir_temporizador(args);

        // Actualizo la interfaz de entrada salida
        entrada_salida->ocupado = 1;
        entrada_salida->pid = proceso_recibido->pid;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_FS_CREATE del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);
        proceso_actualizar_registros(pcb, proceso_recibido->registros);

        t_entrada_salida_fs_create *proceso_completo = malloc(sizeof(t_entrada_salida_fs_create));

        proceso_completo->pid = proceso_recibido->pid;
        proceso_completo->resultado = proceso_recibido->resultado;
        proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
        proceso_completo->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
        proceso_completo->size_interfaz = strlen(proceso_recibido->interfaz) + 1;
        proceso_completo->size_nombre_archivo = strlen(proceso_recibido->nombre_archivo) + 1;

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_CREATE);

        serializar_t_entrada_salida_fs_create(&paquete, proceso_completo);

        kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        kernel_transicion_exec_block(args);
        enviar_paquete(paquete, entrada_salida->socket);

        avisar_planificador(args);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_FS_CREATE.", proceso_recibido->pid);

        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo->nombre_archivo);
        free(proceso_completo);

        free(proceso_recibido->nombre_archivo);
        free(proceso_recibido->interfaz);
        free(proceso_recibido);
        break;
    }
    case CPU_KERNEL_IO_FS_TRUNCATE:
    {
        t_cpu_kernel_fs_truncate *proceso_recibido = deserializar_t_cpu_kernel_fs_truncate(buffer);
        // Log del proceso recibido
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se recibio la instruccion <IO_FS_TRUNCATE> del PID <%d> para la interfaz <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        if (pcb == NULL)
        {
            break;
        }

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_TRUNCATE> del PID <%d> a la interfaz <%s> porque no está conectada.", proceso_recibido->pid, proceso_recibido->interfaz);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo DIALFS
        if (entrada_salida->tipo != ENTRADA_SALIDA_DIALFS)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_TRUNCATE> del PID <%d> a la interfaz <%s> porque no es DIALFS.", proceso_recibido->pid, proceso_recibido->interfaz);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido);

            break;
        }

        if (entrada_salida->ocupado)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo enviar la instrucción <IO_FS_TRUNCATE> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", proceso_recibido->pid, proceso_recibido->interfaz, entrada_salida->pid);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

            kernel_transicion_exec_block(args);

            // Actualizar campo tiene_proxima_io
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se actualiza el campo tiene_proxima_io del proceso <%d> a true", pcb->pid);
                pcb->proxima_io->tiene_proxima_io = true;
            }

            pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
            pcb->proxima_io->tipo = ENTRADA_SALIDA_DIALFS_TRUNCATE;
            pcb->proxima_io->args = list_create();

            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->pid));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->resultado));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_interfaz));
            list_add(pcb->proxima_io->args, strdup(proceso_recibido->nombre_archivo));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_nombre_archivo));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->tamanio_a_truncar));

            avisar_planificador(args);
            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido);

            break;
        }
        pcb->quantum = interrumpir_temporizador(args);

        // Actualizo la interfaz de entrada salida
        entrada_salida->ocupado = 1;
        entrada_salida->pid = proceso_recibido->pid;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_FS_TRUNCATE del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);
        proceso_actualizar_registros(pcb, proceso_recibido->registros);

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_TRUNCATE);
        t_kernel_entrada_salida_fs_truncate *proceso_completo = malloc(sizeof(t_kernel_entrada_salida_fs_truncate));
        proceso_completo->pid = proceso_recibido->pid;
        proceso_completo->resultado = proceso_recibido->resultado;
        proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
        proceso_completo->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
        proceso_completo->size_interfaz = strlen(proceso_recibido->interfaz) + 1;
        proceso_completo->size_nombre_archivo = strlen(proceso_recibido->nombre_archivo) + 1;
        proceso_completo->tamanio_a_truncar = proceso_recibido->tamanio_a_truncar;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Tamanio a truncar: %d", proceso_completo->tamanio_a_truncar);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Interfaz: %s", proceso_completo->interfaz);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size interfaz: %d", proceso_completo->size_interfaz);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Nombre archivo: %s", proceso_completo->nombre_archivo);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size nombre archivo: %d", proceso_completo->size_nombre_archivo);

        serializar_t_kernel_entrada_salida_fs_truncate(&paquete, proceso_completo);

        kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        kernel_transicion_exec_block(args);

        enviar_paquete(paquete, entrada_salida->socket);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envió la instruccion de IO_FS_TRUNCATE a la interfaz %s en socket %d", entrada_salida->interfaz, entrada_salida->socket);

        avisar_planificador(args);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_FS_TRUNCATE.", proceso_recibido->pid);

        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo->nombre_archivo);
        free(proceso_completo);

        free(proceso_recibido->nombre_archivo);
        free(proceso_recibido->interfaz);
        free(proceso_recibido);
        break;
    }
    case CPU_KERNEL_IO_FS_WRITE:
    {
        t_cpu_kernel_fs_write *proceso_recibido = deserializar_t_cpu_kernel_fs_write(buffer);

        // Log del proceso recibido
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se recibio la instruccion <IO_FS_WRITE> del PID <%d> para la interfaz <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        if (pcb == NULL)
        {
            break;
        }

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_WRITE> del PID <%d> a la interfaz <%s> porque no está conectada.", proceso_recibido->pid, proceso_recibido->interfaz);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);
            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo DIALFS
        if (entrada_salida->tipo != ENTRADA_SALIDA_DIALFS)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_WRITE> del PID <%d> a la interfaz <%s> porque no es DIALFS.", proceso_recibido->pid, proceso_recibido->interfaz);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);
            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido);
            break;
        }

        if (entrada_salida->ocupado)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo enviar la instrucción <IO_FS_WRITE> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", proceso_recibido->pid, proceso_recibido->interfaz, entrada_salida->pid);

            proceso_actualizar_registros(pcb, proceso_recibido->registros);

            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

            kernel_transicion_exec_block(args);

            // Actualizar campo tiene_proxima_io
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se actualiza el campo tiene_proxima_io del proceso <%d> a true", pcb->pid);
                pcb->proxima_io->tiene_proxima_io = true;
            }
            pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
            pcb->proxima_io->tipo = ENTRADA_SALIDA_DIALFS_WRITE;
            pcb->proxima_io->args = list_create();
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->pid));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->resultado));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_interfaz));
            list_add(pcb->proxima_io->args, strdup(proceso_recibido->nombre_archivo));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_nombre_archivo));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->puntero_archivo));
            list_add(pcb->proxima_io->args, strdup(proceso_recibido->escribir));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_escribir));
            avisar_planificador(args);
            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido->escribir);
            free(proceso_recibido);
            break;
        }

        pcb->quantum = interrumpir_temporizador(args);

        // Actualizo la interfaz de entrada salida
        entrada_salida->ocupado = 1;
        entrada_salida->pid = proceso_recibido->pid;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_FS_WRITE del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Registros de CPU del proceso <%d>:", proceso_recibido->pid);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "PC: %d", proceso_recibido->registros.pc);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "AX: %d", proceso_recibido->registros.ax);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "BX: %d", proceso_recibido->registros.bx);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "CX: %d", proceso_recibido->registros.cx);
        proceso_actualizar_registros(pcb, proceso_recibido->registros);
        kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        kernel_transicion_exec_block(args);
        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_WRITE);
        t_kernel_entrada_salida_fs_write *proceso_completo = malloc(sizeof(t_kernel_entrada_salida_fs_write));
        proceso_completo->pid = proceso_recibido->pid;
        proceso_completo->resultado = proceso_recibido->resultado;
        proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
        proceso_completo->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
        proceso_completo->size_interfaz = strlen(proceso_recibido->interfaz) + 1;
        proceso_completo->size_nombre_archivo = strlen(proceso_recibido->nombre_archivo) + 1;
        proceso_completo->puntero_archivo = proceso_recibido->puntero_archivo;
        proceso_completo->escribir = proceso_recibido->escribir;
        proceso_completo->size_escribir = proceso_recibido->size_escribir;
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Interfaz: %s", proceso_completo->interfaz);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size interfaz: %d", proceso_completo->size_interfaz);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Nombre archivo: %s", proceso_completo->nombre_archivo);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size nombre archivo: %d", proceso_completo->size_nombre_archivo);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Puntero archivo: %d", proceso_completo->puntero_archivo);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Escribir: %s", proceso_completo->escribir);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size escribir: %d", proceso_completo->size_escribir);

        serializar_t_kernel_entrada_salida_fs_write(&paquete, proceso_completo);

        enviar_paquete(paquete, entrada_salida->socket);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envió la instruccion de IO_FS_WRITE a la interfaz %s en socket %d", entrada_salida->interfaz, entrada_salida->socket);

        avisar_planificador(args);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_FS_WRITE.", proceso_recibido->pid);

        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo->nombre_archivo);
        free(proceso_completo);

        free(proceso_recibido->nombre_archivo);
        free(proceso_recibido->interfaz);
        free(proceso_recibido);
        break;
    }
    case CPU_KERNEL_IO_FS_DELETE:
    {
        t_entrada_salida_fs_create *proceso_recibido = deserializar_t_entrada_salida_fs_create(buffer);

        t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

        t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

        if (pcb == NULL)
        {
            break;
        }

        // Si la interfaz de entrada salida no esta conectada
        if (entrada_salida == NULL)
        {
            pcb->quantum = interrumpir_temporizador(args);
            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_DELETE> del PID <%d> a la interfaz <%s> porque no está conectada.", proceso_recibido->pid, proceso_recibido->interfaz);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);
            break;
        }

        // Si la interfaz de entrada salida pedida no es del tipo DIALFS
        if (entrada_salida->tipo != ENTRADA_SALIDA_DIALFS)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_DELETE> del PID <%d> a la interfaz <%s> porque no es DIALFS.", proceso_recibido->pid, proceso_recibido->interfaz);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);

            kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);

            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido);

            break;
        }

        // Si la interfaz de entrada salida esta ocupada
        if (entrada_salida->ocupado)
        {
            pcb->quantum = interrumpir_temporizador(args);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo enviar la instrucción <IO_FS_DELETE> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", proceso_recibido->pid, proceso_recibido->interfaz, entrada_salida->pid);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

            kernel_transicion_exec_block(args);

            // Actualizar campo tiene_proxima_io
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se actualiza el campo tiene_proxima_io del proceso <%d> a true", pcb->pid);
                pcb->proxima_io->tiene_proxima_io = true;
            }

            pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
            pcb->proxima_io->tipo = ENTRADA_SALIDA_DIALFS_DELETE;
            pcb->proxima_io->args = list_create();

            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->pid));
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->resultado));
            list_add(pcb->proxima_io->args, proceso_recibido->interfaz);
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_interfaz));
            list_add(pcb->proxima_io->args, proceso_recibido->nombre_archivo);
            list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_nombre_archivo));

            avisar_planificador(args);
            free(proceso_recibido->interfaz);
            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido);

            break;
        }
        pcb->quantum = interrumpir_temporizador(args);

        // Actualizo la interfaz de entrada salida
        entrada_salida->ocupado = 1;
        entrada_salida->pid = proceso_recibido->pid;

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_FS_DELETE del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);
        proceso_actualizar_registros(pcb, proceso_recibido->registros);

        t_entrada_salida_fs_create *proceso_completo = malloc(sizeof(t_entrada_salida_fs_create));

        proceso_completo->pid = proceso_recibido->pid;
        proceso_completo->resultado = proceso_recibido->resultado;
        proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
        proceso_completo->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
        proceso_completo->size_interfaz = strlen(proceso_recibido->interfaz) + 1;
        proceso_completo->size_nombre_archivo = strlen(proceso_recibido->nombre_archivo) + 1;

        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_DELETE);
        serializar_t_entrada_salida_fs_create(&paquete, proceso_completo);
        kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

        kernel_transicion_exec_block(args);
        enviar_paquete(paquete, entrada_salida->socket);

        avisar_planificador(args);

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_FS_DELETE.", proceso_recibido->pid);

        eliminar_paquete(paquete);

        free(proceso_completo->interfaz);
        free(proceso_completo->nombre_archivo);
        free(proceso_completo);

        free(proceso_recibido->nombre_archivo);
        free(proceso_recibido->interfaz);
        free(proceso_recibido);
        break;
    }
    case CPU_KERNEL_IO_FS_READ:
    {
        {
            t_cpu_kernel_fs_read *proceso_recibido = deserializar_t_cpu_kernel_fs_read(buffer);

            // Log del proceso recibido
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se recibio la instruccion <IO_FS_READ> del PID <%d> para la interfaz <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

            t_kernel_entrada_salida *entrada_salida = kernel_entrada_salida_buscar_interfaz(args, proceso_recibido->interfaz);

            t_pcb *pcb = proceso_buscar_exec(args->estados, proceso_recibido->pid);

            if (pcb == NULL)
            {
                break;
            }

            // Si la interfaz de entrada salida no esta conectada
            if (entrada_salida == NULL)
            {
                pcb->quantum = interrumpir_temporizador(args);
                kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_READ> del PID <%d> a la interfaz <%s> porque no está conectada.", proceso_recibido->pid, proceso_recibido->interfaz);
                proceso_actualizar_registros(pcb, proceso_recibido->registros);
                kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);
                free(proceso_recibido->nombre_archivo);
                free(proceso_recibido->interfaz);
                free(proceso_recibido);
                break;
            }

            // Si la interfaz de entrada salida pedida no es del tipo DIALFS
            if (entrada_salida->tipo != ENTRADA_SALIDA_DIALFS)
            {
                pcb->quantum = interrumpir_temporizador(args);
                kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar la instrucción <IO_FS_READ> del PID <%d> a la interfaz <%s> porque no es DIALFS.", proceso_recibido->pid, proceso_recibido->interfaz);
                proceso_actualizar_registros(pcb, proceso_recibido->registros);
                kernel_finalizar_proceso(args, proceso_recibido->pid, INVALID_INTERFACE);
                free(proceso_recibido->interfaz);
                free(proceso_recibido->nombre_archivo);
                free(proceso_recibido);
                break;
            }

            if (entrada_salida->ocupado)
            {
                pcb->quantum = interrumpir_temporizador(args);

                kernel_log_generic(args, LOG_LEVEL_DEBUG, "No se pudo enviar la instrucción <IO_FS_READ> del PID <%d> a la interfaz <%s> porque esta ocupada con el proceso PID <%d>", proceso_recibido->pid, proceso_recibido->interfaz, entrada_salida->pid);

                proceso_actualizar_registros(pcb, proceso_recibido->registros);
                kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

                kernel_transicion_exec_block(args);

                // Actualizar campo tiene_proxima_io
                if (pcb->proxima_io->tiene_proxima_io == false)
                {
                    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se actualiza el campo tiene_proxima_io del proceso <%d> a true", pcb->pid);
                    pcb->proxima_io->tiene_proxima_io = true;
                }
                pcb->proxima_io->identificador = strdup(entrada_salida->interfaz);
                pcb->proxima_io->tipo = ENTRADA_SALIDA_DIALFS_READ;
                pcb->proxima_io->args = list_create();
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->pid));
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->resultado));
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_interfaz));
                list_add(pcb->proxima_io->args, strdup(proceso_recibido->nombre_archivo));
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->size_nombre_archivo));
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->puntero_archivo));
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->direccion_fisica));
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->registro_tamanio));
                list_add(pcb->proxima_io->args, string_itoa(proceso_recibido->marco));
                avisar_planificador(args);
                free(proceso_recibido->interfaz);
                free(proceso_recibido->nombre_archivo);
                free(proceso_recibido);
                break;
            }

            pcb->quantum = interrumpir_temporizador(args);

            // Actualizo la interfaz de entrada salida
            entrada_salida->ocupado = 1;
            entrada_salida->pid = proceso_recibido->pid;

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envia el paquete a la interfaz <%s> asociado a la instruccion IO_FS_READ del proceso PID <%d>", proceso_recibido->interfaz, proceso_recibido->pid);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Registros de CPU del proceso <%d>:", proceso_recibido->pid);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "PC: %d", proceso_recibido->registros.pc);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "AX: %d", proceso_recibido->registros.ax);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "BX: %d", proceso_recibido->registros.bx);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "CX: %d", proceso_recibido->registros.cx);
            proceso_actualizar_registros(pcb, proceso_recibido->registros);
            kernel_log_generic(args, LOG_LEVEL_INFO, "PID: <%d> - Bloqueado por: <%s>", proceso_recibido->pid, proceso_recibido->interfaz);

            kernel_transicion_exec_block(args);
            t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_READ);
            t_kernel_entrada_salida_fs_read *proceso_completo = malloc(sizeof(t_kernel_entrada_salida_fs_read));
            proceso_completo->pid = proceso_recibido->pid;
            proceso_completo->resultado = proceso_recibido->resultado;
            proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
            proceso_completo->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
            proceso_completo->size_interfaz = strlen(proceso_recibido->interfaz) + 1;
            proceso_completo->size_nombre_archivo = strlen(proceso_recibido->nombre_archivo) + 1;
            proceso_completo->puntero_archivo = proceso_recibido->puntero_archivo;
            proceso_completo->direccion_fisica = proceso_recibido->direccion_fisica;
            proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Interfaz: %s", proceso_completo->interfaz);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size interfaz: %d", proceso_completo->size_interfaz);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Nombre archivo: %s", proceso_completo->nombre_archivo);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size nombre archivo: %d", proceso_completo->size_nombre_archivo);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Puntero archivo: %d", proceso_completo->puntero_archivo);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Dirección a escribir: %d", proceso_completo->direccion_fisica);
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Tamaño a escribir: %d", proceso_completo->registro_tamanio);

            serializar_t_kernel_entrada_salida_fs_read(&paquete, proceso_completo);

            enviar_paquete(paquete, entrada_salida->socket);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envió la instruccion de IO_FS_READ a la interfaz %s en socket %d", entrada_salida->interfaz, entrada_salida->socket);

            avisar_planificador(args);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se transiciona el PID <%d> a BLOCK por ejecucion de IO_FS_READ.", proceso_recibido->pid);

            eliminar_paquete(paquete);

            free(proceso_completo->interfaz);
            free(proceso_completo->nombre_archivo);
            free(proceso_completo);

            free(proceso_recibido->nombre_archivo);
            free(proceso_recibido->interfaz);
            free(proceso_recibido);
            break;
        }
    }
    default:
    {
        kernel_log_generic(args, LOG_LEVEL_WARNING, "Codigo operacion: <%d>", codigo_operacion);
        kernel_log_generic(args, LOG_LEVEL_WARNING, "[CPU Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_dispatch);
        break;
    }
    }
}