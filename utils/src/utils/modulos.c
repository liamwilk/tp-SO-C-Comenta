#include "modulos.h"

Kernel inicializar_kernel(t_config *config)
{
    Kernel kernel;
    kernel.puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA");
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

void log_kernel(Kernel kernel, t_log *logger){
    log_info(logger, "PUERTO_ESCUCHA: %d", kernel.puertoEscucha);
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