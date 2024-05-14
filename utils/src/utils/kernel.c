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

t_diagrama_estados kernel_inicializar_estados(t_diagrama_estados *estados)
{
    t_list *new = malloc(sizeof(t_list));
    new = list_create();
    t_list *ready = malloc(sizeof(t_list));
    ready = list_create();
    t_list *exec = malloc(sizeof(t_list));
    exec = list_create();
    t_list *block = malloc(sizeof(t_list));
    block = list_create();
    t_list *exit = malloc(sizeof(t_list));
    exit = list_create();
    // Inicializar diccionario de procesos
    estados->procesos = dictionary_create();
    estados->buffer_procesos = dictionary_create();
    t_diagrama_estados diagrama = {
        .new = new,
        .ready = ready,
        .exec = exec,
        .block = block,
        .exit = exit,
        .procesos = estados->procesos,
        .mutex_exec_ready = PTHREAD_MUTEX_INITIALIZER,
        .mutex_ready_exec = PTHREAD_MUTEX_INITIALIZER};
    return diagrama;
}

t_pcb *kernel_nuevo_proceso(t_kernel *kernel, t_diagrama_estados *estados, t_log *logger, char *instrucciones)
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
    proceso_push_new(estados, nuevaPcb);

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
        enviar_paquete(finalizar, kernel->sockets.entrada_salida_generic);
        liberar_conexion(&kernel->sockets.entrada_salida_stdin);
    }

    if (kernel->sockets.entrada_salida_stdin > 0)
    {
        enviar_paquete(finalizar, kernel->sockets.entrada_salida_stdin);
        liberar_conexion(&kernel->sockets.entrada_salida_stdin);
    }

    if (kernel->sockets.entrada_salida_stdout > 0)
    {
        enviar_paquete(finalizar, kernel->sockets.entrada_salida_stdout);
        liberar_conexion(&kernel->sockets.entrada_salida_stdout);
    }

    if (kernel->sockets.entrada_salida_dialfs > 0)
    {
        enviar_paquete(finalizar, kernel->sockets.cpu_dispatch);
        liberar_conexion(&kernel->sockets.entrada_salida_dialfs);
    }

    // Les aviso que se termino el sistema y que se cierren de su lado
    enviar_paquete(finalizar, kernel->sockets.cpu_interrupt);
    enviar_paquete(finalizar, kernel->sockets.memoria);

    // Libero las conexiones en Kernel
    liberar_conexion(&kernel->sockets.cpu_dispatch);
    liberar_conexion(&kernel->sockets.cpu_interrupt);
    liberar_conexion(&kernel->sockets.memoria);

    // Si no fue iniciado la planificacion, debo marcarla como apagada y luego encenderla
    // hilo_planificador_detener(hiloArgs);
    // sem_post(&hiloArgs->kernel->planificador_iniciar);

    // Espero a que todos los hilos terminen, uno por cada hilo de atencion + hilo planificador
    // sem_wait(&kernel->sistema_finalizar); // Este es del hilo planificador pero hay que verificar que este prendido antes de esperarlo

    sem_wait(&kernel->sistema_finalizar); // Memoria
    sem_wait(&kernel->sistema_finalizar); // CPU Dispatch
    sem_wait(&kernel->sistema_finalizar); // CPU Interurpt
    sem_wait(&kernel->sistema_finalizar); // Entrada/Salida conector de hilos (se baja al romper socket server)

    sem_destroy(&kernel->sistema_finalizar);
    sem_destroy(&kernel->planificador_iniciar);
    sem_destroy(&kernel->log_lock);
    sem_destroy(&kernel->thread_log_lock);

    free(finalizar_kernel);
    eliminar_paquete(finalizar);
};

t_pcb *kernel_transicion_ready_exec(t_diagrama_estados *estados, t_kernel *kernel)
{
    pthread_mutex_lock(&estados->mutex_ready_exec);

    t_pcb *proceso = proceso_pop_ready(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_exec(estados, proceso);

    pthread_mutex_unlock(&estados->mutex_ready_exec);

    // Enviar paquete a cpu dispatch
    t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
    serializar_t_registros_cpu(&paquete, proceso->pid, proceso->registros_cpu);
    enviar_paquete(paquete, kernel->sockets.cpu_dispatch);
    free(paquete);

    return proceso;
};

t_pcb *kernel_transicion_exec_block(t_diagrama_estados *estados)
{
    t_pcb *proceso = proceso_pop_exec(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_block(estados, proceso);
    return proceso;
};

t_pcb *kernel_transicion_block_ready(t_diagrama_estados *estados, t_log *logger)
{
    t_pcb *proceso = proceso_pop_block(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_ready(estados, proceso, logger);
    return proceso;
};

t_pcb *kernel_transicion_exec_ready(t_diagrama_estados *estados, t_log *logger, t_kernel *kernel)
{
    pthread_mutex_lock(&estados->mutex_exec_ready);

    t_pcb *proceso = proceso_pop_exec(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_ready(estados, proceso, logger);

    pthread_mutex_unlock(&estados->mutex_exec_ready);
    if (strcmp(kernel->algoritmoPlanificador, "RR") == 0 || strcmp(kernel->algoritmoPlanificador, "VRR") == 0)
    {
        log_info(logger, "PID: <%d> - Desalojado por fin de Quantum", proceso->pid);
        // Enviar señal de fin de quantum
        // TODO: Falta agregar el motivo del interrupt en el paquete
        t_paquete *paquete = crear_paquete(KERNEL_CPU_INTERRUPCION);
        serializar_uint32_t(proceso->pid, paquete);
        enviar_paquete(paquete, kernel->sockets.cpu_interrupt);
        free(paquete);
    }
    return proceso;
};

void kernel_desalojar_proceso(t_diagrama_estados *estados, t_kernel *kernel, t_log *logger, t_pcb *proceso)
{
    // Kernel desalojara el proceso SI Y SOLO SI el proceso se encuentra en la cola de exec
    // Esto es para evitar que termine el quantum y el proceso se vaya a una io, este en estado block y se haga un llamado innecesario a memoria
    usleep(kernel->quantum * 1000);
    t_pcb *aux = list_get(estados->exec, 0);
    if (aux == NULL)
    {
        log_error(logger, "Se quiere desalojar un proceso pero no hay procesos en exec");
        return;
    }
    if (aux->pid != proceso->pid)
    {
        log_error(logger, "El proceso que se  extrajo de la cola de exec no coincide con el proceso que se quiere desalojar");
        return;
    }
    kernel_transicion_exec_ready(estados, logger, kernel);
}