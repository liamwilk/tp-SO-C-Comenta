#include "entradasalida.h"

void hilo_ejecutar_interfaz(t_io *args, int *socket, char *modulo, t_io_funcion_hilo_ptr switch_case_atencion)
{
    while (1)
    {
        pthread_testcancel();

        log_debug(args->logger, "Esperando paquete de %s en socket %d", modulo, *socket);

        t_paquete *paquete = recibir_paquete(args->logger, socket);

        if (paquete == NULL)
        {
            log_info(args->logger, "%s se desconecto.", modulo);
            break;
        }

        revisar_paquete(paquete, args->logger, modulo);

        switch_case_atencion(args, paquete->codigo_operacion, paquete->buffer);

        eliminar_paquete(paquete);
    }
}

void interfaz_generic(t_io *io)
{
    interfaz_generic_inicializar_config(io);
    interfaz_generic_imprimir_log(io);

    pthread_create(&io->threads.thread_conectar_kernel_generic, NULL, conectar_kernel_generic, io);
    pthread_join(io->threads.thread_conectar_kernel_generic, NULL);

    pthread_create(&io->threads.thread_atender_kernel_generic, NULL, atender_kernel_generic, io);
    pthread_join(io->threads.thread_atender_kernel_generic, NULL);
}

void interfaz_identificar(t_op_code opcode, char *identificador, int socket)
{
    t_paquete *paquete = crear_paquete(opcode);
    t_entrada_salida_identificacion identificacion;
    identificacion.identificador = strdup(identificador);
    identificacion.size_identificador = strlen(identificador) + 1;
    serializar_t_entrada_salida_identificacion(&paquete, &identificacion);
    enviar_paquete(paquete, socket);

    free(identificacion.identificador);
    eliminar_paquete(paquete);
}

void inicializar_interfaz(t_io *io)
{
    t_interfaz tipo = determinar_interfaz(io->config);

    switch (tipo)
    {
    case GEN:
        interfaz_generic(io);
        break;
    case STDIN:
        interfaz_stdin(io);
        break;
    case STDOUT:
        interfaz_stdout(io);
        break;
    case DIALFS:
        interfaz_dialfs(io);
        break;
    default:
        log_error(io->logger, "Se introdujo por parametro un tipo de interfaz desconocida. ");
        break;
    }
}

void *conectar_kernel_generic(void *args_void)
{
    t_io *args = (t_io *)args_void;
    conexion_crear(args->logger, args->ipKernel, args->puertoKernel, &args->sockets.socket_kernel_generic, "Kernel", KERNEL_ENTRADA_SALIDA_GENERIC);
    pthread_exit(0);
}

t_interfaz determinar_interfaz(t_config *config)
{
    char *tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    if (!strcmp(tipoInterfaz, "GENERICA"))
    {
        return GEN;
    }
    if (!strcmp(tipoInterfaz, "STDOUT"))
    {
        return STDOUT;
    }
    if (!strcmp(tipoInterfaz, "STDIN"))
    {
        return STDIN;
    }
    if (!strcmp(tipoInterfaz, "DIALFS"))
    {
        return DIALFS;
    }
    free(tipoInterfaz);
    return -1;
};

void interfaz_generic_inicializar_config(t_io *io)
{
    io->tipoInterfaz = config_get_string_value(io->config, "TIPO_INTERFAZ");
    io->ipKernel = config_get_string_value(io->config, "IP_KERNEL");
    io->puertoKernel = config_get_int_value(io->config, "PUERTO_KERNEL");
    io->tiempoUnidadDeTrabajo = config_get_int_value(io->config, "TIEMPO_UNIDAD_TRABAJO");
}

void interfaz_stdin_inicializar_config(t_io *io)
{
    io->tipoInterfaz = config_get_string_value(io->config, "TIPO_INTERFAZ");
    io->ipKernel = config_get_string_value(io->config, "IP_KERNEL");
    io->puertoKernel = config_get_int_value(io->config, "PUERTO_KERNEL");
    io->ipMemoria = config_get_string_value(io->config, "IP_MEMORIA");
    io->puertoMemoria = config_get_int_value(io->config, "PUERTO_MEMORIA");
}

void interfaz_stdout_inicializar_config(t_io *io)
{
    io->tipoInterfaz = config_get_string_value(io->config, "TIPO_INTERFAZ");
    io->tiempoUnidadDeTrabajo = config_get_int_value(io->config, "TIEMPO_UNIDAD_TRABAJO");
    io->ipKernel = config_get_string_value(io->config, "IP_KERNEL");
    io->puertoKernel = config_get_int_value(io->config, "PUERTO_KERNEL");
    io->ipMemoria = config_get_string_value(io->config, "IP_MEMORIA");
    io->puertoMemoria = config_get_int_value(io->config, "PUERTO_MEMORIA");
}

void interfaz_dialfs_inicializar_config(t_io *io)
{
    io->tipoInterfaz = config_get_string_value(io->config, "TIPO_INTERFAZ");
    io->tiempoUnidadDeTrabajo = config_get_int_value(io->config, "TIEMPO_UNIDAD_TRABAJO");
    io->ipKernel = config_get_string_value(io->config, "IP_KERNEL");
    io->puertoKernel = config_get_int_value(io->config, "PUERTO_KERNEL");
    io->ipMemoria = config_get_string_value(io->config, "IP_MEMORIA");
    io->puertoMemoria = config_get_int_value(io->config, "PUERTO_MEMORIA");
    io->dial_fs.pathBaseDialFs = config_get_string_value(io->config, "PATH_BASE_DIALFS");
    io->dial_fs.blockCount = config_get_int_value(io->config, "BLOCK_COUNT");
    io->dial_fs.blockSize = config_get_int_value(io->config, "BLOCK_SIZE");
    io->dial_fs.retrasoCompactacion = config_get_int_value(io->config, "RETRASO_COMPACTACION");
}

void interfaz_generic_imprimir_log(t_io *io)
{
    log_info(io->logger, "TIPO_INTERFAZ: %s", io->tipoInterfaz);
    log_info(io->logger, "TIEMPO_UNIDAD_TRABAJO: %d", io->tiempoUnidadDeTrabajo);
    log_info(io->logger, "IP_KERNEL: %s", io->ipKernel);
    log_info(io->logger, "PUERTO_KERNEL: %d", io->puertoKernel);
}

void interfaz_stdin_imprimir_log(t_io *io)
{
    log_info(io->logger, "TIPO_INTERFAZ: %s", io->tipoInterfaz);
    log_info(io->logger, "IP_KERNEL: %s", io->ipKernel);
    log_info(io->logger, "PUERTO_KERNEL: %d", io->puertoKernel);
    log_info(io->logger, "IP_MEMORIA: %s", io->ipMemoria);
    log_info(io->logger, "PUERTO_MEMORIA: %d", io->puertoMemoria);
}

void interfaz_stdout_imprimir_log(t_io *io)
{
    log_info(io->logger, "TIPO_INTERFAZ: %s", io->tipoInterfaz);
    log_info(io->logger, "TIEMPO_UNIDAD_TRABAJO: %d", io->tiempoUnidadDeTrabajo);
    log_info(io->logger, "IP_KERNEL: %s", io->ipKernel);
    log_info(io->logger, "PUERTO_KERNEL: %d", io->puertoKernel);
    log_info(io->logger, "IP_MEMORIA: %s", io->ipMemoria);
    log_info(io->logger, "PUERTO_MEMORIA: %d", io->puertoMemoria);
}

void interfaz_dialfs_imprimir_log(t_io *io)
{
    log_info(io->logger, "TIPO_INTERFAZ: %s", io->tipoInterfaz);
    log_info(io->logger, "TIEMPO_UNIDAD_TRABAJO: %d", io->tiempoUnidadDeTrabajo);
    log_info(io->logger, "IP_KERNEL: %s", io->ipKernel);
    log_info(io->logger, "PUERTO_KERNEL: %d", io->puertoKernel);
    log_info(io->logger, "IP_MEMORIA: %s", io->ipMemoria);
    log_info(io->logger, "PUERTO_MEMORIA: %d", io->puertoMemoria);
    log_info(io->logger, "PATH_BASE_DIALFS: %s", io->dial_fs.pathBaseDialFs);
    log_info(io->logger, "BLOCK_SIZE: %d", io->dial_fs.blockSize);
    log_info(io->logger, "BLOCK_COUNT: %d", io->dial_fs.blockCount);
    log_info(io->logger, "RETRASO_COMPACTACION: %d", io->dial_fs.retrasoCompactacion);
}

void interfaz_stdin(t_io *args)
{
    interfaz_stdin_inicializar_config(args);
    interfaz_stdin_imprimir_log(args);

    pthread_create(&args->threads.thread_conectar_memoria_stdin, NULL, conectar_memoria_stdin, args);
    pthread_join(args->threads.thread_conectar_memoria_stdin, NULL);

    pthread_create(&args->threads.thread_atender_memoria_stdin, NULL, atender_memoria_stdin, args);
    pthread_detach(args->threads.thread_atender_memoria_stdin);

    pthread_create(&args->threads.thread_conectar_kernel_stdin, NULL, conectar_kernel_stdin, args);
    pthread_join(args->threads.thread_conectar_kernel_stdin, NULL);

    pthread_create(&args->threads.thread_atender_kernel_stdin, NULL, atender_kernel_stdin, args);
    pthread_join(args->threads.thread_atender_kernel_stdin, NULL);
}

void interfaz_stdout(t_io *args)
{
    interfaz_stdout_inicializar_config(args);
    interfaz_stdout_imprimir_log(args);

    pthread_create(&args->threads.thread_conectar_memoria_stdout, NULL, conectar_memoria_stdout, args);
    pthread_join(args->threads.thread_conectar_memoria_stdout, NULL);

    pthread_create(&args->threads.thread_atender_memoria_stdout, NULL, atender_memoria_stdout, args);
    pthread_detach(args->threads.thread_atender_memoria_stdout);

    pthread_create(&args->threads.thread_conectar_kernel_stdout, NULL, conectar_kernel_stdout, args);
    pthread_join(args->threads.thread_conectar_kernel_stdout, NULL);

    pthread_create(&args->threads.thread_atender_kernel_stdout, NULL, atender_kernel_stdout, args);
    pthread_join(args->threads.thread_atender_kernel_stdout, NULL);
}

void interfaz_dialfs(t_io *args)
{
    interfaz_dialfs_inicializar_config(args);
    interfaz_dialfs_imprimir_log(args);

    bloques_mapear(args);
    bitmap_mapear(args);
    metadata_inicializar(args);
    pthread_create(&args->threads.thread_conectar_memoria_dialfs, NULL, conectar_memoria_dialfs, args);
    pthread_join(args->threads.thread_conectar_memoria_dialfs, NULL);

    pthread_create(&args->threads.thread_atender_memoria_dialfs, NULL, atender_memoria_dialfs, args);
    pthread_detach(args->threads.thread_atender_memoria_dialfs);

    pthread_create(&args->threads.thread_conectar_kernel_dialfs, NULL, conectar_kernel_dialfs, args);
    pthread_join(args->threads.thread_conectar_kernel_dialfs, NULL);

    pthread_create(&args->threads.thread_atender_kernel_dialfs, NULL, atender_kernel_dialfs, args);
    pthread_join(args->threads.thread_atender_kernel_dialfs, NULL);
}

void *conectar_memoria_stdin(void *args_void)
{
    t_io *args = (t_io *)args_void;
    conexion_crear(args->logger, args->ipMemoria, args->puertoMemoria, &args->sockets.socket_memoria_stdin, "Memoria", MEMORIA_ENTRADA_SALIDA_STDIN);
    pthread_exit(0);
}

int verificar_argumentos(int argc)
{
    if (argc == 1)
    {
        printf("-----------------------------------------------------------------------------------\n");
        printf("Error: No se ha ingresado el identificador ni path al archivo de configuración.\n");
        printf("Ejemplo desde carpeta entradasalida: ./bin/entradasalida \"Int1\" config/gen.config\n");
        printf("-----------------------------------------------------------------------------------\n");
        return EXIT_FAILURE;
    }
    if (argc == 2)
    {
        printf("----------------------------------------------------------------------------------\n");
        printf("Error: No se ha ingresado el path al archivo de configuración.\n");
        printf("Ejemplo desde carpeta entradasalida: ./bin/entradasalida \"Int1\" config/gen.config\n");
        printf("----------------------------------------------------------------------------------\n");
        return EXIT_FAILURE;
    }
    if (argc > 3)
    {
        printf("----------------------------------------------------------------------------------\n");
        printf("Error: Se han ingresado demasiados argumentos.\n");
        printf("Ejemplo desde carpeta entradasalida: ./bin/entradasalida \"Int1\" config/gen.config\n");
        printf("----------------------------------------------------------------------------------\n");
        return EXIT_FAILURE;
    }
    return 0;
}

void inicializar_argumentos(t_io *args, char *argv[])
{
    // args->identificador = "TECLADO";
    args->identificador = strdup(argv[1]);
    args->logger = iniciar_logger(args->identificador, LOG_LEVEL_DEBUG);
    args->config = crear_config_parametro(args->logger, argv[2]);
    // args->config = crear_config_parametro(args->logger, "config/io/teclado.config");
}

void *conectar_kernel_stdin(void *args_void)
{
    t_io *args = (t_io *)args_void;
    conexion_crear(args->logger, args->ipKernel, args->puertoKernel, &args->sockets.socket_kernel_stdin, "Kernel", KERNEL_ENTRADA_SALIDA_STDIN);
    pthread_exit(0);
}

void *conectar_memoria_stdout(void *args_void)
{
    t_io *args = (t_io *)args_void;
    conexion_crear(args->logger, args->ipMemoria, args->puertoMemoria, &args->sockets.socket_memoria_stdout, "Memoria", MEMORIA_ENTRADA_SALIDA_STDOUT);
    pthread_exit(0);
}

void *conectar_kernel_stdout(void *args_void)
{
    t_io *args = (t_io *)args_void;
    conexion_crear(args->logger, args->ipKernel, args->puertoKernel, &args->sockets.socket_kernel_stdout, "Kernel", KERNEL_ENTRADA_SALIDA_STDOUT);
    pthread_exit(0);
}

void *conectar_memoria_dialfs(void *args_void)
{
    t_io *args = (t_io *)args_void;
    conexion_crear(args->logger, args->ipMemoria, args->puertoMemoria, &args->sockets.socket_memoria_dialfs, "Memoria", MEMORIA_ENTRADA_SALIDA_DIALFS);
    pthread_exit(0);
}

void *conectar_kernel_dialfs(void *args_void)
{
    t_io *args = (t_io *)args_void;
    conexion_crear(args->logger, args->ipKernel, args->puertoKernel, &args->sockets.socket_kernel_dialfs, "Kernel", KERNEL_ENTRADA_SALIDA_DIALFS);
    pthread_exit(0);
}

void finalizar_interfaz(t_io *args)
{
    if (determinar_interfaz(args->config) == DIALFS)
    {
        bloques_desmapear(args);
        bitmap_desmapear(args);
    }
    log_destroy(args->logger);
    config_destroy(args->config);
}

int inicializar_modulo_interfaz(t_io *args, int argc, char *argv[], timer_args_io_t *temporizador)
{
    // if (verificar_argumentos(argc))
    // {
    //     return 1;
    // }
    temporizador->args = args;
    inicializar_argumentos(args, argv);
    interfaz_inicializar_temporizador(args, temporizador);
    inicializar_interfaz(args);
    finalizar_interfaz(args);
    return 0;
}

void *atender_kernel_generic(void *args_void)
{
    t_io *args = (t_io *)args_void;
    interfaz_identificar(ENTRADA_SALIDA_KERNEL_IDENTIFICACION, args->identificador, args->sockets.socket_kernel_generic);
    hilo_ejecutar_interfaz(args, &args->sockets.socket_kernel_generic, "Kernel Generic", switch_case_kernel_generic);
    pthread_exit(0);
}

void *atender_kernel_dialfs(void *args_void)
{
    t_io *args = (t_io *)args_void;
    interfaz_identificar(ENTRADA_SALIDA_KERNEL_IDENTIFICACION, args->identificador, args->sockets.socket_kernel_dialfs);
    hilo_ejecutar_interfaz(args, &args->sockets.socket_kernel_dialfs, "Kernel DialFS", switch_case_kernel_dialfs);
    pthread_exit(0);
}

void *atender_kernel_stdin(void *args_void)
{
    t_io *args = (t_io *)args_void;
    interfaz_identificar(ENTRADA_SALIDA_KERNEL_IDENTIFICACION, args->identificador, args->sockets.socket_kernel_stdin);
    hilo_ejecutar_interfaz(args, &args->sockets.socket_kernel_stdin, "Kernel STDIN", switch_case_kernel_stdin);
    pthread_exit(0);
}

void *atender_kernel_stdout(void *args_void)
{
    t_io *args = (t_io *)args_void;
    interfaz_identificar(ENTRADA_SALIDA_KERNEL_IDENTIFICACION, args->identificador, args->sockets.socket_kernel_stdout);
    hilo_ejecutar_interfaz(args, &args->sockets.socket_kernel_stdout, "Kernel STDOUT", switch_case_kernel_stdout);
    pthread_exit(0);
}

void *atender_memoria_stdin(void *args_void)
{
    t_io *args = (t_io *)args_void;
    interfaz_identificar(MEMORIA_ENTRADA_SALIDA_IDENTIFICACION, args->identificador, args->sockets.socket_memoria_stdin);
    hilo_ejecutar_interfaz(args, &args->sockets.socket_memoria_stdin, "Memoria STDIN", switch_case_memoria_stdin);
    pthread_exit(0);
}

void *atender_memoria_dialfs(void *args_void)
{
    t_io *args = (t_io *)args_void;
    interfaz_identificar(MEMORIA_ENTRADA_SALIDA_IDENTIFICACION, args->identificador, args->sockets.socket_memoria_dialfs);
    hilo_ejecutar_interfaz(args, &args->sockets.socket_memoria_dialfs, "Memoria DialFS", switch_case_memoria_dialfs);
    pthread_exit(0);
}

void *atender_memoria_stdout(void *args_void)
{
    t_io *args = (t_io *)args_void;
    interfaz_identificar(MEMORIA_ENTRADA_SALIDA_IDENTIFICACION, args->identificador, args->sockets.socket_memoria_stdout);
    hilo_ejecutar_interfaz(args, &args->sockets.socket_memoria_stdout, "Memoria STDOUT", switch_case_memoria_stdout);
    pthread_exit(0);
}

void interfaz_interrumpir_temporizador(t_io *args)
{
    if (timer_delete(args->timer.timer) == -1)
    {
        log_debug(args->logger, "Error al eliminar el temporizador");
        return;
    }
}

void interfaz_iniciar_temporizador(t_io *args, double duracion)
{
    // Creo el temporizador
    timer_create(CLOCK_REALTIME, &args->timer.sev, &args->timer.timer);

    // Convierto la duración de milisegundos a segundos
    double tiempo_total = duracion * args->tiempoUnidadDeTrabajo / 1000.0;

    // Separo el tiempo total en segundos y nanosegundos
    time_t segundos = (time_t)tiempo_total;                              // Parte entera en segundos
    long nanosegundos = (long)((tiempo_total - segundos) * 1000000000L); // Parte decimal convertida a nanosegundos

    // Configura el tiempo de inicio y el intervalo del temporizador
    args->timer.its.it_value.tv_sec = segundos;
    args->timer.its.it_value.tv_nsec = nanosegundos;
    args->timer.its.it_interval.tv_sec = 0;
    args->timer.its.it_interval.tv_nsec = 0;

    // Inicio el temporizador
    if (timer_settime(args->timer.timer, 0, &args->timer.its, NULL) == -1)
    {
        log_debug(args->logger, "Error al iniciar el temporizador");
    }

    // Logueo el tiempo del temporizador
    log_debug(args->logger, "Temporizador iniciado: %ld segundos y %ld nanosegundos", segundos, nanosegundos);
}

void interfaz_inicializar_temporizador(t_io *args, timer_args_io_t *temporizador)
{
    // Configuro la estructura sigevent
    args->timer.sev.sigev_notify = SIGEV_THREAD;
    args->timer.sev.sigev_value.sival_ptr = temporizador;
    args->timer.sev.sigev_notify_function = interfaz_manejador_temporizador;
    args->timer.sev.sigev_notify_attributes = NULL;
}

void interfaz_manejador_temporizador(union sigval arg)
{
    timer_args_io_t *timerArgs = (timer_args_io_t *)arg.sival_ptr;

    t_paquete *aviso_sleep = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP);
    t_entrada_salida_kernel_unidad_de_trabajo *unidad = malloc(sizeof(t_entrada_salida_kernel_unidad_de_trabajo));

    unidad->terminado = true;
    unidad->pid = timerArgs->args->pid;

    serializar_t_entrada_salida_kernel_unidad_de_trabajo(&aviso_sleep, unidad);

    enviar_paquete(aviso_sleep, timerArgs->args->sockets.socket_kernel_generic);

    log_info(timerArgs->args->logger, "Se notifica finalizacion de instruccion <IO_GEN_SLEEP> <%s> <%d> del PID <%d>", timerArgs->args->identificador, timerArgs->args->unidades, timerArgs->args->pid);

    eliminar_paquete(aviso_sleep);
    free(unidad);
}

char *leer_input_usuario(uint32_t size_input)
{
    char *input = NULL;
    printf("Introduce un texto (maximo <%d> bytes):\n", size_input);

    while (1)
    {
        input = readline("Dato a guardar en Memoria > ");

        if (input == NULL)
        {
            fprintf(stderr, "Error al leer el input\n");
            return NULL;
        }

        if (strlen(input) == 0)
        {
            printf("Error: No se ha introducido ningún texto. Intentalo de nuevo.\n");
            free(input);
            continue;
        }

        if (strlen(input) <= size_input)
        {
            input = realloc(input, size_input + 1);
            if (input == NULL)
            {
                fprintf(stderr, "Error al reasignar memoria\n");
                return NULL;
            }
            memset(input + strlen(input), ' ', size_input - strlen(input));
            input[size_input] = '\0';
            break;
        }

        printf("Error: El texto introducido supera el límite de <%d> bytes. Intentalo de nuevo.\n", size_input);
        free(input);
    }

    return input;
}

int fs_buscar_bloque_libre(t_io *args)
{
    for (int i = 0; i < args->dial_fs.blockCount; i++)
    {
        if (bitarray_test_bit(args->dial_fs.bitarray, i) == 0)
        {
            return i;
        }
    }
    return -1;
};
void fs_archivo_crear(t_io *args, char *nombre, int indice_bloque_libre)
{
    //**Creamos la fcb**/
    t_fcb *nuevo_fcb = malloc(sizeof(t_fcb));
    nuevo_fcb->total_size = 0;
    nuevo_fcb->inicio = indice_bloque_libre;
    nuevo_fcb->fin_bloque = indice_bloque_libre;
    char *full_path = string_new();
    string_append(&full_path, args->dial_fs.pathBaseDialFs);
    string_append(&full_path, "/");
    string_append(&full_path, args->identificador);
    string_append(&full_path, "/");
    string_append(&full_path, nombre);

    //**Guardamos el archivo**/
    log_debug(args->logger, "Creando archivo en %s", full_path);
    int fd = open(full_path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        log_error(args->logger, "Error al crear el archivo");
        return;
    }
    close(fd);

    // Crear archivo en path
    t_config *metadata = config_create(full_path);
    if (metadata == NULL)
    {
        log_error(args->logger, "Error al crear el archivo de metadata");
        return;
    }
    config_set_value(metadata, "TAMANIO_ARCHIVO", "0");
    config_set_value(metadata, "BLOQUE_INICIAL", string_itoa(indice_bloque_libre));
    config_save_in_file(metadata, full_path);
    nuevo_fcb->metadata = metadata;

    //**Mapeamos el nuevo archivo al diccionario**/
    dictionary_put(args->dial_fs.archivos, nombre, nuevo_fcb);
    log_debug(args->logger, "Archivo: %s, Tamaño: %d, Bloque inicial: %d Bloque final: %d", nombre, nuevo_fcb->total_size, nuevo_fcb->inicio, nuevo_fcb->fin_bloque);
}

int fs_buscar_primera_ocurrencia_libre(t_io *args, int bloque_referencia, int cantidad_bloques)
{
    int contador = 0;
    for (int i = bloque_referencia; i < args->dial_fs.blockCount; i++)
    {
        if (bitarray_test_bit(args->dial_fs.bitarray, i) == 0)
        {
            contador++;
            if (contador == cantidad_bloques)
            {
                return i - cantidad_bloques + 1;
            }
        }
        else
        {
            contador = 0;
        }
    }
    return -1;
}

char *fs_buscar_por_bloque_fin(t_io *args, int bloque_fin)
{
    t_list *keys = dictionary_keys(args->dial_fs.archivos);
    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        t_fcb *fcb = dictionary_get(args->dial_fs.archivos, key);
        if (fcb->fin_bloque == bloque_fin)
        {
            list_destroy(keys);
            return key;
        }
    }
    list_destroy(keys);
    return NULL;
}

void fs_desplazar_archivo_hacia_derecha(t_io *args, char *archivo, int cantidad_bloques)
{
    t_fcb *archivo_a_desplazar = dictionary_get(args->dial_fs.archivos, archivo);
    log_debug(args->logger, "El archivo %s se desplaza %d bloques  a la derecha", archivo, cantidad_bloques);

    char *contenido_archivo = malloc(archivo_a_desplazar->total_size);

    // Calculamos el puntero absoluto
    uint32_t puntero_absoluto = archivo_a_desplazar->inicio * args->dial_fs.blockSize;
    memcpy(contenido_archivo, (char *)args->dial_fs.archivo_bloques + puntero_absoluto, archivo_a_desplazar->total_size);

    // Se verifica si es un archivo que ocupa un bloque
    if (archivo_a_desplazar->inicio == archivo_a_desplazar->fin_bloque)
    {
        log_debug(args->logger, "[%s]: Se setea el bit %d en 0", archivo, archivo_a_desplazar->inicio);
        bitarray_clean_bit(args->dial_fs.bitarray, archivo_a_desplazar->inicio);
        log_debug(args->logger, "[%s]: Se mueve %d bloques", archivo, cantidad_bloques);
        log_debug(args->logger, "[%s]: Se setea el bit %d en 1", archivo, archivo_a_desplazar->inicio + cantidad_bloques);
        bitarray_set_bit(args->dial_fs.bitarray, archivo_a_desplazar->inicio + cantidad_bloques);
    }
    else
    {
        // Ponemos los primeros n bloques en 0
        for (int i = archivo_a_desplazar->inicio; i < archivo_a_desplazar->inicio + cantidad_bloques; i++)
        {
            log_debug(args->logger, "[%s]: Se setea el bit %d en 0", archivo, i);
            bitarray_clean_bit(args->dial_fs.bitarray, i);
        }
        // Desplazamos en el bitmap los bloques ocupados hacia la derecha
        for (int i = archivo_a_desplazar->inicio + cantidad_bloques; i <= archivo_a_desplazar->fin_bloque + cantidad_bloques; i++)
        {
            log_debug(args->logger, "[%s]: Se setea el bit %d en 1", archivo, i);
            bitarray_set_bit(args->dial_fs.bitarray, i);
        }
    }
    // Le sumamos n al bloque_inicio del comparator
    archivo_a_desplazar->inicio = archivo_a_desplazar->inicio + cantidad_bloques;
    // Recalculamos el fin_bloque
    archivo_a_desplazar->fin_bloque = archivo_a_desplazar->inicio + (archivo_a_desplazar->total_size / args->dial_fs.blockSize);

    // Actualizamos ese metadata
    config_set_value(archivo_a_desplazar->metadata, "BLOQUE_INICIAL", string_itoa(archivo_a_desplazar->inicio));
    config_save(archivo_a_desplazar->metadata);

    // Actualizamos el bloques.dat
    puntero_absoluto = archivo_a_desplazar->inicio * args->dial_fs.blockSize;
    memcpy((char *)args->dial_fs.archivo_bloques + puntero_absoluto, contenido_archivo, archivo_a_desplazar->total_size);

    free(contenido_archivo);
}

t_list *fs_obtener_archivos_ordenados(t_io *args)
{
    t_list *elements = dictionary_elements(args->dial_fs.archivos);
    // Ordenamos los archivos por bloque inicial con list sort de menor a mayor
    t_list *sorted_elements = list_sorted(elements, (void *)fs_comparar_archivos_por_bloque_inicial);
    t_list *sorted_keys = list_create();
    t_list *keys = dictionary_keys(args->dial_fs.archivos);
    for (int i = 0; i < list_size(sorted_elements); i++)
    {
        t_fcb *fcb = list_get(sorted_elements, i);
        for (int j = 0; j < list_size(keys); j++)
        {
            char *key = list_get(keys, j);
            t_fcb *fcb_key = dictionary_get(args->dial_fs.archivos, key);
            if (fcb_key->inicio == fcb->inicio)
            {
                list_add(sorted_keys, key);
                break;
            }
        }
    }
    list_destroy(elements);
    list_destroy(sorted_elements);
    return sorted_keys;
}

bool fs_comparar_archivos_por_bloque_inicial(void *archivo1, void *archivo2)
{
    t_fcb *fcb1 = (t_fcb *)archivo1;
    t_fcb *fcb2 = (t_fcb *)archivo2;
    // Ordenar de bloque inicio menor a mayor
    return fcb1->inicio < fcb2->inicio;
}

void fs_consumir_unidad_trabajo(t_io *args)
{
    // El FS siempre consumira 1 unidad de trabajo
    log_debug(args->logger, "Consumiendo 1 unidad de trabajo en %d ms", args->tiempoUnidadDeTrabajo);
    sleep(args->tiempoUnidadDeTrabajo / 1000);
}

bool fs_tiene_compactar(t_io *args, t_fcb *archivo, char *nombre_archivo, int cantidad_a_truncar)
{
    bool compactar = false;
    int fin_bloque = archivo->fin_bloque + cantidad_a_truncar;
    // Obtenemos los archivos ordenados de forma ascendente por bloque de inicio
    t_list *keys = fs_obtener_archivos_ordenados(args);
    for (int i = 0; i < list_size(keys); i++)
    {
        char *key = list_get(keys, i);
        if (strcmp(key, nombre_archivo) == 0)
        {
            // Verificamos si hay que compactar
            if (i == list_size(keys) - 1)
            {
                break;
            }
            char *key_siguiente = list_get(keys, i + 1);
            t_fcb *comparator = dictionary_get(args->dial_fs.archivos, key_siguiente);
            if (fin_bloque >= comparator->inicio)
            {
                compactar = true;
                break;
            }
            break;
        }
    }

    // Liberamos memoria
    list_destroy(keys);
    return compactar;
};

int fs_bloques_ocupados(t_io *args)
{
    // Contar cantidad de bloques ocupados con bitarray
    int bloques_ocupados = 0;
    for (int i = 0; i < args->dial_fs.blockCount; i++)
    {
        if (bitarray_test_bit(args->dial_fs.bitarray, i) == 1)
        {
            bloques_ocupados++;
        }
    }
    return bloques_ocupados;
}

void fs_compactar(t_io *args, t_kernel_entrada_salida_fs_truncate *truncate, t_fcb *archivo, int cantidad_bloques_a_truncar)
{
    log_info(args->logger, "DialFS - Inicio Compactación: PID: <%d> - Inicio Compactación.", truncate->pid);
    t_list *keys = fs_obtener_archivos_ordenados(args);

    // Estrategia:
    // 1. Recorrer los archivos ordenados por bloque inicial
    for (int i = 0; i < list_size(keys); i++)
    {
        // 2. Desde el archivo que quiero truncar y la cantidad que quiero truncar busco la primer ocurrencia de n bloques contiguos libres

        int bloque_libre = fs_buscar_primera_ocurrencia_libre(args, archivo->fin_bloque, cantidad_bloques_a_truncar);
        if (bloque_libre == -1)
        {
            // Llegado a este caso se termina de buscar
            break;
        }
        // 3. Busco el archivo mas proximo al bloque fin libre (Se hace restando 1)
        int bloque_fin_mas_proximo = bloque_libre - 1;
        char *archivo_mas_proximo = fs_buscar_por_bloque_fin(args, bloque_fin_mas_proximo);

        // No se compacta el archivo que se quiere truncar
        if (strcmp(archivo_mas_proximo, truncate->nombre_archivo) == 0)
        {
            break;
        }

        // 4. Ese archivo encontrado se lo desplaza a la derecha la cantidad indicada por cantidad_bloques_a_truncar
        fs_desplazar_archivo_hacia_derecha(args, archivo_mas_proximo, cantidad_bloques_a_truncar);
    }

    // Se emula retraso de compactación
    int tiempo_dormir = args->dial_fs.retrasoCompactacion / 1000;
    sleep(tiempo_dormir);

    // Log obligatorio
    log_info(args->logger, "DialFS - Fin Compactación: PID: <%d> - Fin Compactación.", truncate->pid);
}