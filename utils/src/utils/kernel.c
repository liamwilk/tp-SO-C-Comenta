#include "kernel.h"

t_kernel kernel_inicializar(t_config *config)
{
    t_kernel kernel = {
        .puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA"),
        .ipMemoria = config_get_string_value(config, "IP_MEMORIA"),
        .puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA"),
        .ipCpu = config_get_string_value(config, "IP_CPU"),
        .puertoCpuDispatch = config_get_int_value(config, "PUERTO_CPU_DISPATCH"),
        .puertoCpuInterrupt = config_get_int_value(config, "PUERTO_CPU_INTERRUPT"),
        .algoritmoPlanificador = config_get_string_value(config, "ALGORITMO_PLANIFICACION"),
        .quantum = config_get_int_value(config, "QUANTUM"),
        .recursos = config_get_string_value(config, "RECURSOS"),
        .instanciasRecursos = config_get_string_value(config, "INSTANCIAS_RECURSOS"),
        .gradoMultiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION"),
        .sockets = {0, 0, 0, 0}};
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
    log_info(logger, "ALGORITMO_PLANIFICACION: %s", kernel.algoritmoPlanificador);
    log_info(logger, "QUANTUM: %d", kernel.quantum);
    log_info(logger, "RECURSOS: %s", kernel.recursos);
    log_info(logger, "INSTANCIAS_RECURSOS: %s", kernel.instanciasRecursos);
    log_info(logger, "GRADO_MULTIPROGRAMACION: %d", kernel.gradoMultiprogramacion);
    printf("\n");
}

t_kernel_sockets kernel_sockets_agregar(t_kernel *kernel, KERNEL_SOCKETS type, int socket)
{
    switch (type)
    {
    case MEMORIA:
        kernel->sockets.memoria = socket;
        break;
    case CPU_DISPATCH:
        kernel->sockets.cpu_dispatch = socket;
        break;
    case CPU_INTERRUPT:
        kernel->sockets.cpu_interrupt = socket;
        break;
    case SERVER:
        kernel->sockets.server = socket;
        break;
    default:
        break;
    }
    return kernel->sockets;
};

diagrama_estados kernel_inicializar_estados(diagrama_estados *estados)
{
    t_new *new = malloc(sizeof(t_new));
    new->cola = list_create();
    new->diccionario = dictionary_create();
    t_ready *ready = malloc(sizeof(t_ready));
    ready->cola = list_create();
    ready->diccionario = dictionary_create();
    t_exec *exec = malloc(sizeof(t_exec));
    exec->cola = list_create();
    exec->diccionario = dictionary_create();
    t_block *block = malloc(sizeof(t_block));
    block->cola = list_create();
    block->diccionario = dictionary_create();
    t_exit *exit = malloc(sizeof(t_exit));
    exit->cola = list_create();
    exit->diccionario = dictionary_create();
    diagrama_estados diagrama = {
        .new = new,
        .ready = ready,
        .exec = exec,
        .block = block,
        .exit = exit};
    return diagrama;
}

t_pcb *kernel_nuevo_proceso(t_kernel *kernel, t_new *colaNew, t_log *logger, char *instrucciones)
{
    t_pcb *nuevaPcb = pcb_crear(logger, kernel->quantum);
    log_debug(logger, "[PCB] Program Counter: %d", nuevaPcb->registros_cpu->pc);
    log_debug(logger, "[PCB] Quantum: %d", nuevaPcb->quantum);
    log_debug(logger, "[PCB] PID: %d", nuevaPcb->pid);
    log_debug(logger, "[PROCESO] Instrucciones: %s", instrucciones);

    /**----ENVIAR A MEMORIA----**/
    t_kernel_memoria_proceso *proceso = malloc(sizeof(t_kernel_memoria_proceso));
    proceso->path_instrucciones = strdup(instrucciones);
    proceso->pid = nuevaPcb->pid;
    proceso->size_path = strlen(instrucciones) + 1;
    proceso->program_counter = nuevaPcb->registros_cpu->pc;

    t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_NUEVO_PROCESO);
    serializar_t_kernel_memoria_proceso(&paquete, proceso);
    enviar_paquete(paquete, kernel->sockets.memoria);

    eliminar_paquete(paquete);
    free(proceso);
    proceso_agregar_new(colaNew, nuevaPcb);

    return nuevaPcb;
}

/** FUNCIONES DE CONSOLA**/

void kernel_finalizar(t_kernel *kernel)
{
    t_paquete *finalizar = crear_paquete(FINALIZAR_SISTEMA);

    t_entrada_salida_kernel_finalizar *finalizar_kernel = malloc(sizeof(t_entrada_salida_kernel_finalizar));
    finalizar_kernel->terminado = true;
    actualizar_buffer(finalizar, sizeof(bool));
    serializar_t_entrada_salida_kernel_finalizar(&finalizar, finalizar_kernel);

    // Bajo el servidor de atencion de I/O para no aceptar mas conexiones
    liberar_conexion(&kernel->sockets.server);

    if (kernel->sockets.entrada_salida_generic > 0)
    {
        liberar_conexion(&kernel->sockets.entrada_salida_stdin);
    }

    if (kernel->sockets.entrada_salida_stdin > 0)
    {
        liberar_conexion(&kernel->sockets.entrada_salida_stdin);
    }

    if (kernel->sockets.entrada_salida_stdout > 0)
    {
        liberar_conexion(&kernel->sockets.entrada_salida_stdout);
    }

    if (kernel->sockets.entrada_salida_dialfs > 0)
    {
        liberar_conexion(&kernel->sockets.entrada_salida_dialfs);
    }

    // Les aviso que se termino el sistema y que se cierren de su lado
    enviar_paquete(finalizar, kernel->sockets.cpu_interrupt);
    enviar_paquete(finalizar, kernel->sockets.memoria);

    // Libero las conexiones en Kernel
    liberar_conexion(&kernel->sockets.cpu_dispatch);
    liberar_conexion(&kernel->sockets.cpu_interrupt);
    liberar_conexion(&kernel->sockets.memoria);

    eliminar_paquete(finalizar);
};