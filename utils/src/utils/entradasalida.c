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
    if (!strcmp(tipoInterfaz, "GEN"))
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
    io->pathBaseDialFs = config_get_string_value(io->config, "PATH_BASE_DIALFS");
    io->blockCount = config_get_int_value(io->config, "BLOCK_COUNT");
    io->blockSize = config_get_int_value(io->config, "BLOCK_SIZE");
    io->retrasoCompactacion = config_get_int_value(io->config, "RETRASO_COMPACTACION");
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
    log_info(io->logger, "PATH_BASE_DIALFS: %s", io->pathBaseDialFs);
    log_info(io->logger, "BLOCK_SIZE: %d", io->blockSize);
    log_info(io->logger, "BLOCK_COUNT: %d", io->blockCount);
    log_info(io->logger, "RETRASO_COMPACTACION: %d", io->retrasoCompactacion);
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
    args->identificador = strdup(argv[1]);
    args->logger = iniciar_logger(args->identificador, LOG_LEVEL_DEBUG);
    args->config = crear_config_parametro(args->logger, argv[2]);
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
    log_destroy(args->logger);
    config_destroy(args->config);
}

int inicializar_modulo_interfaz(t_io *args, int argc, char *argv[], timer_args_io_t *temporizador)
{
    if (verificar_argumentos(argc))
    {
        return -1;
    }
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

void interfaz_iniciar_temporizador(t_io *args, int duracion)
{
    // Crea el temporizador
    timer_create(CLOCK_REALTIME, &args->timer.sev, &args->timer.timer);

    switch (determinar_interfaz(args->config))
    {
    case GEN:
    {
        // Convierto el tiempo de milisegundos a segundos
        args->duracion = (duracion * args->tiempoUnidadDeTrabajo) / 1000;
        break;
    }
    case STDIN:
    {
        // Placehodler
        break;
    }
    case STDOUT:
    {
        // Placeholder
        break;
    }
    case DIALFS:
    {
        // Placeholder
        break;
    }
    }

    // Configura el tiempo de inicio y el intervalo del temporizador
    args->timer.its.it_value.tv_sec = args->duracion;
    args->timer.its.it_value.tv_nsec = 0;
    args->timer.its.it_interval.tv_sec = 0;
    args->timer.its.it_interval.tv_nsec = 0;

    // Inicia el temporizador
    timer_settime(args->timer.timer, 0, &args->timer.its, NULL);
}

void interfaz_inicializar_temporizador(t_io *args, timer_args_io_t *temporizador)
{
    // Configura la estructura sigevent
    args->timer.sev.sigev_notify = SIGEV_THREAD;
    args->timer.sev.sigev_value.sival_ptr = temporizador;
    args->timer.sev.sigev_notify_function = interfaz_manejador_temporizador;
    args->timer.sev.sigev_notify_attributes = NULL;
}

void interfaz_manejador_temporizador(union sigval arg)
{
    timer_args_io_t *timerArgs = (timer_args_io_t *)arg.sival_ptr;

    switch (determinar_interfaz(timerArgs->args->config))
    {
    case GEN:
    {
        // Convierto el tiempo de milisegundos a segundos
        t_paquete *aviso_sleep = crear_paquete(ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP);
        t_entrada_salida_kernel_unidad_de_trabajo *unidad = malloc(sizeof(t_entrada_salida_kernel_unidad_de_trabajo));

        unidad->terminado = true;
        unidad->pid = timerArgs->args->pid;

        serializar_t_entrada_salida_kernel_unidad_de_trabajo(&aviso_sleep, unidad);

        enviar_paquete(aviso_sleep, timerArgs->args->sockets.socket_kernel_generic);

        log_info(timerArgs->args->logger, "Se notifica finalizacion de instruccion <IO_GEN_SLEEP> <%s> <%d> del PID <%d>", timerArgs->args->identificador, timerArgs->args->unidades, timerArgs->args->pid);

        eliminar_paquete(aviso_sleep);
        free(unidad);
        break;
    }
    case STDIN:
    {
        // Placehodler
        break;
    }
    case STDOUT:
    {
        // Placeholder
        break;
    }
    case DIALFS:
    {
        // Placeholder
        break;
    }
    }
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
            for (int i = strlen(input); i < size_input; i++)
            {
                input[i] = '\n';
            }
            printf("Size cadena: %ld\n", strlen(input));
            break;
        }

        printf("Error: El texto introducido supera el límite de <%d> bytes. Intentalo de nuevo.\n", size_input);
        free(input);
    }

    return input;
}