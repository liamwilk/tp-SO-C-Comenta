#include "modulos.h"

void handshake_crear(t_handshake *handshake, t_log *logger)
{
    handshake->socketDestino = crear_conexion(handshake->logger, handshake->ipDestino, handshake->puertoDestino, handshake->modulo);
    uint32_t result;
    int conexion = 1;
    send(handshake->socketDestino, &conexion, sizeof(uint32_t), 0);
    recv(handshake->socketDestino, &result, sizeof(uint32_t), MSG_WAITALL);

    if (result == 0)
    {
        log_info(handshake->logger, "[%s] Conexion establecida.", handshake->modulo);
    }
    else
    {
        log_error(handshake->logger, "[%s] Error en la conexión.", handshake->modulo);
    }
};

t_kernel kernel_inicializar(t_config *config)
{
    t_kernel kernel;
    kernel.puertoEscucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    kernel.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    kernel.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
    kernel.ipCpu = config_get_string_value(config, "IP_CPU");
    kernel.puertoCpuDispatch = config_get_int_value(config, "PUERTO_CPU_DISPATCH");
    kernel.puertoCpuInterrupt = config_get_int_value(config, "PUERTO_CPU_INTERRUPT");
    kernel.algoritmoPlanificador = config_get_string_value(config, "ALGORITMO_PLANIFICADOR");
    kernel.quantum = config_get_int_value(config, "QUANTUM");
    kernel.recursos = config_get_string_value(config, "RECURSOS");
    kernel.instanciasRecursos = config_get_string_value(config, "INSTANCIAS_RECURSOS");
    kernel.gradoMultiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    return kernel;
};

void kernel_log(t_kernel kernel, t_log *logger)
{
    log_info(logger, "PUERTO_ESCUCHA: %s", kernel.puertoEscucha);
    log_info(logger, "IP_MEMORIA: %s", kernel.ipMemoria);
    log_info(logger, "PUERTO_MEMORIA: %d", kernel.puertoMemoria);
    log_info(logger, "IP_CPU: %s", kernel.ipCpu);
    log_info(logger, "PUERTO_CPU_DISPATCH: %d", kernel.puertoCpuDispatch);
    log_info(logger, "PUERTO_CPU_INTERRUPT: %d", kernel.puertoCpuInterrupt);
    log_info(logger, "ALGORITMO_PLANIFICADOR: %s", kernel.algoritmoPlanificador);
    log_info(logger, "QUANTUM: %d", kernel.quantum);
    log_info(logger, "RECURSOS: %s", kernel.recursos);
    log_info(logger, "INSTANCIAS_RECURSOS: %s", kernel.instanciasRecursos);
    log_info(logger, "GRADO_MULTIPROGRAMACION: %d", kernel.gradoMultiprogramacion);
}

t_handshake kernel_handshake_memoria(t_kernel kernel, void *fn, t_log *logger)
{
    // Handshake memoria
    t_handshake handshakeMemoria = {
        kernel.ipMemoria,
        kernel.puertoMemoria,
        0,
        "Memoria",
        logger};

    hilo_crear_d(&fn, &handshakeMemoria, logger);
    return handshakeMemoria;
};

t_handshake kernel_handshake_cpu_dispatch(t_kernel kernel, void *fn, t_log *logger)
{
    // Handshake memoria
    t_handshake handshakeCpuDispatch = {
        kernel.ipCpu,
        kernel.puertoCpuDispatch,
        0,
        "CPU Dispatch",
        logger};
    hilo_crear_d(&fn, &handshakeCpuDispatch, logger);
    return handshakeCpuDispatch;
};

t_handshake kernel_handshake_cpu_interrupt(t_kernel kernel, void *fn, t_log *logger)
{
    // Handshake Cpu Interrupt
    t_handshake handshakeCpuInterrupt = {
        kernel.ipCpu,
        kernel.puertoCpuInterrupt,
        0,
        "CPU Interrupt",
        logger};
    hilo_crear_d(&fn, &handshakeCpuInterrupt, logger);
    return handshakeCpuInterrupt;
};
