#include "cpu.h"

void instruccion_ciclo(t_cpu *args, t_buffer *buffer)
{
    if (instruccion_recibir(args, buffer))
    {
        log_error(args->logger, "Instruccion invalida");
        return;
    }

    if (instruccion_ejecutar(args))
    {
        if (args->tipo_instruccion != EXIT)
        {
            log_debug(args->logger, "Se difurca la ejecucion del proceso PID <%d> en la instruccion <%s>.", args->proceso.pid, args->instruccion.array[0]);
        }
        return;
    }

    if (args->flag_interrupt)
    {
        instruccion_interrupt(args);
        args->flag_interrupt = 0;
        return;
    }

    instruccion_solicitar(args);
}

void inicializar_modulo_cpu(t_cpu *argumentos, t_log_level nivel, int argc, char *argv[])
{
    inicializar_argumentos_cpu(argumentos, nivel, argc, argv);
    inicializar_servidores_cpu(argumentos);
    inicializar_hilos_cpu(argumentos);
    liberar_recursos_cpu(argumentos);
}

void liberar_recursos_cpu(t_cpu *argumentos)
{
    log_destroy(argumentos->logger);
    config_destroy(argumentos->config);
}

void inicializar_servidores_cpu(t_cpu *argumentos)
{
    argumentos->config_leida.socket_server_dispatch = iniciar_servidor(argumentos->logger, argumentos->config_leida.puertoEscuchaDispatch);
    argumentos->config_leida.socket_server_interrupt = iniciar_servidor(argumentos->logger, argumentos->config_leida.puertoEscuchaInterrupt);
}

void inicializar_argumentos_cpu(t_cpu *args, t_log_level nivel, int argc, char *argv[])
{
    args->logger = iniciar_logger("CPU", nivel);
    inicializar_config(&args->config, args->logger, argc, argv);
    cpu_leer_config(args);
    cpu_imprimir_log(args);
}

void *conectar_memoria(void *args_void)
{
    t_cpu *args = (t_cpu *)args_void;
    conexion_crear(args->logger, args->config_leida.ipMemoria, args->config_leida.puertoMemoria, &args->config_leida.socket_memoria, "Memoria", MEMORIA_CPU);
    pthread_exit(0);
}

void *atender_memoria(void *args_void)
{
    t_cpu *args = (t_cpu *)args_void;
    cpu_enviar_aviso_memoria_tam_pagina(args);
    hilo_ejecutar_cpu(args, args->config_leida.socket_memoria, "Memoria", switch_case_memoria);
    pthread_exit(0);
}

void *esperar_kernel_dispatch(void *args_void)
{
    t_cpu *args = (t_cpu *)args_void;
    conexion_recibir(args->logger, args->config_leida.socket_server_dispatch, &args->config_leida.socket_kernel_dispatch, "Kernel por Dispatch", CPU_DISPATCH_KERNEL);
    pthread_exit(0);
}

void *atender_kernel_dispatch(void *args_void)
{
    t_cpu *args = (t_cpu *)args_void;
    hilo_ejecutar_cpu(args, args->config_leida.socket_kernel_dispatch, "Kernel Dispatch", switch_case_kernel_dispatch);
    pthread_exit(0);
}

void *esperar_kernel_interrupt(void *args_void)
{
    t_cpu *args = (t_cpu *)args_void;
    conexion_recibir(args->logger, args->config_leida.socket_server_interrupt, &args->config_leida.socket_kernel_interrupt, "Kernel por Interrupt", CPU_INTERRUPT_KERNEL);
    pthread_exit(0);
}

void *atender_kernel_interrupt(void *args_void)
{
    t_cpu *args = (t_cpu *)args_void;
    hilo_ejecutar_cpu(args, args->config_leida.socket_kernel_interrupt, "Kernel Interrupt", switch_case_kernel_interrupt);
    pthread_exit(0);
}

void inicializar_hilos_cpu(t_cpu *args)
{
    pthread_create(&args->threads.thread_conectar_memoria, NULL, conectar_memoria, args);
    pthread_join(args->threads.thread_conectar_memoria, NULL);

    pthread_create(&args->threads.thread_atender_memoria, NULL, atender_memoria, args);
    pthread_detach(args->threads.thread_atender_memoria);

    pthread_create(&args->threads.thread_esperar_kernel_dispatch, NULL, esperar_kernel_dispatch, args);
    pthread_join(args->threads.thread_esperar_kernel_dispatch, NULL);

    pthread_create(&args->threads.thread_atender_kernel_dispatch, NULL, atender_kernel_dispatch, args);
    pthread_detach(args->threads.thread_atender_kernel_dispatch);

    pthread_create(&args->threads.thread_esperar_kernel_interrupt, NULL, esperar_kernel_interrupt, args);
    pthread_join(args->threads.thread_esperar_kernel_interrupt, NULL);

    pthread_create(&args->threads.thread_atender_kernel_interrupt, NULL, atender_kernel_interrupt, args);
    pthread_join(args->threads.thread_atender_kernel_interrupt, NULL);
}

void hilo_ejecutar_cpu(t_cpu *args, int socket, char *modulo, t_funcion_cpu_ptr switch_case_atencion)
{
    while (1)
    {
        pthread_testcancel();

        log_debug(args->logger, "Esperando paquete de %s en socket %d", modulo, socket);

        t_paquete *paquete = recibir_paquete(args->logger, &socket);

        if (paquete == NULL)
        {
            log_warning(args->logger, "%s se desconecto del socket %d.", modulo, socket);
            break;
        }

        revisar_paquete(paquete, args->logger, modulo);

        switch_case_atencion(args, paquete->codigo_operacion, paquete->buffer);

        eliminar_paquete(paquete);
    }
}

void cpu_leer_config(t_cpu *args)
{
    args->config_leida.puertoEscuchaDispatch = config_get_int_value(args->config, "PUERTO_ESCUCHA_DISPATCH");
    args->config_leida.puertoEscuchaInterrupt = config_get_int_value(args->config, "PUERTO_ESCUCHA_INTERRUPT");
    args->config_leida.ipMemoria = config_get_string_value(args->config, "IP_MEMORIA");
    args->config_leida.puertoMemoria = config_get_int_value(args->config, "PUERTO_MEMORIA");
    args->config_leida.cantidadEntradasTlb = config_get_int_value(args->config, "CANTIDAD_ENTRADAS_TLB");
    args->config_leida.algoritmoTlb = config_get_string_value(args->config, "ALGORITMO_TLB");
}

void cpu_imprimir_log(t_cpu *args)
{
    log_info(args->logger, "PUERTO_ESCUCHA_DISPATCH: %d", args->config_leida.puertoEscuchaDispatch);
    log_info(args->logger, "PUERTO_ESCUCHA_INTERRUPT: %d", args->config_leida.puertoEscuchaInterrupt);
    log_info(args->logger, "IP_MEMORIA: %d", args->config_leida.puertoMemoria);
    log_info(args->logger, "PUERTO_MEMORIA: %d", args->config_leida.puertoEscuchaInterrupt);
    log_info(args->logger, "CANTIDAD_ENTRADAS_TLB: %d", args->config_leida.cantidadEntradasTlb);
    log_info(args->logger, "ALGORITMO_TLB: %s", args->config_leida.algoritmoTlb);
};

void instruccion_solicitar(t_cpu *args)
{
    t_paquete *paquete = crear_paquete(CPU_MEMORIA_PROXIMA_INSTRUCCION);
    actualizar_buffer(paquete, sizeof(uint32_t) + sizeof(uint32_t));
    serializar_uint32_t(args->proceso.registros.pc, paquete);
    serializar_uint32_t(args->proceso.pid, paquete);
    enviar_paquete(paquete, args->config_leida.socket_memoria);
    eliminar_paquete(paquete);
    log_debug(args->logger, "Instruccion PC %d de PID <%d> pedida a Memoria", args->proceso.registros.pc, args->proceso.pid);
}

void log_instruccion(t_cpu *args)
{
    char log_message[256] = {0};
    int offset = snprintf(log_message, sizeof(log_message), "PID: <%d> - Ejecutando:", args->proceso.pid);

    for (int i = 0; i < args->instruccion.cantidad_elementos; i++)
    {
        offset += snprintf(log_message + offset, sizeof(log_message) - offset, " <%s>", args->instruccion.array[i]);
    }

    log_info(args->logger, "%s", log_message);
}

int instruccion_ejecutar(t_cpu *args)
{
    remover_salto_linea(args->instruccion.array[args->instruccion.cantidad_elementos - 1]);
    log_instruccion(args);

    switch (args->tipo_instruccion)
    {
    case SET:
    {
        // SET (Registro, Valor): Asigna al registro el valor pasado como parámetro.
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        char *registro_destino = strdup(args->instruccion.array[1]);
        char *valor = strdup(args->instruccion.array[2]);

        t_registro registro_tipo = obtener_tipo_registro(registro_destino);

        if (registro_tipo == REGISTRO_32)
        {
            uint32_t *registro = determinar_tipo_registro_uint32_t(registro_destino, &args->proceso);
            if (registro == 0)
            {
                log_error(args->logger, "Registro %s a insertar invalido", registro_destino);
                return -1;
            }
            uint32_t valor_a_enviar = strtoul(valor, NULL, 10);
            if (valor_a_enviar > UINT32_MAX)
            {
                log_error(args->logger, "El numero es mas grande que la capacidad de uint32");
                return -1;
            }
            *registro = valor_a_enviar;
        }
        else if (registro_tipo == REGISTRO_8) // Misma lógica que para registro de 4 bytes;
        {
            uint8_t *registro = determinar_tipo_registro_uint8_t(registro_destino, &args->proceso);
            if (registro == 0)
            {
                log_error(args->logger, "Registro %s a insertar invalido", registro_destino);
                return -1;
            }
            unsigned long num = strtoul(valor, NULL, 10);
            if (num > UINT8_MAX)
            {
                log_error(args->logger, "El numero es mas grande que la capacidad de uint8");
                return -1;
            }
            uint8_t valor_a_enviar = (uint8_t)num;
            *registro = valor_a_enviar;
            log_debug(args->logger, "Valor final del registro %s: %d", registro_destino, *registro);
        }
        else
        {
            log_error(args->logger, "No existe registro con ese tipo de dato");
        }
        imprimir_registros(args);

        free(registro_destino);
        free(valor);
        return 0;
    }
    case SUM:
    {
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        char *argumento_destino = strdup(args->instruccion.array[1]);
        char *argumento_origen = strdup(args->instruccion.array[2]);

        // Determino si es un registro de 32 o 8 bits
        t_registro argumento_destino_tipo = obtener_tipo_registro(argumento_destino);
        t_registro argumento_origen_tipo = obtener_tipo_registro(argumento_origen);

        log_debug(args->logger, "Argumento destino tipo: %d", argumento_destino_tipo);
        log_debug(args->logger, "Argumento origen tipo: %d", argumento_origen_tipo);

        if (argumento_destino_tipo != INVALIDO && argumento_origen_tipo != INVALIDO)
        {

            if (argumento_destino_tipo == argumento_origen_tipo)
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_destino_anterior = *registro_destino;
                    *registro_destino = *registro_destino + *registro_origen;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino %s no encontrado", argumento_destino);
                        return -1;
                    }

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen %s no encontrado", argumento_origen);
                        return -1;
                    }
                    uint32_t registro_destino_anterior = *registro_destino;
                    *registro_destino += *registro_origen;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);
                }
            }
            else
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {

                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_destino_anterior = *registro_destino;

                    uint32_t registro_origen_casteado = casteo_uint32_t(*registro_origen);

                    *registro_destino = *registro_destino + registro_origen_casteado;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);

                    log_debug(args->logger, "Argumento destino tipo: %d", argumento_destino_tipo);
                }
                if (argumento_destino_tipo == REGISTRO_8)
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint8_t registro_destino_anterior = *registro_destino;
                    uint8_t registro_origen_casteado;

                    if (*registro_origen > 0)
                    {
                        registro_origen_casteado = casteo_uint8_t(*registro_origen);

                        if (registro_origen_casteado == 0)
                        {
                            log_error(args->logger, "No se puede realizar la suma, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                            return -1;
                        }
                    }
                    else
                    { // Dado que son unsigned, son siempre positivos. Entonces, el unico caso que queda es que sea 0.
                        registro_origen_casteado = 0;
                    }

                    if (*registro_destino + registro_origen_casteado > UINT8_MAX)
                    {
                        log_error(args->logger, "No se puede realizar la suma, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                        return -1;
                    }

                    *registro_destino = *registro_destino + registro_origen_casteado;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);

                    log_debug(args->logger, "Argumento destino tipo: %d", argumento_destino_tipo);
                }
                break;
            }
        }
        else
        {
            log_error(args->logger, "Se reconocieron los registros %s y %s. No se pueden sumar porque al menos uno no es válido.", argumento_destino, argumento_origen);
            return -1;
        }

        free(argumento_destino);
        free(argumento_origen);
        return 0;
    }
    case SUB:
    {
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        char *argumento_destino = strdup(args->instruccion.array[1]);
        char *argumento_origen = strdup(args->instruccion.array[2]);

        // Determino si es un registro de 32 o 8 bits
        t_registro argumento_destino_tipo = obtener_tipo_registro(argumento_destino);
        t_registro argumento_origen_tipo = obtener_tipo_registro(argumento_origen);

        if (argumento_destino_tipo != INVALIDO && argumento_origen_tipo != INVALIDO)
        {
            if (argumento_destino_tipo == argumento_origen_tipo)
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    if (*registro_destino < *registro_origen)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= *registro_origen;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino %s no encontrado", argumento_destino);
                        return -1;
                    }

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen %s no encontrado", argumento_origen);
                        return -1;
                    }

                    if (*registro_destino < *registro_origen)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino = *registro_destino - *registro_origen;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);
                }
            }
            else
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_origen_casteado = casteo_uint32_t(*registro_origen);

                    if (*registro_destino < registro_origen_casteado)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= registro_origen_casteado;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint8_t registro_origen_casteado;

                    if (*registro_origen > 0)
                    {
                        registro_origen_casteado = casteo_uint8_t(*registro_origen);

                        if (registro_origen_casteado == 0)
                        {
                            log_error(args->logger, "No se puede realizar la resta, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                            return -1;
                        }
                    }
                    else
                    { // Dado que son unsigned, son siempre positivos. Entonces, el unico caso que queda es que sea 0.
                        registro_origen_casteado = 0;
                    }

                    if (*registro_destino < registro_origen_casteado)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= registro_origen_casteado;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
            }
        }
        else
        {
            log_error(args->logger, "Se reconocieron los registros %s y %s. No se pueden sumar porque al menos uno no es válido.", argumento_destino, argumento_origen);
            return -1;
        }

        free(argumento_destino);
        free(argumento_origen);
        return 0;
    }
    case JNZ:
    {
        // JNZ (Registro, Instrucción): Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro.
        char *registro = strdup(args->instruccion.array[1]);
        char *instruccion = strdup(args->instruccion.array[2]);

        t_registro registro_tipo_size = obtener_tipo_registro(registro);

        if (registro_tipo_size == REGISTRO_8)
        {
            uint8_t *registro_destino = determinar_tipo_registro_uint8_t(registro, &args->proceso);
            if (*registro_destino != 0)
            {
                uint32_t valor_pc = strtoul(instruccion, NULL, 10);
                args->proceso.registros.pc = valor_pc;
            }
        }
        else if (registro_tipo_size == REGISTRO_32)
        {
            uint32_t *registro_destino = determinar_tipo_registro_uint32_t(registro, &args->proceso);
            if (*registro_destino != 0)
            {
                uint32_t valor_pc = strtoul(instruccion, NULL, 10);
                args->proceso.registros.pc = valor_pc;
            }
        }
        else
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }
        return 0;
    }
    case IO_GEN_SLEEP:
    {
        // IO_GEN_SLEEP (Interfaz, Unidades de trabajo): Esta instrucción solicita al Kernel que se envíe a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo.

        char *interfaz = strdup(args->instruccion.array[1]);
        uint32_t tiempo = atoi(args->instruccion.array[2]);

        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        log_debug(args->logger, "Interfaz: %s", interfaz);
        log_debug(args->logger, "Tiempo: %d", tiempo);

        log_debug(args->logger, "Se envia un sleep de %d a la Interfaz IO Generic ID %s", tiempo, interfaz);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_GEN_SLEEP);

        t_cpu_kernel_io_gen_sleep *unidad = malloc(sizeof(t_cpu_kernel_io_gen_sleep));
        unidad->tiempo = tiempo;
        unidad->size_interfaz = strlen(interfaz) + 1;
        unidad->interfaz = strdup(interfaz);
        unidad->pid = args->proceso.pid;
        unidad->registros = args->proceso.registros;

        serializar_t_cpu_kernel_io_gen_sleep(&paquete, unidad);

        enviar_paquete(paquete, args->config_leida.socket_kernel_interrupt);

        imprimir_registros(args);

        free(unidad->interfaz);
        free(unidad);
        eliminar_paquete(paquete);
        return 1;
    }
    case MOV_IN:
    {
        log_debug(args->logger, "reconoci un MOV_IN");

        // TODO: Implementar

        // Ahora lo que hago es testear si funciona la MMU correctamente

        // t_cpu_memoria_numero_frame *proceso = malloc(sizeof(t_cpu_memoria_numero_frame));
        // proceso->pid = cpu_proceso->pid;

        // proceso->numero_pagina = calcular_numero_pagina(cpu_proceso);

        // t_paquete *paquete = crear_paquete(CPU_MEMORIA_NUMERO_FRAME);
        // serializar_t_cpu_memoria_numero_frame(&paquete, proceso);

        // enviar_paquete(paquete, cpu_paquete.socket_memoria);
        // free(paquete);

        // uint32_t numero_marco = deserializar_t_memoria_cpu_numero_marco(recibir_paquete(cpu_paquete.socket_memoria));

        // log_info(args->logger, "Numero de marco: %d", numero_marco);

        return 1;
    }
    case MOV_OUT:
    {
        log_debug(args->logger, "reconoci un MOV_OUT");
        return 1;
    }
    case RESIZE:
    {
        log_debug(args->logger, "reconoci un RESIZE");
        return 1;
    }
    case COPY_STRING:
    {
        log_debug(args->logger, "reconoci un COPY_STRING");
        return 1;
    }
    case IO_STDIN_READ:
    {
        log_debug(args->logger, "reconoci un IO_STDIN_READ");
        return 1;
    }
    case IO_STDOUT_WRITE:
    {
        log_debug(args->logger, "reconoci un IO_STDOUT_WRITE");
        return 1;
    }
    case IO_FS_CREATE:
    {
        log_debug(args->logger, "reconoci un IO_FS_CREATE");
        return 1;
    }
    case IO_FS_DELETE:
    {
        log_debug(args->logger, "reconoci un IO_FS_DELETE");
        return 1;
    }
    case IO_FS_TRUNCATE:
    {
        log_debug(args->logger, "reconoci un IO_FS_TRUNCATE");
        return 1;
    }
    case IO_FS_WRITE:
    {
        log_debug(args->logger, "reconoci un IO_FS_WRITE");
        return 1;
    }
    case IO_FS_READ:
    {
        log_debug(args->logger, "reconoci un IO_FS_READ");
        return 1;
    }
    case WAIT:
    {
        char *nombre_recurso = strdup(args->instruccion.array[1]);
        log_debug(args->logger, "Recurso a esperar: %s", nombre_recurso);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_WAIT);

        t_cpu_kernel_solicitud_recurso *contexto = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        contexto->pid = args->proceso.pid;
        contexto->registros = &(args->proceso.registros);
        contexto->size_nombre_recurso = strlen(nombre_recurso) + 1;
        contexto->nombre_recurso = strdup(nombre_recurso);

        serializar_t_cpu_kernel_solicitud_recurso(&paquete, contexto);

        enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

        free(nombre_recurso);
        free(contexto->nombre_recurso);
        free(contexto);

        eliminar_paquete(paquete);

        return 1;
    }
    case SIGNAL:
    {
        char *nombre_recurso = strdup(args->instruccion.array[1]);
        log_debug(args->logger, "Recurso a liberar: %s", nombre_recurso);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_SIGNAL);

        t_cpu_kernel_solicitud_recurso *contexto = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        contexto->pid = args->proceso.pid;
        contexto->registros = &(args->proceso.registros);
        contexto->size_nombre_recurso = strlen(nombre_recurso) + 1;
        contexto->nombre_recurso = strdup(nombre_recurso);

        serializar_t_cpu_kernel_solicitud_recurso(&paquete, contexto);

        enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

        free(nombre_recurso);
        free(contexto->nombre_recurso);
        free(contexto);
        eliminar_paquete(paquete);

        return 1;
    }
    case EXIT:
    {
        cpu_kernel_avisar_finalizacion(args->proceso, args->config_leida.socket_kernel_dispatch);
        return 1;
    }
    }
    log_error(args->logger, "Instruccion invalida");
    return 0;
};

int instruccion_recibir(t_cpu *args, t_buffer *buffer)
{
    log_info(args->logger, "PID : <%d> - FETCH - Program Counter : <%d>", args->proceso.pid, args->proceso.registros.pc);

    t_memoria_cpu_instruccion *dato = deserializar_t_memoria_cpu_instruccion(buffer);
    args->tipo_instruccion = determinar_codigo_instruccion(dato->array[0]);
    args->proceso.registros.pc += 1;
    if (args->tipo_instruccion == -1)
    {
        return 1;
    }
    args->instruccion = *dato;
    free(dato);
    return 0;
};

void proceso_recibir(t_cpu *args, t_buffer *buffer)
{
    t_kernel_cpu_proceso *proceso_cpu = deserializar_t_kernel_cpu_proceso(buffer);
    args->proceso.pid = proceso_cpu->pid,
    args->proceso.registros.pc = proceso_cpu->registros.pc,
    args->proceso.registros.eax = proceso_cpu->registros.eax,
    args->proceso.registros.ebx = proceso_cpu->registros.ebx,
    args->proceso.registros.ecx = proceso_cpu->registros.ecx,
    args->proceso.registros.edx = proceso_cpu->registros.edx,
    args->proceso.registros.ax = proceso_cpu->registros.ax,
    args->proceso.registros.bx = proceso_cpu->registros.bx,
    args->proceso.registros.cx = proceso_cpu->registros.cx,
    args->proceso.registros.dx = proceso_cpu->registros.dx,
    args->proceso.registros.si = proceso_cpu->registros.si,
    args->proceso.registros.di = proceso_cpu->registros.di,

    imprimir_registros(args);
}

void cpu_kernel_avisar_finalizacion(t_cpu_proceso proceso, int socket_kernel_dispatch)
{
    t_paquete *paquete = crear_paquete(CPU_KERNEL_PROCESO);
    t_cpu_kernel_proceso fin_proceso = {
        .ejecutado = 1, .pid = proceso.pid, .registros = {.pc = proceso.registros.pc, .eax = proceso.registros.eax, .ebx = proceso.registros.ebx, .ecx = proceso.registros.ecx, .edx = proceso.registros.edx, .si = proceso.registros.si, .di = proceso.registros.di, .ax = proceso.registros.ax, .bx = proceso.registros.bx, .cx = proceso.registros.cx, .dx = proceso.registros.dx}};

    serializar_t_cpu_kernel_proceso(&paquete, &fin_proceso);
    enviar_paquete(paquete, socket_kernel_dispatch);
    eliminar_paquete(paquete);
};

uint32_t *determinar_tipo_registro_uint32_t(char *instruccion, t_cpu_proceso *proceso)
{
    if (!strcmp(instruccion, "PC"))
    {
        return &proceso->registros.pc;
    }
    if (!strcmp(instruccion, "EAX"))
    {
        return &proceso->registros.eax;
    }
    if (!strcmp(instruccion, "EBX"))
    {
        return &proceso->registros.ebx;
    }
    if (!strcmp(instruccion, "ECX"))
    {
        return &proceso->registros.ecx;
    }
    if (!strcmp(instruccion, "EDX"))
    {
        return &proceso->registros.edx;
    }
    if (!strcmp(instruccion, "SI"))
    {
        return &proceso->registros.si;
    }
    if (!strcmp(instruccion, "DI"))
    {
        return &proceso->registros.di;
    }
    return NULL;
};

uint8_t *determinar_tipo_registro_uint8_t(char *instruccion, t_cpu_proceso *proceso)
{
    if (!strcmp(instruccion, "AX"))
    {
        return &proceso->registros.ax;
    }
    if (!strcmp(instruccion, "BX"))
    {
        return &proceso->registros.bx;
    }
    if (!strcmp(instruccion, "CX"))
    {
        return &proceso->registros.cx;
    }
    if (!strcmp(instruccion, "DX"))
    {
        return &proceso->registros.dx;
    }
    return NULL;
};

t_instruccion determinar_codigo_instruccion(char *instruccion)
{
    // Remover \n
    remover_salto_linea(instruccion);

    /* Todas las posibles instrucciones:
    {
        SET,
        SUM,
        SUB,
        JNZ,
        IO_GEN_SLEEP,
        MOV_IN,
        MOV_OUT,
        RESIZE,
        COPY_STRING,
        IO_STDIN_READ,
        IO_STDOUT_WRITE,
        IO_FS_CREATE,
        IO_FS_DELETE,
        IO_FS_TRUNCATE,
        IO_FS_WRITE,
        IO_FS_READ,
        WAIT,
        SIGNAL,
        EXIT

    */

    if (strcmp(instruccion, "SET") == 0)
    {
        return SET;
    }
    if (strcmp(instruccion, "SUM") == 0)
    {
        return SUM;
    }
    if (strcmp(instruccion, "SUB") == 0)
    {
        return SUB;
    }
    if (strcmp(instruccion, "JNZ") == 0)
    {
        return JNZ;
    }
    if (strcmp(instruccion, "IO_GEN_SLEEP") == 0)
    {
        return IO_GEN_SLEEP;
    }
    if (strcmp(instruccion, "EXIT") == 0)
    {
        return EXIT;
    }
    if (strcmp(instruccion, "MOV_IN") == 0)
    {
        return MOV_IN;
    }
    if (strcmp(instruccion, "MOV_OUT") == 0)
    {
        return MOV_OUT;
    }
    if (strcmp(instruccion, "RESIZE") == 0)
    {
        return RESIZE;
    }
    if (strcmp(instruccion, "COPY_STRING") == 0)
    {
        return COPY_STRING;
    }
    if (strcmp(instruccion, "IO_STDIN_READ") == 0)
    {
        return IO_STDIN_READ;
    }
    if (strcmp(instruccion, "IO_STDOUT_WRITE") == 0)
    {
        return IO_STDOUT_WRITE;
    }
    if (strcmp(instruccion, "IO_FS_CREATE") == 0)
    {
        return IO_FS_CREATE;
    }
    if (strcmp(instruccion, "IO_FS_DELETE") == 0)
    {
        return IO_FS_DELETE;
    }
    if (strcmp(instruccion, "IO_FS_TRUNCATE") == 0)
    {
        return IO_FS_TRUNCATE;
    }
    if (strcmp(instruccion, "IO_FS_WRITE") == 0)
    {
        return IO_FS_WRITE;
    }
    if (strcmp(instruccion, "IO_FS_READ") == 0)
    {
        return IO_FS_READ;
    }
    if (strcmp(instruccion, "WAIT") == 0)
    {
        return WAIT;
    }
    if (strcmp(instruccion, "SIGNAL") == 0)
    {
        return SIGNAL;
    }
    return -1;
};

t_registro obtener_tipo_registro(char *nombre_registro)
{
    if (!strcmp(nombre_registro, "PC"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "EAX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "EBX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "ECX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "EDX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "AX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "BX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "CX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "DX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "DI"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "SI"))
    {
        return REGISTRO_32;
    }
    else
    {
        return INVALIDO;
    }
}

void imprimir_registros(t_cpu *args)
{
    log_debug(args->logger, "Registros del proceso PID <%d>", args->proceso.pid);
    log_debug(args->logger, "PC: %d", args->proceso.registros.pc);
    log_debug(args->logger, "EAX: %d", args->proceso.registros.eax);
    log_debug(args->logger, "EBX: %d", args->proceso.registros.ebx);
    log_debug(args->logger, "ECX: %d", args->proceso.registros.ecx);
    log_debug(args->logger, "EDX: %d", args->proceso.registros.edx);
    log_debug(args->logger, "SI: %d", args->proceso.registros.si);
    log_debug(args->logger, "DI: %d", args->proceso.registros.di);
    log_debug(args->logger, "AX: %d", args->proceso.registros.ax);
    log_debug(args->logger, "BX: %d", args->proceso.registros.bx);
    log_debug(args->logger, "CX: %d", args->proceso.registros.cx);
    log_debug(args->logger, "DX: %d", args->proceso.registros.dx);
}

bool casteo_verificar_uint_8t_valido(uint32_t valor)
{
    return valor <= UINT8_MAX;
}

uint8_t casteo_uint8_t(uint32_t valor)
{
    if (!casteo_verificar_uint_8t_valido(valor))
    {
        return 0;
    }
    return (uint8_t)valor;
}

uint32_t casteo_uint32_t(uint8_t valor)
{
    return (uint32_t)valor;
}

void recibir_interrupcion(t_cpu *args, t_buffer *buffer)
{
    t_kernel_cpu_interrupcion *interrupcion = deserializar_t_kernel_cpu_interrupcion(buffer);
    if (interrupcion->pid == args->proceso.pid)
    {
        log_debug(args->logger, "[INTERRUPCION/%s] PID: <%d>", interrupcion->motivo, interrupcion->pid);
        free(interrupcion);
        args->flag_interrupt = 1;
    }
    else
    {
        log_error(args->logger, "El PID recibido (%d) no se corresponde con el que se esta ejecutando (%d)", interrupcion->pid, args->proceso.pid);
        free(interrupcion);
        args->flag_interrupt = 1;
    }
}

void instruccion_interrupt(t_cpu *args)
{
    log_debug(args->logger, "Se procede a interrumpir el proceso de PID <%d> y enviar el contexto de ejecucion a Kernel", args->proceso.pid);

    t_cpu_kernel_proceso proceso_interrumpido = {
        .ejecutado = 0, .pid = args->proceso.pid, .registros = {.ax = args->proceso.registros.ax, .bx = args->proceso.registros.bx, .cx = args->proceso.registros.cx, .dx = args->proceso.registros.dx, .eax = args->proceso.registros.eax, .ebx = args->proceso.registros.ebx, .ecx = args->proceso.registros.ecx, .edx = args->proceso.registros.edx, .si = args->proceso.registros.si, .di = args->proceso.registros.di, .pc = args->proceso.registros.pc}};

    t_paquete *paquete = crear_paquete(CPU_KERNEL_PROCESO);
    serializar_t_cpu_kernel_proceso(&paquete, &proceso_interrumpido);
    enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
    eliminar_paquete(paquete);
}

void cpu_enviar_aviso_memoria_tam_pagina(t_cpu *cpu)
{
    t_paquete *paquete = crear_paquete(CPU_MEMORIA_TAM_PAGINA);
    uint32_t aviso = 1;

    serializar_t_memoria_cpu_tam_pagina(&paquete, aviso);
    enviar_paquete(paquete, cpu->config_leida.socket_memoria);

    eliminar_paquete(paquete);
}

int mmu(t_cpu *cpu, uint32_t direccion_logica, uint32_t numero_marco)
{
    int numero_pagina = calcular_numero_pagina(cpu, direccion_logica);
    int desplazamiento = direccion_logica - numero_pagina * cpu->tam_pagina;

    int direccion_fisica = numero_marco * cpu->tam_pagina + desplazamiento;

    return direccion_fisica;
}

int calcular_numero_pagina(t_cpu *cpu, uint32_t direccion_logica)
{
    return direccion_logica / cpu->tam_pagina;
}