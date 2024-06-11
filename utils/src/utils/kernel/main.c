#include "main.h"

/**FUNCIONES DE PROPOSITO GENERAL PARA KERNEL**/
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

    t_list *ready_mayor_prioridad = malloc(sizeof(t_list));
    ready_mayor_prioridad = list_create();

    // Inicializar diccionario de procesos
    estados->procesos = dictionary_create();
    t_diagrama_estados diagrama = {
        .new = new,
        .ready = ready,
        .exec = exec,
        .block = block,
        .exit = exit,
        .procesos = estados->procesos,
        .ready_mayor_prioridad = ready_mayor_prioridad, // VRR
        .mutex_exec_ready = PTHREAD_MUTEX_INITIALIZER,
        .mutex_ready_exec = PTHREAD_MUTEX_INITIALIZER,
        .mutex_block_ready = PTHREAD_MUTEX_INITIALIZER,
        .mutex_new_ready = PTHREAD_MUTEX_INITIALIZER,
        .mutex_block_exit = PTHREAD_MUTEX_INITIALIZER,
        .mutex_new = PTHREAD_MUTEX_INITIALIZER,
        .mutex_exec_ready_mayor_prioridad = PTHREAD_MUTEX_INITIALIZER,
        .mutex_ready_exec_mayor_prioridad = PTHREAD_MUTEX_INITIALIZER,
        .mutex_block_ready_mayor_prioridad = PTHREAD_MUTEX_INITIALIZER,
    };
    return diagrama;
}

void kernel_desalojar_proceso(hilos_args *kernel_hilos_args, t_pcb *pcb, int quantum)
{
    char *estado = proceso_estado(kernel_hilos_args->estados, pcb->pid);

    if (strcmp(estado, "EXEC") != 0)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "Se esta intentando desalojar el proceso <%d> pero no se encuentra en EXEC", pcb->pid);
        return;
    }
    iniciar_temporizador(kernel_hilos_args, quantum);
}

void kernel_finalizar(hilos_args *args)
{
    // Creo el paquete de finalizar
    t_paquete *finalizar = crear_paquete(FINALIZAR_SISTEMA);
    t_entrada_salida_kernel_finalizar *finalizar_kernel = malloc(sizeof(t_entrada_salida_kernel_finalizar));
    finalizar_kernel->terminado = true;
    actualizar_buffer(finalizar, sizeof(bool));
    serializar_t_entrada_salida_kernel_finalizar(&finalizar, finalizar_kernel);

    // Obtengo el valor del semaforo planificador_iniciar, y si está apagado, primero lo prendo para que se termine el hilo planificador y luego espero a que termine

    int value;
    sem_getvalue(&args->kernel->planificador_iniciar, &value);

    // Si el semaforo esta en 0, debo prenderlo para que el hilo planificador termine
    if (value == 0)
    {
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "El hilo planificador se encontraba bloqueado. Lo desbloqueo para poder finalizarlo.");
        sem_post(&args->kernel->planificador_iniciar);
    }

    // Espero a que termine el hilo planificador
    sem_wait(&args->kernel->sistema_finalizar);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Hilo planificador finalizado");

    // Recupero todas las interfaces que estan conectadas del diccionario (si estan en el diccionario, estan conectadas)
    t_list *conectados = dictionary_keys(args->kernel->sockets.dictionary_entrada_salida);

    int cantidad_entrada_salida = list_size(conectados);

    // Si hay entradas y salidas conectadas, debo esperar a que terminen antes de cerrar el sistema
    if (cantidad_entrada_salida > 0)
    {
        for (int i = 0; i < cantidad_entrada_salida; i++)
        {
            char *interfaz = list_get(conectados, i);
            char *interfaz_copia = strdup(interfaz);
            t_kernel_entrada_salida *modulo = kernel_entrada_salida_buscar_interfaz(args, interfaz);

            // Liberar la conexion rompe el hilo de entrada/salida y se auto-elimina.
            liberar_conexion(&modulo->socket);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se desconectó la interfaz de entrada salida %s", interfaz_copia);
            free(interfaz_copia);
        }

        // Destruyo la lista auxiliar generada a partir de las keys del diccionario
        list_destroy(conectados);
    }

    // Destruyo todo lo de entrada/salida
    list_destroy(args->kernel->sockets.list_entrada_salida);
    dictionary_destroy(args->kernel->sockets.dictionary_entrada_salida);

    // Bajo el servidor interno de atencion de I/O para no aceptar mas conexiones
    liberar_conexion(&args->kernel->sockets.server);

    // Les aviso a Memoria y CPU que se termine el sistema
    enviar_paquete(finalizar, args->kernel->sockets.cpu_interrupt);
    enviar_paquete(finalizar, args->kernel->sockets.memoria);

    // Libero las conexiones desde Kernel
    liberar_conexion(&args->kernel->sockets.cpu_dispatch);
    liberar_conexion(&args->kernel->sockets.cpu_interrupt);
    liberar_conexion(&args->kernel->sockets.memoria);

    // Memoria + CPU dispatch + CPU interrupt + el gestor de hilos de entrada/salida
    int cantidad_modulos = 4;

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Esperando a que se cierren los modulos: Memoria, CPU Dispatch, CPU Interrupt y Atencion de Entrada/Salida.");

    for (int i = 0; i < cantidad_modulos; i++)
    {
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Esperando a que se cierre un modulo del sistema.");
        sem_wait(&args->kernel->sistema_finalizar);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Un modulo del sistema termino de cerrar.");
    }

    // Borro el comando FINALIZAR de la seccion de texto de la consola antes de terminar.
    pthread_mutex_lock(&args->kernel->lock);
    rl_set_prompt("");
    rl_replace_line("", 0);
    pthread_mutex_unlock(&args->kernel->lock);

    rl_redisplay();

    pthread_mutex_destroy(&args->kernel->lock);

    sem_destroy(&args->kernel->sistema_finalizar);
    sem_destroy(&args->kernel->log_lock);

    free(finalizar_kernel);
    eliminar_paquete(finalizar);
};

void kernel_revisar_paquete(t_paquete *paquete, hilos_args *args, char *modulo)
{
    if (paquete->codigo_operacion != FINALIZAR_SISTEMA)
    {
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Paquete recibido de modulo %s", modulo);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Deserializado del paquete:");
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Codigo de operacion: %d", paquete->codigo_operacion);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Size del buffer en paquete: %d", paquete->size_buffer);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Deserializado del buffer:");
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Size del stream: %d", paquete->buffer->size);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Offset del stream: %d", paquete->buffer->offset);

        if (paquete->size_buffer != paquete->buffer->size + (2 * sizeof(uint32_t)))
        {
            kernel_log_generic(args, LOG_LEVEL_WARNING, "Error en el tamaño del buffer. Se esperaba %d y se recibio %ld", paquete->size_buffer, paquete->buffer->size + (2 * sizeof(uint32_t)));
        }
    }
    else
    {
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Kernel solicito el apagado del modulo.");
    }
}

void hilo_planificador_iniciar(hilos_args *args)
{
    pthread_mutex_lock(&args->kernel->lock);
    args->kernel->detener_planificador = false;
    pthread_mutex_unlock(&args->kernel->lock);
}

void hilo_planificador_estado(hilos_args *args, bool estado)
{
    pthread_mutex_lock(&args->kernel->lock);
    args->kernel->estado_planificador = estado;
    pthread_mutex_unlock(&args->kernel->lock);
}

void hilo_planificador_detener(hilos_args *args)
{
    pthread_mutex_lock(&args->kernel->lock);
    args->kernel->detener_planificador = true;
    pthread_mutex_unlock(&args->kernel->lock);
}

t_pcb *kernel_nuevo_proceso(hilos_args *args, t_diagrama_estados *estados, t_log *logger, char *instrucciones)
{
    pthread_mutex_lock(&estados->mutex_new);
    t_pcb *nuevaPcb = pcb_crear(logger, args->kernel->quantum);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PCB] Program Counter: %d", nuevaPcb->registros_cpu->pc);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PCB] Quantum: %d", nuevaPcb->quantum);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PCB] PID: %d", nuevaPcb->pid);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PROCESO] Instrucciones: %s", instrucciones);

    proceso_push_new(estados, nuevaPcb);
    pthread_mutex_unlock(&estados->mutex_new);

    /**----ENVIAR A MEMORIA----**/
    t_kernel_memoria_proceso *proceso = malloc(sizeof(t_kernel_memoria_proceso));
    proceso->path_instrucciones = strdup(instrucciones);
    proceso->pid = nuevaPcb->pid;
    proceso->size_path = strlen(instrucciones) + 1;
    proceso->program_counter = nuevaPcb->registros_cpu->pc;

    t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_NUEVO_PROCESO);
    serializar_t_kernel_memoria_proceso(&paquete, proceso);
    enviar_paquete(paquete, args->kernel->sockets.memoria);

    eliminar_paquete(paquete);
    free(proceso);

    return nuevaPcb;
}

void registrar_manejador_senales()
{
    struct sigaction sa;
    sa.sa_handler = actualizar_prompt;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGWINCH, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void *hilos_atender_entrada_salida_dialfs(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;
    hilos_ejecutar_entrada_salida(io_args, "I/O DialFS", switch_case_kernel_entrada_salida_dialfs);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_generic(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;
    hilos_ejecutar_entrada_salida(io_args, "I/O Generic", switch_case_kernel_entrada_salida_generic);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_stdin(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;
    hilos_ejecutar_entrada_salida(io_args, "I/O STDIN", switch_case_kernel_entrada_salida_stdin);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_stdout(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;
    hilos_ejecutar_entrada_salida(io_args, "I/O STDOUT", switch_case_kernel_entrada_salida_stdout);
    pthread_exit(0);
}

void kernel_wait(hilos_args *args, uint32_t pid, char *recursoSolicitado)
{

    t_recurso *recurso_encontrado = recurso_buscar(args->recursos, recursoSolicitado);
    t_pcb *proceso_en_exec = proceso_buscar_exec(args->estados, pid);

    if (recurso_encontrado == NULL)
    {
        proceso_en_exec->quantum = interrumpir_temporizador(args);
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro el recurso <%s> solicitado por CPU", recursoSolicitado);
        kernel_finalizar_proceso(args, pid, INVALID_RESOURCE);
        return;
    };

    if (recurso_encontrado->instancias < 0)
    {
        proceso_en_exec->quantum = interrumpir_temporizador(args);
        kernel_log_generic(args, LOG_LEVEL_INFO, "Se bloquea el proceso <%d> por falta de instancias del recurso <%s>", pid, recursoSolicitado);
        list_add(recurso_encontrado->procesos_bloqueados, proceso_en_exec);
        kernel_transicion_exec_block(args);
        return;
    }

    recurso_encontrado->instancias--;
    kernel_log_generic(args, LOG_LEVEL_INFO, "Se ocupo una instancia del recurso <%s> - Instancias restantes <%d>", recursoSolicitado, recurso_encontrado->instancias);
    kernel_manejar_ready(args, pid, EXEC_READY);
    return;
}

void kernel_signal(hilos_args *args, uint32_t pid, char *recurso, t_recurso_motivo_liberacion MOTIVO)
{
    t_recurso *recurso_encontrado = recurso_buscar(args->recursos, recurso);

    if (recurso_encontrado == NULL)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro el recurso <%s> solicitado por CPU", recurso);
        kernel_finalizar_proceso(args, pid, INVALID_RESOURCE);
    };

    recurso_encontrado->instancias++;

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se libero una instancia del recurso <%s> - Instancias restantes <%d>", recurso, recurso_encontrado->instancias);
    if (list_size(recurso_encontrado->procesos_bloqueados) > 0 && recurso_encontrado->instancias >= 0)
    {
        t_pcb *pcb = list_remove(recurso_encontrado->procesos_bloqueados, 0);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se desbloquea el proceso <%d> por liberacion de recurso <%s>", pcb->pid, recurso);
        if (MOTIVO == SIGNAL_RECURSO)
        {
            // Mando a ready el que esta bloqueado pues se libero un recurso
            kernel_manejar_ready(args, pcb->pid, BLOCK_READY);
        }
        else
        {
            // Solamente libero el recurso ya que lo mande a exit el proceso
            return;
        }
    }

    kernel_manejar_ready(args, pid, EXEC_READY);
    return;
}

bool kernel_finalizar_proceso(hilos_args *kernel_hilos_args, uint32_t pid, KERNEL_MOTIVO_FINALIZACION MOTIVO)
{
    char *estado = proceso_estado(kernel_hilos_args->estados, pid);
    if (estado == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "El PID <%d> no existe", pid);
        return false;
    }

    if (strcmp(estado, "EXIT") == 0)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "El PID <%d> YA ha sido eliminado", pid);
        return false;
    }

    switch (MOTIVO)
    {
    case INTERRUPTED_BY_USER:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            interrumpir_temporizador(kernel_hilos_args);
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_WARNING, "El proceso <%d> se encuentra en ejecucion, se procede a desalojarlo", pid);
            kernel_interrumpir_cpu(kernel_hilos_args, pid, "FINALIZAR_PROCESO");
            kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
            kernel_transicion_exec_exit(kernel_hilos_args);
            return false;
        }
        if (strcmp(estado, "BLOCK") == 0)
        {
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_WARNING, "El proceso <%d> se encuentra bloqueado, se procede a desbloquearlo", pid);
            kernel_transicion_block_exit(kernel_hilos_args, pid);
            //  Verificamos que el proceso este blockeado por recurso y no por I/O
            char *recurso_bloqueado = recurso_buscar_pid(kernel_hilos_args->recursos, pid);
            if (recurso_bloqueado != NULL)
            {
                kernel_log_generic(kernel_hilos_args, LOG_LEVEL_DEBUG, "El proceso <%d> se encuentra bloqueado por recurso <%s>", pid, recurso_bloqueado);
                kernel_signal(kernel_hilos_args, pid, recurso_bloqueado, INTERRUPCION);
            }
            else
            {
                kernel_interrumpir_io(kernel_hilos_args, pid, "FINALIZAR_PROCESO");
            }
            kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
            return false;
        }

        // Esta en ready o en new por lo tanto se puede eliminar tranquilamente
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INTERRUPTED_BY_USER>", pid);
        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    case INVALID_INTERFACE:
    {
        t_pcb *pcb_en_exit = kernel_transicion_exec_exit(kernel_hilos_args);
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INVALID_INTERFACE>", pcb_en_exit->pid);
        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pcb_en_exit->pid));
        return true;
    }
    case SUCCESS:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            kernel_transicion_exec_exit(kernel_hilos_args);
        }

        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <SUCCESS>", pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    case INVALID_RESOURCE:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            kernel_transicion_exec_exit(kernel_hilos_args);
        }
        else
        {
            kernel_transicion_block_exit(kernel_hilos_args, pid);
        }
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INVALID_RESOURCE>", pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
    }
    case OUT_OF_MEMORY:
    {
        if (strcmp(estado, "EXEC") == 0)
        {
            kernel_transicion_exec_exit(kernel_hilos_args);
        }
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <OUT_OF_MEMORY>", pid);
        kernel_avisar_memoria_finalizacion_proceso(kernel_hilos_args, pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    default:

        return false;
    }
}

void kernel_interrumpir_cpu(hilos_args *kernel_hilos_args, uint32_t pid, char *motivo)
{
    t_paquete *paquete = crear_paquete(KERNEL_CPU_INTERRUPCION);
    t_kernel_cpu_interrupcion interrupcion = {.pid = pid, .motivo = motivo, .len_motivo = strlen(motivo) + 1};
    serializar_t_kernel_cpu_interrupcion(&paquete, &interrupcion);
    enviar_paquete(paquete, kernel_hilos_args->kernel->sockets.cpu_interrupt);
    free(paquete);
}

void kernel_avisar_memoria_finalizacion_proceso(hilos_args *args, uint32_t pid)
{
    t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
    t_kernel_memoria_finalizar_proceso *proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
    proceso->pid = pid;
    serializar_t_kernel_memoria_finalizar_proceso(&paquete, proceso);
    enviar_paquete(paquete, args->kernel->sockets.memoria);
    eliminar_paquete(paquete);
    free(proceso);
}
void kernel_inicializar_temporizador(hilos_args *argumentos, timer_args_t *temporizador)
{
    // Configura la estructura sigevent
    argumentos->sev.sigev_notify = SIGEV_THREAD;
    argumentos->sev.sigev_value.sival_ptr = temporizador;
    argumentos->sev.sigev_notify_function = kernel_fin_quantum;
    argumentos->sev.sigev_notify_attributes = NULL;
}
void kernel_fin_quantum(union sigval arg)
{
    timer_args_t *timerArgs = (timer_args_t *)arg.sival_ptr;

    if (list_size(timerArgs->args->estados->exec) > 0)
    {
        t_pcb *pcb = list_get(timerArgs->args->estados->exec, 0);
        kernel_log_generic(timerArgs->args, LOG_LEVEL_INFO, "PID: <%d> - Desalojado por fin de Quantum", pcb->pid);
        kernel_interrumpir_cpu(timerArgs->args, pcb->pid, "FIN DE QUANTUM");
    }
}