#include "cpu.h"

t_cpu cpu_inicializar(t_config *config)
{
    t_cpu cpu = {
        .puertoEscuchaDispatch = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH"),
        .puertoEscuchaInterrupt = config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT"),
        .ipMemoria = config_get_string_value(config, "IP_MEMORIA"),
        .puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA"),
        .cantidadEntradasTlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB"),
        .algoritmoTlb = config_get_string_value(config, "ALGORITMO_TLB"),
    };
    return cpu;
};

void cpu_log(t_cpu cpu, t_log *logger)
{
    log_info(logger, "PUERTO_ESCUCHA_DISPATCH: %d", cpu.puertoEscuchaDispatch);
    log_info(logger, "PUERTO_ESCUCHA_INTERRUPT: %d", cpu.puertoEscuchaInterrupt);
    log_info(logger, "IP_MEMORIA: %d", cpu.puertoMemoria);
    log_info(logger, "PUERTO_MEMORIA: %d", cpu.puertoEscuchaInterrupt);
    log_info(logger, "CANTIDAD_ENTRADAS_TLB: %d", cpu.cantidadEntradasTlb);
    log_info(logger, "ALGORITMO_TLB: %s", cpu.algoritmoTlb);
    printf("\n");
};

void cpu_memoria_pedir_proxima_instruccion(t_cpu_proceso *proceso, int socket_memoria)
{
    t_paquete *paquete = crear_paquete(CPU_MEMORIA_PROXIMA_INSTRUCCION);
    actualizar_buffer(paquete, sizeof(uint32_t) + sizeof(uint32_t));
    serializar_uint32_t(proceso->registros.pc, paquete);
    serializar_uint32_t(proceso->pid, paquete);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

void cpu_ejecutar_instruccion(t_memoria_cpu_instruccion *datos_instruccion, t_instruccion INSTRUCCION, t_cpu_proceso *cpu_proceso, t_log *logger)
{
    // Aumento de program counter
    switch (INSTRUCCION)
    {
    case SET:
        log_debug(logger, "reconoci un SET");
        break;
    case SUM:
        log_debug(logger, "reconoci un SUM");
        break;
    case SUB:
        log_debug(logger, "reconoci un SUB");
        break;
    case JNZ:
        log_debug(logger, "reconoci un JNZ");
        break;
    case IO_GEN_SLEEP:
        log_debug(logger, "reconoci un IO_GEN_SLEEP");
        break;
    case EXIT:
        log_debug(logger, "Se hallo instruccion EXIT");
        break;
    }

    log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", cpu_proceso->pid, datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2]); // Log oblitario

    // Hardcodeo (por ahora)
    // uint32_t hay_interrupt = 0;

    //  Check interrupt
    // if (!hay_interrupt)
    // 	cpu_memoria_pedir_proxima_instruccion(pid, pc, socket_memoria) break;
    // atendiendo_interrupcion = true;
};

int cpu_memoria_recibir_instruccion(t_buffer *buffer, t_log *logger, t_memoria_cpu_instruccion *datos_instruccion, t_instruccion *INSTRUCCION, t_cpu_proceso *proceso)
{
    t_memoria_cpu_instruccion *dato = deserializar_t_memoria_cpu_instruccion(buffer);
    *INSTRUCCION = determinar_codigo_instruccion(dato->array[0]);
    proceso->registros.pc += 1;
    log_info(logger, "PID : <%d> - FETCH - Program Counter : <%d>", proceso->pid, proceso->registros.pc); // Log oblitario
    if (*INSTRUCCION == -1)
    {
        log_error(logger, "Instruccion invalida");
        return -1;
    }
    if (*INSTRUCCION == EXIT)
    {
        log_info(logger, "Finalizacion del proceso");
        return 1;
    }
    *datos_instruccion = *dato;
    free(dato);
    return 0;
};

t_cpu_proceso cpu_kernel_recibir_proceso(t_buffer *buffer, t_log *logger)
{
    t_kernel_cpu_proceso *proceso_cpu = deserializar_t_kernel_cpu_proceso(buffer);
    log_debug(logger, "Registros cpu recibido de Kernel Dispatch");
    log_debug(logger, "PID: %d", proceso_cpu->pid);
    log_debug(logger, "PC: %d", proceso_cpu->registros.pc);
    log_debug(logger, "EAX: %d", proceso_cpu->registros.eax);
    log_debug(logger, "EBX: %d", proceso_cpu->registros.ebx);
    log_debug(logger, "ECX: %d", proceso_cpu->registros.ecx);
    log_debug(logger, "EDX: %d", proceso_cpu->registros.edx);
    log_debug(logger, "AX: %d", proceso_cpu->registros.ax);
    log_debug(logger, "BX: %d", proceso_cpu->registros.bx);
    log_debug(logger, "CX: %d", proceso_cpu->registros.cx);
    log_debug(logger, "DX: %d", proceso_cpu->registros.dx);
    log_debug(logger, "SI: %d", proceso_cpu->registros.si);
    log_debug(logger, "DI: %d", proceso_cpu->registros.di);

    /*GUARDAR REGISTROS*/
    t_cpu_proceso proceso = {
        .pid = proceso_cpu->pid,
        .registros = {
            .pc = proceso_cpu->registros.pc,
            .eax = proceso_cpu->registros.eax,
            .ebx = proceso_cpu->registros.ebx,
            .ecx = proceso_cpu->registros.ecx,
            .edx = proceso_cpu->registros.edx,
            .ax = proceso_cpu->registros.ax,
            .bx = proceso_cpu->registros.bx,
            .cx = proceso_cpu->registros.cx,
            .dx = proceso_cpu->registros.dx,
            .si = proceso_cpu->registros.si,
            .di = proceso_cpu->registros.di,
        },
    };
    return proceso;
};

void cpu_kernel_avisar_finalizacion(t_cpu_proceso proceso, int socket_kernel_interrupt)
{
    t_paquete *paquete_proceso = crear_paquete(CPU_KERNEL_PROCESO);
    t_cpu_kernel_proceso *fin_proceso = malloc(sizeof(t_cpu_kernel_proceso));
    fin_proceso->registros = malloc(sizeof(t_registros_cpu));

    fin_proceso->ejecutado = 1;
    fin_proceso->pid = proceso.pid;
    fin_proceso->registros->pc = proceso.registros.pc;
    fin_proceso->registros->eax = proceso.registros.eax;
    fin_proceso->registros->ebx = proceso.registros.ebx;
    fin_proceso->registros->ecx = proceso.registros.ecx;
    fin_proceso->registros->edx = proceso.registros.edx;
    fin_proceso->registros->si = proceso.registros.si;
    fin_proceso->registros->di = proceso.registros.di;
    fin_proceso->registros->ax = proceso.registros.ax;
    fin_proceso->registros->bx = proceso.registros.bx;
    fin_proceso->registros->cx = proceso.registros.cx;
    fin_proceso->registros->dx = proceso.registros.dx;

    serializar_t_cpu_kernel_proceso(&paquete_proceso, fin_proceso);
    enviar_paquete(paquete_proceso, socket_kernel_interrupt);

    eliminar_paquete(paquete_proceso);
    free(fin_proceso->registros);
    free(fin_proceso);
};

uint32_t determinar_tipo_registro_uint32_t(char *instruccion, t_cpu_proceso *proceso)
{
    if (!strcmp(instruccion, "eax"))
    {
        return proceso->registros.eax;
    }
    if (!strcmp(instruccion, "ebx"))
    {
        return proceso->registros.ebx;
    }
    if (!strcmp(instruccion, "ecx"))
    {
        return proceso->registros.ecx;
    }
    if (!strcmp(instruccion, "edx"))
    {
        return proceso->registros.edx;
    }
    if (!strcmp(instruccion, "si"))
    {
        return proceso->registros.si;
    }
    if (!strcmp(instruccion, "di"))
    {
        return proceso->registros.di;
    }
    return -1;
};

uint8_t determinar_tipo_registro_uint8_t(char *instruccion, t_cpu_proceso *proceso)
{
    if (!strcmp(instruccion, "ax"))
    {
        return proceso->registros.ax;
    }
    if (!strcmp(instruccion, "bx"))
    {
        return proceso->registros.bx;
    }
    if (!strcmp(instruccion, "cx"))
    {
        return proceso->registros.cx;
    }
    if (!strcmp(instruccion, "dx"))
    {
        return proceso->registros.dx;
    }
    return -1;
};

t_instruccion determinar_codigo_instruccion(char *instruccion)
{
    if (!strcmp(instruccion, "SET"))
    {
        return SET;
    }
    if (!strcmp(instruccion, "SUM"))
    {
        return SUM;
    }
    if (!strcmp(instruccion, "SUB"))
    {
        return SUB;
    }
    if (!strcmp(instruccion, "JNZ"))
    {
        return JNZ;
    }
    if (!strcmp(instruccion, "IO_GEN_SLEEP"))
    {
        return IO_GEN_SLEEP;
    }
    if (!strcmp(instruccion, "EXIT"))
    {
        return EXIT;
    }

    return -1;
};