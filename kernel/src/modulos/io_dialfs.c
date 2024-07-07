#include "../hilos.h"

void switch_case_kernel_entrada_salida_dialfs(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
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
    case ENTRADA_SALIDA_KERNEL_IO_FS_CREATE:
    {
        t_entrada_salida_fs_create *create = deserializar_t_entrada_salida_fs_create(buffer);
        t_kernel_cpu_io_fs_create *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_fs_create));

        // Verifico si este proceso no ha ya sido marcado como eliminado  en kernel
        if (kernel_verificar_proceso_en_exit(io_args->args, create->pid))
        {
            break;
        };

        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_FS_CREATE);

        if (create->resultado == 1)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%s/%d] Se completó la operación de IO_FS_CREATE para el proceso PID <%d>", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, create->pid);
            t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(io_args->args, create->pid);
            if (io == NULL)
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_ERROR, "No se encontro el tipo DIALFS para el proceso PID <%d>", create->pid);
            }
            else
            {
                io_args->entrada_salida->ocupado = 0;
                io_args->entrada_salida->pid = 0;
                kernel_manejar_ready(io_args->args, create->pid, BLOCK_READY);

                proceso_enviar->pid = create->pid;
                proceso_enviar->resultado = 1;
                proceso_enviar->motivo = strdup("Se completó la operación de IO_FS_CREATE");

                proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

                serializar_t_kernel_cpu_io_fs_create(&paquete, proceso_enviar);
                enviar_paquete(paquete, io_args->args->kernel->sockets.cpu_dispatch);

                kernel_proximo_io_fs(io_args->args, io);
                avisar_planificador(io_args->args);
            }
        }
        else
        {
            kernel_manejar_ready(io_args->args, create->pid, BLOCK_READY);
            proceso_enviar->pid = pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("Ocurrio un error a la hora de crear el archivo.");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            io_args->entrada_salida->ocupado = 0;
            io_args->entrada_salida->pid = 0;

            serializar_t_kernel_cpu_io_fs_create(&paquete, proceso_enviar);
            enviar_paquete(paquete, io_args->args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);
            free(proceso_enviar->motivo);
            free(proceso_enviar);
        }
        free(create);
        break;
    }
    case ENTRADA_SALIDA_KERNEL_IO_FS_TRUNCATE:
    {
        t_kernel_entrada_salida_fs_truncate *truncate = deserializar_t_kernel_entrada_salida_fs_truncate(buffer);
        // Verifico si este proceso no ha ya sido marcado como eliminado  en kernel
        if (kernel_verificar_proceso_en_exit(io_args->args, truncate->pid))
        {
            break;
        };

        if (truncate->resultado == 1)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%s/%d] Se completó la operación de IO_FS_TRUNCATE para el proceso PID <%d>", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, truncate->pid);
            t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(io_args->args, truncate->pid);
            if (io == NULL)
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_ERROR, "No se encontro el tipo DIALFS para el proceso PID <%d>", truncate->pid);
            }
            else
            {
                io_args->entrada_salida->ocupado = 0;
                io_args->entrada_salida->pid = 0;
                kernel_manejar_ready(io_args->args, truncate->pid, BLOCK_READY);
                kernel_proximo_io_fs(io_args->args, io);
            }
        }
        else
        {
            kernel_manejar_ready(io_args->args, truncate->pid, BLOCK_READY);
        }
        free(truncate);
        avisar_planificador(io_args->args);
        break;
    }
    case ENTRADA_SALIDA_KERNEL_IO_FS_DELETE:
    {
        t_entrada_salida_fs_create *delete = deserializar_t_entrada_salida_fs_create(buffer);
        t_kernel_cpu_io_fs_create *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_fs_create));

        // Verifico si este proceso no ha ya sido marcado como eliminado  en kernel
        if (kernel_verificar_proceso_en_exit(io_args->args, delete->pid))
        {
            break;
        };

        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_FS_DELETE);

        if (delete->resultado == 1)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%s/%d] Se completó la operación de IO_FS_DELETE para el proceso PID <%d>", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, delete->pid);
            t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(io_args->args, delete->pid);
            if (io == NULL)
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_ERROR, "No se encontro el tipo DIALFS para el proceso PID <%d>", delete->pid);
            }
            else
            {
                io_args->entrada_salida->ocupado = 0;
                io_args->entrada_salida->pid = 0;
                kernel_manejar_ready(io_args->args, delete->pid, BLOCK_READY);

                proceso_enviar->pid = delete->pid;
                proceso_enviar->resultado = 1;
                proceso_enviar->motivo = strdup("Se completó la operación de IO_FS_DELETE");

                proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

                serializar_t_kernel_cpu_io_fs_create(&paquete, proceso_enviar);
                enviar_paquete(paquete, io_args->args->kernel->sockets.cpu_dispatch);

                kernel_proximo_io_fs(io_args->args, io);
                avisar_planificador(io_args->args);
            }
        }
        else
        {
            kernel_manejar_ready(io_args->args, delete->pid, BLOCK_READY);
            proceso_enviar->pid = pid;
            proceso_enviar->resultado = 0;
            proceso_enviar->motivo = strdup("Ocurrio un error a la hora de crear el archivo.");
            proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

            io_args->entrada_salida->ocupado = 0;
            io_args->entrada_salida->pid = 0;

            serializar_t_kernel_cpu_io_fs_create(&paquete, proceso_enviar);
            enviar_paquete(paquete, io_args->args->kernel->sockets.cpu_dispatch);
            eliminar_paquete(paquete);
            free(proceso_enviar->motivo);
            free(proceso_enviar);
        }
        free(delete);
        break;
    }
    case ENTRADA_SALIDA_KERNEL_IO_FS_WRITE:
    {
        t_kernel_entrada_salida_fs_write *write = deserializar_t_kernel_entrada_salida_fs_write(buffer);

        // Verifico si este proceso no ha ya sido marcado como eliminado  en kernel
        if (kernel_verificar_proceso_en_exit(io_args->args, write->pid))
        {
            break;
        };

        // El 0 es para completado
        // 1 es para archivo que no existe
        // 2 es para archivo  fuera del maximum size del archivo
        // 3 es para archivo que no tiene espacio suficiente
        if (write->resultado == 0)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%s/%d] Se completó la operación de IO_FS_WRITE para el proceso PID <%d>", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, write->pid);
            t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(io_args->args, write->pid);
            if (io == NULL)
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_ERROR, "No se encontro el tipo DIALFS para el proceso PID <%d>", write->pid);
            }
            else
            {
                io_args->entrada_salida->ocupado = 0;
                io_args->entrada_salida->pid = 0;
                kernel_manejar_ready(io_args->args, write->pid, BLOCK_READY);
                kernel_proximo_io_fs(io_args->args, io);
            }
        }
        else
        {
            kernel_manejar_ready(io_args->args, write->pid, BLOCK_READY);
        }
        free(write);
        avisar_planificador(io_args->args);
        break;
    }
    case ENTRADA_SALIDA_KERNEL_IO_FS_READ:
    {
        t_entrada_salida_fs_read_kernel *read = deserializar_t_entrada_salida_fs_read_kernel(buffer);

        if (kernel_verificar_proceso_en_exit(io_args->args, read->pid))
        {
            break;
        };

        if (read->resultado == 0)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%s/%d] Se completó la operación de IO_FS_READ para el proceso PID <%d>", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, read->pid);
            t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(io_args->args, read->pid);
            if (io == NULL)
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_ERROR, "No se encontro el tipo DIALFS para el proceso PID <%d>", read->pid);
            }
            else
            {
                io_args->entrada_salida->ocupado = 0;
                io_args->entrada_salida->pid = 0;
                kernel_manejar_ready(io_args->args, read->pid, BLOCK_READY);
                // TODO: Enviar a memoria dato para ser guardado
                kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "Se debe guardar en espacio de usuario el dato: %s en la direccion fisica %d", read->dato, read->direccion_fisica);
                kernel_proximo_io_fs(io_args->args, io);
            }
        }
        else
        {
            kernel_manejar_ready(io_args->args, read->pid, BLOCK_READY);
        }
        free(read->dato);
        free(read);
        avisar_planificador(io_args->args);
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