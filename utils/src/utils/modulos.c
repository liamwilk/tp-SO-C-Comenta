#include "modulos.h"


t_kernel kernel_inicializar(t_config *config)
{
    t_kernel kernel;
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

void kernel_log(t_kernel kernel, t_log *logger)
{
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


/*--------CPU--------*/

t_cpu cpu_inicializar(t_config *config)
{
    t_cpu cpu;
    cpu.puertoEscuchaDispatch = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH");
    cpu.puertoEscuchaInterrupt = config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    cpu.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    cpu.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
    cpu.cantidadEntradasTlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    cpu.algoritmoTlb = config_get_string_value(config, "ALGORITMO_TLB");
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
};

/*--------EntradaSalida--------*/

t_entradasalida entradasalida_inicializar(t_config *config)
{
    t_entradasalida entradasalida;
    entradasalida.blockCount = config_get_int_value(config, "BLOCK_COUNT");
    entradasalida.blockSize = config_get_int_value(config, "BLOCK_SIZE");
    entradasalida.ipKernel = config_get_string_value(config, "IP_KERNEL");
    entradasalida.ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    entradasalida.pathBaseDialFs = config_get_string_value(config, "PATH_BASE_DIALFS");
    entradasalida.puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
    entradasalida.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
    entradasalida.tiempoUnidadDeTrabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    entradasalida.tipoInterfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    return entradasalida;
};

void entradasalida_log(t_entradasalida entradasalida, t_log *logger)
{
    log_info(logger, "BLOCK_COUNT: %d", entradasalida.blockCount);
    log_info(logger, "BLOCK_SIZE: %d", entradasalida.blockSize);
    log_info(logger, "IP_KERNEL: %s", entradasalida.ipKernel);
    log_info(logger, "IP_MEMORIA: %s", entradasalida.ipMemoria);
    log_info(logger, "PATH_BASE_DIALFS: %s", entradasalida.pathBaseDialFs);
    log_info(logger, "PUERTO_KERNEL: %d", entradasalida.puertoKernel);
    log_info(logger, "PUERTO_MEMORIA: %d", entradasalida.puertoMemoria);
    log_info(logger, "TIEMPO_UNIDAD_TRABAJO: %d", entradasalida.tiempoUnidadDeTrabajo);
    log_info(logger, "TIPO_INTERFAZ: %s", entradasalida.tipoInterfaz);
};

/*--------MEMORIA--------*/
t_memoria memoria_inicializar(t_config *config)
{
    t_memoria memoria;
    memoria.puertoEscucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    memoria.tamMemoria = config_get_int_value(config, "TAM_MEMORIA");
    memoria.tamPagina = config_get_int_value(config, "TAM_PAGINA");
    memoria.pathInstrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    memoria.retardoRespuesta = config_get_int_value(config, "RETARDO_RESPUESTA");
    return memoria;
};

void memoria_log(t_memoria memoria, t_log *logger)
{
    log_info(logger, "PUERTO_ESCUCHA: %s", memoria.puertoEscucha);
    log_info(logger, "TAM_MEMORIA: %d", memoria.tamMemoria);
    log_info(logger, "TAM_PAGINA: %d", memoria.tamPagina);
    log_info(logger, "PATH_INSTRUCCIONES: %s", memoria.pathInstrucciones);
    log_info(logger, "RETARDO_RESPUESTA: %d", memoria.retardoRespuesta);
};