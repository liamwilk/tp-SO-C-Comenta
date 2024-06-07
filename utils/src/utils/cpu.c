#include "cpu.h"

void instruccion_ciclo(t_cpu *args, t_buffer *buffer)
{
    if (instruccion_recibir(args, buffer))
    {
        log_error(args->logger, "Instruccion invalida.");
        return;
    }

    if (instruccion_ejecutar(args))
    {
        if (args->tipo_instruccion != EXIT)
        {
            log_debug(args->logger, "Se bifurca el ciclo de instruccion en la ejecucion del proceso PID <%d> en la instruccion <%s>.", args->proceso.pid, args->instruccion.array[0]);
        }
        return;
    }
    else if (args->tipo_instruccion == -1)
    {
        log_error(args->logger, "Se finaliza la ejecucion del proceso PID <%d> por error en la instruccion <%s>.", args->proceso.pid, args->instruccion.array[0]);
        args->proceso.ejecutado = 0;
        instruccion_finalizar(args);
    }

    if (args->flag_interrupt)
    {
        instruccion_interrupt(args);
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

void instruccion_finalizar(t_cpu *args)
{
    t_paquete *paquete = crear_paquete(CPU_KERNEL_PROCESO);
    t_cpu_kernel_proceso fin_proceso = {
        .ejecutado = args->proceso.ejecutado, .pid = args->proceso.pid, .registros = {.pc = args->proceso.registros.pc, .eax = args->proceso.registros.eax, .ebx = args->proceso.registros.ebx, .ecx = args->proceso.registros.ecx, .edx = args->proceso.registros.edx, .si = args->proceso.registros.si, .di = args->proceso.registros.di, .ax = args->proceso.registros.ax, .bx = args->proceso.registros.bx, .cx = args->proceso.registros.cx, .dx = args->proceso.registros.dx}};

    serializar_t_cpu_kernel_proceso(&paquete, &fin_proceso);
    enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
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
        .ejecutado = 2, .pid = args->proceso.pid, .registros = {.ax = args->proceso.registros.ax, .bx = args->proceso.registros.bx, .cx = args->proceso.registros.cx, .dx = args->proceso.registros.dx, .eax = args->proceso.registros.eax, .ebx = args->proceso.registros.ebx, .ecx = args->proceso.registros.ecx, .edx = args->proceso.registros.edx, .si = args->proceso.registros.si, .di = args->proceso.registros.di, .pc = args->proceso.registros.pc}};

    t_paquete *paquete = crear_paquete(CPU_KERNEL_PROCESO);
    serializar_t_cpu_kernel_proceso(&paquete, &proceso_interrumpido);
    enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
    eliminar_paquete(paquete);

    args->flag_interrupt = 0;
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

uint32_t calcular_numero_pagina(t_cpu *cpu, uint32_t direccion_logica)
{
    return floor(direccion_logica / cpu->tam_pagina);
}