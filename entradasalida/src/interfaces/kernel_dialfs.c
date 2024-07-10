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

        log_info(args->logger, "PID: <%d> - Operacion: <IO_FS_CREATE>", create->pid);
        log_info(args->logger, "PID: <%d> - Crear Archivo: <%s>", create->pid, create->nombre_archivo);

        t_entrada_salida_fs_create *proceso = malloc(sizeof(t_entrada_salida_fs_create));

        // Unidades de trabajo emulando una IO
        fs_consumir_unidad_trabajo(args);

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

        // Verificamos que el archivo  exista
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

        // Se marca el bitmap en ocupado
        bitarray_set_bit(args->dial_fs.bitarray, posicion);

        //  Envio la respuesta a Kernel
        proceso->pid = create->pid;
        proceso->resultado = 1; // Se actualiza el resultado
        proceso->nombre_archivo = strdup(create->nombre_archivo);
        proceso->interfaz = strdup(create->interfaz);
        proceso->size_interfaz = strlen(create->interfaz) + 1;
        proceso->size_nombre_archivo = strlen(create->nombre_archivo) + 1;

        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_CREATE);
        serializar_t_entrada_salida_fs_create(&paquete, proceso);
        enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);

        log_debug(args->logger, "Se envio la respuesta al Kernel en el socket %d", args->sockets.socket_kernel_dialfs);
        eliminar_paquete(paquete);

        // Libero Memoria
        free(proceso->nombre_archivo);
        free(proceso->interfaz);
        free(proceso);

        free(create->nombre_archivo);
        free(create->interfaz);
        free(create);
        break;
    }
    case KERNEL_ENTRADA_SALIDA_IO_FS_TRUNCATE:
    {
        t_kernel_entrada_salida_fs_truncate *truncate = deserializar_t_kernel_entrada_salida_fs_truncate(buffer);

        t_kernel_entrada_salida_fs_truncate *proceso = malloc(sizeof(t_kernel_entrada_salida_fs_truncate));

        /**LOGS OBLIGATORIOS**/
        log_info(args->logger, "PID: <%d> - Operacion: <IO_FS_TRUNCATE>", truncate->pid);
        log_info(args->logger, "PID : <%d> - Truncar Archivo : <%s> - Tamaño : <%d>", truncate->pid, truncate->nombre_archivo, truncate->tamanio_a_truncar);

        // Unidades de trabajo emulando una IO
        fs_consumir_unidad_trabajo(args);

        // Obtenemos el archivo a truncar del diccionario
        t_fcb *archivo = dictionary_get(args->dial_fs.archivos, truncate->nombre_archivo);

        // Verifico que no sea NULO
        if (archivo == NULL)
        {
            log_error(args->logger, "El archivo no existe");
            proceso->pid = truncate->pid;
            proceso->resultado = 0;
            proceso->nombre_archivo = strdup(truncate->nombre_archivo);
            proceso->tamanio_a_truncar = truncate->tamanio_a_truncar;
            proceso->size_nombre_archivo = strlen(truncate->nombre_archivo) + 1;
            proceso->interfaz = strdup(truncate->interfaz);
            proceso->size_interfaz = strlen(truncate->interfaz) + 1;
            t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_TRUNCATE);
            serializar_t_kernel_entrada_salida_fs_truncate(&paquete, proceso);
            enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);
            eliminar_paquete(paquete);
            free(proceso->nombre_archivo);
            free(proceso->interfaz);
            free(proceso);
            free(truncate->nombre_archivo);
            free(truncate->interfaz);
            free(truncate);
            break;
        }

        // Obtengo cuanto sera el tamaño del archivo luego de truncar
        int size_original = archivo->total_size;
        int tamanio_post_truncate = archivo->total_size + truncate->tamanio_a_truncar;

        // Verifico si se quiere truncar un archivo vacio y ademas el tamaño a truncar es menor al tamaño de un bloque
        if (size_original == 0 && (tamanio_post_truncate < args->dial_fs.blockSize))
        {
            archivo->total_size = tamanio_post_truncate;
            config_set_value(archivo->metadata, "TAMANIO_ARCHIVO", string_itoa(archivo->total_size));
            config_save(archivo->metadata);
            proceso->pid = truncate->pid;
            proceso->resultado = 1;
            proceso->nombre_archivo = strdup(truncate->nombre_archivo);
            proceso->tamanio_a_truncar = truncate->tamanio_a_truncar;
            proceso->size_nombre_archivo = strlen(truncate->nombre_archivo) + 1;
            proceso->interfaz = strdup(truncate->interfaz);
            proceso->size_interfaz = strlen(truncate->interfaz) + 1;
            t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_TRUNCATE);
            serializar_t_kernel_entrada_salida_fs_truncate(&paquete, proceso);
            enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);
            eliminar_paquete(paquete);
            free(proceso->nombre_archivo);
            free(proceso->interfaz);
            free(proceso);
            free(truncate->nombre_archivo);
            free(truncate->interfaz);
            free(truncate);
            break;
        }

        // Verifico si se quiere truncar un archivo vacio y ademas el tamaño a truncar es mayor al tamaño de un bloque
        if (size_original == 0)
        {
            // El tamaño a truncar  sera entonces n - blockSize
            truncate->tamanio_a_truncar = truncate->tamanio_a_truncar - args->dial_fs.blockSize;
        }

        int cantidad_bloques_a_truncar = (int)(truncate->tamanio_a_truncar / args->dial_fs.blockSize);
        log_debug(args->logger, "Cantidad de bloques a truncar: %d", cantidad_bloques_a_truncar);

        // Contar cantidad de bloques ocupados con bitarray
        int bloques_ocupados = fs_bloques_ocupados(args);
        log_debug(args->logger, "Cantidad de bloques ocupados: %d", bloques_ocupados);

        // Se calcula la cantidad de bloques ocupados teniendo en cuenta el truncado
        int total = cantidad_bloques_a_truncar + bloques_ocupados;
        log_debug(args->logger, "Cantidad de bloques ocupados luego de truncar: %d", total);

        if (total > args->dial_fs.blockCount)
        {
            log_error(args->logger, "No hay suficientes bloques para truncar el archivo");
            proceso->pid = truncate->pid;
            proceso->resultado = 0;
            proceso->nombre_archivo = strdup(truncate->nombre_archivo);
            proceso->tamanio_a_truncar = truncate->tamanio_a_truncar;
            proceso->size_nombre_archivo = strlen(truncate->nombre_archivo) + 1;
            proceso->interfaz = strdup(truncate->interfaz);
            proceso->size_interfaz = strlen(truncate->interfaz) + 1;
            t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_TRUNCATE);
            serializar_t_kernel_entrada_salida_fs_truncate(&paquete, proceso);
            enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);
            eliminar_paquete(paquete);
            free(proceso->nombre_archivo);
            free(proceso->interfaz);
            free(proceso);
            free(truncate->nombre_archivo);
            free(truncate->interfaz);
            free(truncate);
            break;
        }

        int fin_bloque_tras_truncar = archivo->inicio + cantidad_bloques_a_truncar;

        // Recorremos el diccionario para verificar que no hay que compactar (no se dispone de bloques contiguos)
        bool compactar = fs_tiene_compactar(args, archivo, truncate->nombre_archivo, cantidad_bloques_a_truncar);

        if (compactar)
        {
            fs_compactar(args, truncate, archivo, cantidad_bloques_a_truncar);
        }

        log_debug(args->logger, "El archivo %s dispone de %d bloques contiguos, se procede a truncar para el PID %d", truncate->nombre_archivo, cantidad_bloques_a_truncar, truncate->pid);

        // Se procede a truncarlo
        // Recorremos el bitmap desde fin_bloque anterior
        for (int i = archivo->fin_bloque + 1; i <= archivo->fin_bloque + cantidad_bloques_a_truncar; i++)
        {
            log_debug(args->logger, "[%s]: Se setea el bit %d en 1", truncate->nombre_archivo, i);
            bitarray_set_bit(args->dial_fs.bitarray, i);
        }

        // Actualizamos total_size y fin_bloque del archivo
        archivo->total_size = tamanio_post_truncate;

        log_debug(args->logger, "Tamaño total de %s es : %d bytes", truncate->nombre_archivo, archivo->total_size);
        archivo->fin_bloque = fin_bloque_tras_truncar;
        log_debug(args->logger, "Fin bloque del archivo %s: %d", truncate->nombre_archivo, archivo->fin_bloque);

        // Actualizar el config y guardar en archivo
        config_set_value(archivo->metadata, "TAMANIO_ARCHIVO", string_itoa(archivo->total_size));
        config_save(archivo->metadata);

        proceso->pid = truncate->pid;
        proceso->resultado = 1;
        proceso->nombre_archivo = strdup(truncate->nombre_archivo);
        proceso->tamanio_a_truncar = truncate->tamanio_a_truncar;
        proceso->size_nombre_archivo = strlen(truncate->nombre_archivo) + 1;
        proceso->interfaz = strdup(truncate->interfaz);
        proceso->size_interfaz = strlen(truncate->interfaz) + 1;
        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_TRUNCATE);
        serializar_t_kernel_entrada_salida_fs_truncate(&paquete, proceso);
        enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);
        eliminar_paquete(paquete);
        free(proceso->nombre_archivo);
        free(proceso->interfaz);
        free(proceso);
        free(truncate->nombre_archivo);
        free(truncate->interfaz);
        free(truncate);
        break;
    }
    case FINALIZAR_SISTEMA:
    {
        log_info(args->logger, "Se recibio la señal de desconexión de Kernel. Cierro hilo");
        pthread_cancel(args->threads.thread_atender_kernel_dialfs);
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    case KERNEL_ENTRADA_SALIDA_IO_FS_DELETE:
    {
        // Uso las funciones de serializacion de create para no repetir la misma logica
        t_entrada_salida_fs_create *delete = deserializar_t_entrada_salida_fs_create(buffer);
        log_info(args->logger, "PID: <%d> - Operacion: <IO_FS_DELETE>", delete->pid);
        log_info(args->logger, "PID: <%d> - Eliminar Archivo: <%s>", delete->pid, delete->nombre_archivo);

        // Unidades de trabajo emulando una IO
        fs_consumir_unidad_trabajo(args);

        t_fcb *archivo = dictionary_get(args->dial_fs.archivos, delete->nombre_archivo);

        if (archivo == NULL)
        {
            log_error(args->logger, "El archivo no existe");
            break;
        }

        // Liberamos el bitmap desde bloque inicio hasta bloque fin
        for (int i = archivo->inicio; i <= archivo->fin_bloque; i++)
        {
            log_debug(args->logger, "[%s]:Se setea el bit %d en 0", delete->nombre_archivo, i);
            bitarray_clean_bit(args->dial_fs.bitarray, i);
        }

        // Eliminar el archivo del fs
        remove(archivo->metadata->path);
        t_fcb *fcb_eliminada = dictionary_remove(args->dial_fs.archivos, delete->nombre_archivo);
        free(fcb_eliminada->metadata);
        free(fcb_eliminada);

        //  Envio la respuesta al Kernel
        t_entrada_salida_fs_create *proceso = malloc(sizeof(t_entrada_salida_fs_create));
        proceso->pid = delete->pid;
        proceso->resultado = 1; // Se actualiza el resultado
        proceso->nombre_archivo = strdup(delete->nombre_archivo);
        proceso->interfaz = strdup(delete->interfaz);
        proceso->size_interfaz = strlen(delete->interfaz) + 1;
        proceso->size_nombre_archivo = strlen(delete->nombre_archivo) + 1;

        // Envio la respuesta al Kernel
        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_DELETE);
        serializar_t_entrada_salida_fs_create(&paquete, proceso);
        enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);

        log_debug(args->logger, "Se envio la respuesta al Kernel en el socket %d", args->sockets.socket_kernel_dialfs);
        eliminar_paquete(paquete);

        free(proceso->nombre_archivo);
        free(proceso->interfaz);
        free(proceso);

        free(delete->nombre_archivo);
        free(delete->interfaz);
        free(delete);
        break;
    }
    case KERNEL_ENTRADA_SALIDA_IO_FS_WRITE:
    {
        t_kernel_entrada_salida_fs_write *write = deserializar_t_kernel_entrada_salida_fs_write(buffer);
        log_info(args->logger, "PID: <%d> - Operacion: <IO_FS_WRITE>", write->pid);

        log_info(args->logger, "PID: <%d> - Escribir Archivo: <%s> - Tamaño a Escribir: <%ld> - Puntero Archivo: <%d>", write->pid, write->nombre_archivo, strlen(write->escribir) + 1, write->puntero_archivo);
        log_debug(args->logger, "Contenido a escribir: %s", write->escribir);

        // Calculamos cuantos bloques ocupara el archivo
        int cantidad_bloques = (int)((strlen(write->escribir) + 1) / args->dial_fs.blockSize);
        log_debug(args->logger, "Cantidad de bloques a escribir: %d", cantidad_bloques);

        // Unidades de trabajo emulando una IO
        fs_consumir_unidad_trabajo(args);

        // Obtenemos el archivo a escribir del diccionario
        t_fcb *archivo = dictionary_get(args->dial_fs.archivos, write->nombre_archivo);
        if (archivo == NULL)
        {
            log_error(args->logger, "El archivo no existe");
            write->resultado = 1; // 1 es para archivo que no existe
            break;
        }

        // Se crea un puntero absoluto, Si por ejemplo me dicen escribir en el byte 100 del archivo, el puntero absoluto seria 100 + (inicio_bloque * blocksize)
        uint32_t puntero_absoluto = write->puntero_archivo + (archivo->inicio * args->dial_fs.blockSize);

        // Comprobamos que al escribir no se pase del tamaño del archivo
        if (write->puntero_archivo + cantidad_bloques > archivo->fin_bloque)
        {
            log_error(args->logger, "Se intenta escribir fuera del maximum size del archivo");
            write->resultado = 2; // 2 es para archivo  fuera del maximum size del archivo
            break;
        }
        log_debug(args->logger, "Puntero absoluto en bloques.dat: %d", puntero_absoluto);

        // Escribimos en el archivo, para ello usamos la variable mapeada en memoria el valor de write->escribir
        memcpy((char *)args->dial_fs.archivo_bloques + puntero_absoluto, write->escribir, strlen(write->escribir) + 1);

        write->resultado = 0; // 0 es para archivo que se escribio correctamente
        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_WRITE);
        serializar_t_kernel_entrada_salida_fs_write(&paquete, write);
        enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);
        eliminar_paquete(paquete);
        free(write->nombre_archivo);
        free(write->escribir);
        free(write);
        break;
    }
    case KERNEL_ENTRADA_SALIDA_IO_FS_READ:
    {
        t_kernel_entrada_salida_fs_read *read = deserializar_t_kernel_entrada_salida_fs_read(buffer);

        /**LOGS OBLIGATORIOS**/
        log_info(args->logger, "PID: <%d> - Operacion: <IO_FS_READ>", read->pid);
        log_info(args->logger, "PID: <%d> - Leer Archivo: <%s> - Tamaño a Leer: <%d> - Puntero Archivo: <%d>", read->pid, read->nombre_archivo, read->registro_tamanio, read->puntero_archivo);

        /**OBTENEMOS LA DATA DEL ARCHVO**/
        t_fcb *archivo = dictionary_get(args->dial_fs.archivos, read->nombre_archivo);
        if (archivo == NULL)
        {
            log_error(args->logger, "El archivo no existe");
            break;
        }

        // Unidades de trabajo emulando una IO
        fs_consumir_unidad_trabajo(args);

        // Se crea un puntero absoluto, Si por ejemplo me dicen escribir en el byte 100 del archivo, el puntero absoluto seria 100 + (inicio_bloque * blocksize)
        uint32_t puntero_absoluto = read->puntero_archivo + (archivo->inicio * args->dial_fs.blockSize);
        log_debug(args->logger, "Puntero absoluto en bloques.dat: %d", puntero_absoluto);

        // Almacenamos el contenido en una variable
        char *contenido = malloc(read->registro_tamanio + 1);
        memcpy(contenido, (char *)args->dial_fs.archivo_bloques + puntero_absoluto, read->registro_tamanio + 1);
        log_debug(args->logger, "Contenido que se leyo: %s", contenido);

        // Enviamos a kernel
        t_entrada_salida_fs_read_kernel *proceso = malloc(sizeof(t_entrada_salida_fs_read_kernel));
        proceso->pid = read->pid;
        proceso->resultado = 0;
        proceso->nombre_archivo = strdup(read->nombre_archivo);
        proceso->registro_tamanio = read->registro_tamanio;
        proceso->puntero_archivo = read->puntero_archivo;
        proceso->dato = contenido;
        proceso->size_dato = strlen(contenido) + 1;
        proceso->size_nombre_archivo = strlen(read->nombre_archivo) + 1;
        proceso->direccion_fisica = read->direccion_fisica;
        proceso->marco = read->marco;
        proceso->desplazamiento = read->desplazamiento;
        proceso->numero_pagina = read->numero_pagina;
        proceso->interfaz = strdup(read->interfaz);
        proceso->size_interfaz = strlen(read->interfaz) + 1;
        proceso->registro_direccion = read->registro_direccion;

        // Crear el paquete
        t_paquete *paquete = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_FS_READ);
        serializar_t_entrada_salida_fs_read_kernel(&paquete, proceso);
        enviar_paquete(paquete, args->sockets.socket_kernel_dialfs);
        eliminar_paquete(paquete);

        free(proceso->nombre_archivo);
        free(proceso->interfaz);
        free(proceso);
        free(contenido);
        free(read->nombre_archivo);
        free(read);
        break;
    }
    default:
    {
        liberar_conexion(&args->sockets.socket_kernel_dialfs);
        break;
    }
    }
}
