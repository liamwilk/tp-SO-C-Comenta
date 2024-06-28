#include <utils/entradasalida.h>

void switch_case_kernel_dialfs(t_io *args, t_op_code codigo_operacion, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO:
    {
        log_error(args->logger, "Kernel rechazó la identificación de Entrada/Salida porque ya se encontraba conectada.");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    case KERNEL_IO_INTERRUPCION:
    {
        t_kernel_io_interrupcion *interrupcion = deserializar_t_kernel_io_interrupcion(buffer);
        log_warning(args->logger, "[KERNEL/INTERRUPCION/DIALFS] Se recibio una interrupcion con motivo: %s para el PID %d", interrupcion->motivo, interrupcion->pid);
        break;
    }
    case KERNEL_ENTRADA_SALIDA_IO_FS_CREATE:
    {
        t_entrada_salida_fs_create *create = deserializar_t_entrada_salida_fs_create(buffer);
        log_info(args->logger, "PID: <%d> - Crear Archivo: <%s>", create->pid, create->nombre_archivo);

        t_entrada_salida_fs_create *proceso = malloc(sizeof(t_entrada_salida_fs_create));

        // Verifico que haya un bloque libre

        int posicion = fs_buscar_bloque_libre(args);

        if (posicion == -1)
        {
            log_error(args->logger, "No hay bloques libres para crear el archivo");

            proceso->pid = create->pid;
            proceso->resultado = 0;
            proceso->nombre_archivo = strdup(create->nombre_archivo);
            proceso->interfaz = strdup(create->interfaz);
            proceso->size_interfaz = strlen(create->interfaz) + 1;
            proceso->size_nombre_archivo = strlen(create->nombre_archivo) + 1;

            // Envio la respuesta al Kernel
            t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_CREATE);
            serializar_t_entrada_salida_fs_create(&paquete, proceso);

            enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);

            eliminar_paquete(paquete);

            free(proceso->nombre_archivo);
            free(proceso->interfaz);
            free(proceso);

            free(create->nombre_archivo);
            free(create->interfaz);
            free(create);
            break;
        }
        // Verificamos que el archivo no exista
        t_fcb *archivo = dictionary_get(args->dial_fs.archivos, create->nombre_archivo);

        if (archivo != NULL)
        {
            log_error(args->logger, "El archivo ya existe");
            proceso->pid = create->pid;
            proceso->resultado = 0;
            proceso->nombre_archivo = strdup(create->nombre_archivo);
            proceso->interfaz = strdup(create->interfaz);
            proceso->size_interfaz = strlen(create->interfaz) + 1;
            proceso->size_nombre_archivo = strlen(create->nombre_archivo) + 1;

            // Envio la respuesta al Kernel
            t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_CREATE);
            serializar_t_entrada_salida_fs_create(&paquete, proceso);

            enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);

            eliminar_paquete(paquete);

            free(proceso->nombre_archivo);
            free(proceso->interfaz);
            free(proceso);

            free(create->nombre_archivo);
            free(create->interfaz);
            free(create);
            break;
        }

        //  Creo el archivo
        fs_archivo_crear(args, create->nombre_archivo, posicion);

        // Envio la respuesta al Kernel

        proceso->pid = create->pid;
        proceso->resultado = 1; // Se actualiza el resultado
        proceso->nombre_archivo = strdup(create->nombre_archivo);
        proceso->interfaz = strdup(create->interfaz);
        proceso->size_interfaz = strlen(create->interfaz) + 1;
        proceso->size_nombre_archivo = strlen(create->nombre_archivo) + 1;

        // Envio la respuesta al Kernel
        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_CREATE);
        serializar_t_entrada_salida_fs_create(&paquete, proceso);

        enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);

        eliminar_paquete(paquete);

        free(proceso->nombre_archivo);
        free(proceso->interfaz);
        free(proceso);

        free(create->nombre_archivo);
        free(create->interfaz);
        free(create);
        break;
    }
    case FINALIZAR_SISTEMA:
    {
        log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
        pthread_cancel(args->threads.thread_atender_kernel_dialfs);
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    default:
    {
        log_warning(args->logger, "Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    }
}
