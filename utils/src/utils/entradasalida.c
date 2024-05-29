#include "entradasalida.h"

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

int inicializar_modulo_interfaz(t_io *args, int argc, char *argv[])
{
    if (verificar_argumentos(argc))
    {
        return -1;
    }
    inicializar_argumentos(args, argv);
    inicializar_interfaz(args);
    finalizar_interfaz(args);
    return 0;
}