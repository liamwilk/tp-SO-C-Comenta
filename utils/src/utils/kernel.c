#include "kernel.h"

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

void kernel_log_generic(hilos_args *args, t_log_level nivel, const char *mensaje, ...)
{
    va_list args_list;
    va_start(args_list, mensaje);

    // Calcular el tamaño necesario para el mensaje formateado
    int needed_size = vsnprintf(NULL, 0, mensaje, args_list) + 1;

    // Crear un buffer para el mensaje formateado
    char *mensaje_formateado = malloc(needed_size);

    if (mensaje_formateado != NULL)
    {
        // Reiniciar 'args_list' para usarla de nuevo
        va_end(args_list);
        va_start(args_list, mensaje);

        // Formatear el mensaje
        vsnprintf(mensaje_formateado, needed_size, mensaje, args_list);

        // Llamar a la función que imprime el mensaje simple
        kernel_log_generic_rl(args, nivel, mensaje_formateado);

        // Liberar la memoria del mensaje formateado
        free(mensaje_formateado);
    }
    else
    {
        log_error(args->logger, "Error al reservar memoria para imprimir log");
    }

    va_end(args_list);
}

void kernel_log_generic_rl(hilos_args *args, t_log_level nivel, const char *mensaje)
{
    sem_wait(&args->kernel->log_lock);

    pthread_mutex_lock(&args->kernel->lock);

    // Guardar el estado actual de readline
    int saved_point = rl_point;
    char *saved_line = rl_copy_text(0, rl_end);

    // Limpiar la línea actual
    rl_save_prompt();
    rl_set_prompt("");
    rl_replace_line("", 0);

    pthread_mutex_unlock(&args->kernel->lock);

    // Muestro devuelta el prompt de readline
    rl_redisplay();

    // Imprimir el mensaje de log usando la función de log correspondiente
    switch (nivel)
    {
    case LOG_LEVEL_TRACE:
        log_trace(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_DEBUG:
        log_debug(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_INFO:
        log_info(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_WARNING:
        log_warning(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_ERROR:
        log_error(args->logger, "%s", mensaje);
        break;
    default:
        log_error(args->logger, "Nivel de log inválido en kernel_log_generic_rl");
        break;
    }

    pthread_mutex_lock(&args->kernel->lock);

    // Restaurar el estado de readline
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_restore_prompt();
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;

    pthread_mutex_unlock(&args->kernel->lock);

    // Vuelvo a mostrar el prompt de readline
    rl_redisplay();

    // Libero el semaforo de log
    sem_post(&args->kernel->log_lock);
    free(saved_line);
}

void kernel_desalojar_proceso(hilos_args *kernel_hilos_args, t_pcb *pcb, int quantum)
{
    t_temporal *temporal = temporal_create();

    t_algoritmo ALGORITMO_ACTUAL = determinar_algoritmo(kernel_hilos_args->kernel->algoritmoPlanificador);

    char *algoritmoActual = ALGORITMO_ACTUAL == VRR ? "Virtual Round Robin" : "Round Robin";

    pcb->tiempo_fin = temporal_create();

    //!! Se guarda esta variable ya que puede suceder que durante el sleep el proceso sea eliminado y para mantener una referencia al pid
    uint32_t process_pid = pcb->pid;

    sleep(quantum / 1000);

    temporal_stop(temporal);

    char *estado = proceso_estado(kernel_hilos_args->estados, process_pid);

    if (strcmp(estado, "EXIT") == 0)
    {
        kernel_log_generic(kernel_hilos_args, ALGORITMO_ACTUAL == RR ? LOG_LEVEL_WARNING : LOG_LEVEL_DEBUG, "[%s] El proceso <%d> ya ha sido eliminado", algoritmoActual, process_pid);
        temporal_destroy(temporal);
        return;
    }
    if (pcb->tiempo_fin->status == TEMPORAL_STATUS_RUNNING)
    {
        temporal_stop(pcb->tiempo_fin);
    };

    int64_t diff = temporal_diff(temporal, pcb->tiempo_fin);

    if (strcmp(estado, "BLOCK") == 0 && diff > 0)
    {
        kernel_log_generic(kernel_hilos_args, ALGORITMO_ACTUAL == RR ? LOG_LEVEL_WARNING : LOG_LEVEL_DEBUG, "[%s] El proceso <%d> se encuentra bloqueado y le sobro <%ld> ms de quantum", algoritmoActual, pcb->pid, diff);
        temporal_destroy(temporal);
        return;
    }
    if (strcmp(estado, "READY") == 0 && diff > 0)
    {
        kernel_log_generic(kernel_hilos_args, ALGORITMO_ACTUAL == RR ? LOG_LEVEL_WARNING : LOG_LEVEL_DEBUG, "[%s] El proceso <%d> se encuentra ready y le sobro <%ld> ms de quantum", algoritmoActual, pcb->pid, diff);
        temporal_destroy(temporal);
        return;
    }
    if (strcmp(estado, "EXEC") == 0)
    {
        kernel_interrumpir_cpu(kernel_hilos_args, pid, "QUANTUM");
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Desalojado por fin de Quantum", pid);
        temporal_destroy(temporal);
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

t_pcb *kernel_transicion_exec_ready(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_exec_ready);

    t_pcb *proceso = proceso_pop_exec(kernel_hilos_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de exec a ready fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready);
        return NULL;
    }
    proceso_push_ready(kernel_hilos_args->estados, proceso);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <READY>", proceso->pid);
    // Log oficial de la catedra (En procedimiento)
    log_ready(kernel_hilos_args, false);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready);
    return proceso;
};

t_pcb *kernel_transicion_ready_exec(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_ready_exec);

    t_pcb *proceso = proceso_pop_ready(kernel_hilos_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de ready a exec fallida. No hay procesos en ready.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec);
        return NULL;
    }
    proceso_push_exec(kernel_hilos_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXEC>", proceso->pid);

    // Enviar paquete a cpu dispatch
    t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
    serializar_t_registros_cpu(&paquete, proceso->pid, proceso->registros_cpu);
    enviar_paquete(paquete, kernel_hilos_args->kernel->sockets.cpu_dispatch);
    free(paquete);

    return proceso;
};

t_pcb *kernel_transicion_block_ready(hilos_args *kernel_hilos_args, uint32_t pid)
{

    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_block_ready);
    t_pcb *proceso = proceso_remover_block(kernel_hilos_args->estados, pid);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de block a ready fallida. No hay procesos en block.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready);
        return NULL;
    }
    proceso_push_ready(kernel_hilos_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <BLOCK> - Estado Actual: <READY>", proceso->pid);

    // Log oficial de la catedra (En procedimiento)
    log_ready(kernel_hilos_args, false);
    return proceso;
};

t_pcb *kernel_transicion_exec_block(hilos_args *kernel_hilo_args)
{
    pthread_mutex_lock(&kernel_hilo_args->estados->mutex_exec_block);
    // Recorrer la lista de exec
    t_pcb *proceso = proceso_pop_exec(kernel_hilo_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de exec a block fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_block);
        return NULL;
    }
    proceso_push_block(kernel_hilo_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_block);
    // Log obligatorio de la catedra
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <BLOCK>", proceso->pid);
    return proceso;
};

t_pcb *kernel_transicion_exec_exit(hilos_args *kernel_hilo_args)
{
    pthread_mutex_lock(&kernel_hilo_args->estados->mutex_exec_exit);

    t_pcb *proceso = proceso_pop_exec(kernel_hilo_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_ERROR, "[ESTADOS/TRANSICION] Transicion de EXEC a EXIT fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_exit);
        return NULL;
    }
    proceso_push_exit(kernel_hilo_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_exit);

    // log oficial de la catedra
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <EXIT>", proceso->pid);

    // Se añade el signal para que siga ejecutando el planificador
    sem_post(&kernel_hilo_args->kernel->planificador_iniciar);

    return proceso;
};

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
            t_kernel_entrada_salida *modulo = entrada_salida_buscar_interfaz(args, interfaz);

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

t_pcb *kernel_transicion_new_ready(hilos_args *kernel_hilo_args)
{
    pthread_mutex_lock(&kernel_hilo_args->estados->mutex_new_ready);

    t_pcb *proceso = list_get(kernel_hilo_args->estados->new, 0);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_ERROR, "[ESTADOS/TRANSICION] Transicion de NEW a READY fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_new_ready);
        return NULL;
    }
    if (proceso->memoria_aceptado == false)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_WARNING, "No se puede mover el proceso PID: <%d> a ready, ya que no fue aceptado por memoria", proceso->pid);
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_new_ready);
        return NULL;
    }
    proceso_pop_new(kernel_hilo_args->estados);
    proceso_push_ready(kernel_hilo_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_new_ready);

    // log oficial de la catedra
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <NEW> - Estado Actual: <READY>", proceso->pid);

    // log oficial de la catedra (En procedimiento)
    log_ready(kernel_hilo_args, false);
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_DEBUG, "[PLANIFICADOR LARGO PLAZO] Proceso PID: <%d> movido a ready", proceso->pid);
    return proceso;
}

void log_ready(hilos_args *kernel_hilos_args, bool prioritaria)
{
    char *msg = prioritaria == false ? "Cola Ready : [" : "Cola Ready Mayor Prioridad : [";
    t_list *listaARecorrer = prioritaria == false ? kernel_hilos_args->estados->ready : kernel_hilos_args->estados->ready_mayor_prioridad;
    // Iterate over ready
    for (int i = 0; i < list_size(listaARecorrer); i++)
    {
        t_pcb *pcb = list_get(listaARecorrer, i);
        char *pid = string_itoa(pcb->pid);
        msg = string_from_format("%s %s", msg, pid);
    }
    msg = string_from_format("%s ]", msg);
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "%s", msg);
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
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_WARNING, "El proceso <%d> se encuentra en ejecucion, se procede a desalojarlo", pid);
            kernel_interrumpir_cpu(kernel_hilos_args, pid, "FINALIZAR_PROCESO");
            kernel_transicion_exec_exit(kernel_hilos_args);
            return false;
        }
        if (strcmp(estado, "BLOCK") == 0)
        {
            kernel_log_generic(kernel_hilos_args, LOG_LEVEL_WARNING, "El proceso <%d> se encuentra bloqueado, se procede a desbloquearlo", pid);
            kernel_transicion_block_exit(kernel_hilos_args, pid);
            kernel_interrumpir_io(kernel_hilos_args, pid, "FINALIZAR_PROCESO");
            return false;
        }

        // Esta en ready o en new por lo tanto se puede eliminar tranquilamente
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INTERRUPTED_BY_USER>", pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    case INVALID_INTERFACE:
    {
        t_pcb *pcb_en_exit = kernel_transicion_exec_exit(kernel_hilos_args);
        proceso_avisar_timer(kernel_hilos_args->kernel->algoritmoPlanificador, pcb_en_exit);
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INVALID_INTERFACE>", pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    case SUCCESS:
    {
        kernel_transicion_exec_exit(kernel_hilos_args);
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <SUCCESS>", pid);
        proceso_matar(kernel_hilos_args->estados, string_itoa(pid));
        return true;
    }
    default:

        return false;
    }
}

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

t_consola_operacion obtener_operacion(char *funcion)
{
    char *operacionesStrings[] = {
        "PROCESO_ESTADO",
        "EJECUTAR_SCRIPT",
        "INICIAR_PROCESO",
        "MULTIPROGRAMACION",
        "FINALIZAR_PROCESO",
        "FINALIZAR",
        "DETENER_PLANIFICACION",
        "INICIAR_PLANIFICACION",
    };
    for (int i = 0; i < TOPE_ENUM_CONSOLA; i++)
    {
        if (strcmp(operacionesStrings[i], funcion) == 0)
        {
            return i;
        }
    }
    return TOPE_ENUM_CONSOLA;
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

char *generar_prompt()
{
    // Obtengo el nombre de usuario desde la variable de entorno "USER".
    const char *username = getenv("USER");
    if (username == NULL)
    {
        // Si "USER" no está definida, obtengo el nombre de usuario a partir del UID.
        struct passwd *pw = getpwuid(getuid());
        username = pw->pw_name;
    }

    // Obtengo el nombre del host
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    // Obtengo el directorio de trabajo actual (cwd)
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        // Manejo el error si falló la obtención del cwd.
        perror("getcwd() error");
        return NULL;
    }

    // Obtengo el directorio home del usuario.
    const char *home_dir = getenv("HOME");
    char primera_letra = '\0';

    // Si el cwd comienza con el home_dir, reemplazar el home_dir con "~".
    if (home_dir != NULL && strstr(cwd, home_dir) == cwd)
    {
        size_t home_len = strlen(home_dir);
        memmove(cwd + 1, cwd + home_len, strlen(cwd) - home_len + 1);
        cwd[0] = '~';

        // Guardar la primera letra del directorio que sigue al home.
        if (cwd[1] != '\0' && cwd[1] != '/')
        {
            primera_letra = cwd[1];
        }
        else if (cwd[2] != '\0')
        {
            char *next_dir = strchr(cwd + 2, '/');
            if (next_dir != NULL)
            {
                primera_letra = cwd[2];
            }
        }
    }

    // Obtengo el ancho de la terminal
    int ancho_terminal = obtener_ancho_terminal();
    if (ancho_terminal == -1)
    {
        // Valor predeterminado en caso de error
        ancho_terminal = 80;
    }
    // Calculo el margen extra basado en un porcentaje del ancho de la terminal
    double margen_porcentaje = 0.50;
    int margen_extra = (int)(ancho_terminal * margen_porcentaje);

    // Calcular la longitud máxima permitida para el prompt
    int prompt_max_length = ancho_terminal - 10 - margen_extra;

    // Ajusto el cwd si es demasiado largo
    if (strlen(cwd) > prompt_max_length)
    {
        char *last_slash = strrchr(cwd, '/');
        if (last_slash != NULL && last_slash != cwd)
        {
            // Muevo el contenido después del último slash a un buffer temporal
            char temp[PATH_MAX / 2];
            strncpy(temp, last_slash + 1, sizeof(temp) - 1);

            // Me aseguro la terminación nula del buffer temporal
            temp[sizeof(temp) - 1] = '\0';

            // Me aseguro que la longitud total no exceda el tamaño del buffer cwd
            size_t max_temp_length = sizeof(cwd) - 4;
            if (strlen(temp) > max_temp_length)
            {
                temp[max_temp_length] = '\0';
            }

            // Construyo la nueva cadena cwd con la primera letra y el contenido del buffer temporal
            snprintf(cwd, sizeof(cwd), "~/%c/%s", primera_letra, temp);
        }
        else
        {
            // Si no hay un slash, construyo una cadena indicando el directorio padre
            snprintf(cwd, sizeof(cwd), "~/../%s", last_slash + 1);
        }
    }

    // Defino los códigos de color para el prompt

    const char *green_bold = "\001\x1b[1;32m\002";
    const char *blue_bold = "\001\x1b[1;34m\002";
    const char *reset = "\001\x1b[0m\002";

    // Reservo memoria para el prompt completo
    char *prompt = malloc(PATH_MAX + HOST_NAME_MAX + 50);
    if (prompt == NULL)
    {
        // Manejo el error si falla la asignación de memoria
        perror("malloc() error");
        return NULL;
    }

    // Construyo el prompt
    snprintf(prompt, PATH_MAX + HOST_NAME_MAX + 50, "%s%s@%s%s:%s%s%s$ ", green_bold, username, hostname, reset, blue_bold, cwd, reset);

    return prompt;
}

char *autocompletado_instruccion(const char *input_text, int state)
{
    // Lista de comandos disponibles para autocompletar
    static char *comandos[] = {
        "EJECUTAR_SCRIPT",
        "INICIAR_PROCESO",
        "FINALIZAR_PROCESO",
        "DETENER_PLANIFICACION",
        "INICIAR_PLANIFICACION",
        "MULTIPROGRAMACION",
        "PROCESO_ESTADO",
        "FINALIZAR",
        NULL};

    static int command_index, input_length;
    char *command;

    // Inicialización en la primera llamada (state == 0)
    if (state == 0)
    {
        command_index = 0;                 // Reinicia el índice de los comandos
        input_length = strlen(input_text); // Longitud del texto ingresado
    }

    // Recorre la lista de comandos buscando coincidencias
    while ((command = comandos[command_index++]))
    {
        // Devuelve el comando si coincide con el texto ingresado
        if (strncmp(command, input_text, input_length) == 0)
        {
            return strdup(command); // Retorna una copia del comando encontrado
        }
    }

    return NULL; // No se encontraron más comandos coincidentes
}

char *autocompletado_argumento(const char *input_text, int state)
{
    static DIR *dir;
    static struct dirent *entry;
    static int input_length;

    // Inicializa en la primera llamada (state == 0)
    if (state == 0)
    {
        if (dir)
        {
            closedir(dir);
        }
        dir = opendir(".");
        input_length = strlen(input_text);
    }

    // Recorre el directorio buscando coincidencias
    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, input_text, input_length) == 0)
        {
            // Retorna una copia del nombre del archivo/directorio encontrado
            return strdup(entry->d_name);
        }
    }

    // No se encontraron más archivos/directorios coincidentes
    closedir(dir);
    dir = NULL;
    return NULL;
}

char **autocompletado(const char *text, int start, int end)
{
    // Determinar si es un comando o un parametro
    if (start == 0)
    {
        // Completando un comando
        rl_completion_append_character = ' ';
        return rl_completion_matches(text, autocompletado_instruccion);
    }
    else
    {
        // Completando un parámetro (archivo/directorio)
        rl_completion_append_character = '\0';
        return rl_completion_matches(text, autocompletado_argumento);
    }
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

void actualizar_prompt(int signal)
{
    // Borro el prompt anterior
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();

    // Genero el nuevo prompt
    rl_set_prompt(generar_prompt());
    rl_on_new_line();
    rl_redisplay();
}

int obtener_ancho_terminal()
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
    {
        perror("ioctl");
        return -1;
    }
    return w.ws_col;
}

void reiniciar_prompt(hilos_args *hiloArgs)
{
    sem_wait(&hiloArgs->kernel->log_lock);
    pthread_mutex_lock(&hiloArgs->kernel->lock);
    rl_set_prompt("");
    rl_replace_line("", 0);
    pthread_mutex_unlock(&hiloArgs->kernel->lock);
    rl_redisplay();
    sem_post(&hiloArgs->kernel->log_lock);
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

void kernel_interrumpir_io(hilos_args *args, uint32_t pid, char *motivo)
{
    t_paquete *paquete = crear_paquete(KERNEL_IO_INTERRUPCION);
    t_kernel_io_interrupcion interrupcion = {.pid = pid, .motivo = motivo, .len_motivo = strlen(motivo) + 1};
    serializar_t_kernel_io_interrupcion(&paquete, &interrupcion);

    // Buscar la io correspondiente y su socket
    t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz(args, pid);
    enviar_paquete(paquete, io->socket);
    free(paquete);
}

void kernel_transicion_block_exit(hilos_args *kernel_hilos_args, uint32_t pid)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_block_exit);

    t_pcb *proceso = proceso_remover_block(kernel_hilos_args->estados, pid);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS/TRANSICION] Transicion de BLOCK a EXIT fallida. PID <%d> no encontrado en la cola de block", pid);
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_exit);
        return;
    }

    proceso_push_exit(kernel_hilos_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_exit);

    // log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <BLOCK> - Estado Actual: <EXIT>", proceso->pid);
}

t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, uint32_t pid)
{
    for (int i = 0; i < list_size(args->kernel->sockets.list_entrada_salida); i++)
    {
        t_kernel_entrada_salida *modulo = list_get(args->kernel->sockets.list_entrada_salida, i);
        if (modulo->pid == pid)
        {
            return modulo;
        }
    }
    return NULL;
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

t_pcb *kernel_transicion_ready_exec_mayor_prioridad(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_ready_exec_mayor_prioridad);

    t_pcb *proceso = proceso_pop_cola_prioritaria(kernel_hilos_args->estados);

    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de ready a exec fallida. No hay procesos en cola prioritaria");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec_mayor_prioridad);
        return NULL;
    }

    proceso_push_exec(kernel_hilos_args->estados, proceso);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_ready_exec_mayor_prioridad);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <READY_PRIORIDAD> - Estado Actual: <EXEC>", proceso->pid);

    // Enviar paquete a cpu dispatch
    t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
    serializar_t_registros_cpu(&paquete, proceso->pid, proceso->registros_cpu);
    enviar_paquete(paquete, kernel_hilos_args->kernel->sockets.cpu_dispatch);
    free(paquete);

    return proceso;
}
t_pcb *kernel_transicion_exec_ready_mayor_prioridad(hilos_args *kernel_hilos_args)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_exec_ready_mayor_prioridad);

    t_pcb *proceso = proceso_pop_exec(kernel_hilos_args->estados);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de exec a ready de mayor prioridad fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready_mayor_prioridad);
        return NULL;
    }
    proceso_push_cola_prioritaria(kernel_hilos_args->estados, proceso);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <EXEC> - Estado Actual: <READY_PRIORIDAD>", proceso->pid);

    // Log oficial de la catedra (En procedimiento)
    log_ready(kernel_hilos_args, true);
    log_ready(kernel_hilos_args, false);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready_mayor_prioridad);
    return proceso;
}

t_pcb *kernel_transicion_block_ready_mayor_prioridad(hilos_args *kernel_hilos_args, uint32_t pid)
{
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_block_ready_mayor_prioridad);
    t_pcb *proceso = proceso_remover_block(kernel_hilos_args->estados, pid);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_ERROR, "[ESTADOS] Transicion de block a ready de mayor prioridad fallida. No hay procesos en block.");
        pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready_mayor_prioridad);
        return NULL;
    }
    proceso_push_cola_prioritaria(kernel_hilos_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_block_ready_mayor_prioridad);

    // Log oficial de la catedra
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <BLOCK> - Estado Actual: <READY_PRIORIDAD>", proceso->pid);

    // Log oficial de la catedra (En procedimiento)
    log_ready(kernel_hilos_args, true);
    log_ready(kernel_hilos_args, false);
    return proceso;
}

void kernel_manejar_ready(hilos_args *args, uint32_t pid, t_transiciones_ready TRANSICION)
{

    switch (TRANSICION)
    {
    case EXEC_READY:
        t_pcb *pcb = proceso_buscar_exec(args->estados, pid);

        if (pcb == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "[KERNEL/MANEJAR_EXEC_READY] Se quiere buscar el proceso <%d> en exec y no se encuentra, posible condicion de carrera", pid);
        }
        if (determinar_algoritmo(args->kernel->algoritmoPlanificador) == FIFO)
        {
            kernel_transicion_exec_ready(args);
            return;
        }
        proceso_avisar_timer(args->kernel->algoritmoPlanificador, pcb);
        if (pcb->tiempo_fin->status == TEMPORAL_STATUS_RUNNING)
        {
            // Esto no tendria que pasar nunca pero por las dudas
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "[VIRTUAL_ROUND_ROBIN/TIMER] El timer sigue en ejecucion, no se puede mover el proceso <%d> a ready", pid);
            return;
        }

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "El PID: <%d> ejecuto <%ld> ms", pcb->pid, pcb->tiempo_fin->elapsed_ms);
        if (proceso_tiene_prioridad(args->kernel->algoritmoPlanificador, args->kernel->quantum, pcb->tiempo_fin->elapsed_ms))
        {
            kernel_transicion_exec_ready_mayor_prioridad(args);
        }
        else
        {
            kernel_transicion_exec_ready(args);
        }
        break;
    case BLOCK_READY:
        pcb = proceso_buscar_block(args->estados, pid);
        if (pcb == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_ERROR, "[KERNEL/MANEJAR_BLOCK_READY] Se quiere buscar el proceso <%d> en block y no se encuentra, posible condicion de carrera", pid);
            break;
        }
        if (determinar_algoritmo(args->kernel->algoritmoPlanificador) == FIFO)
        {
            // El mismo algoritmo lo maneja
            kernel_transicion_block_ready(args, pid);
            break;
        }
        proceso_avisar_timer(args->kernel->algoritmoPlanificador, pcb);

        if (pcb->tiempo_fin->status == TEMPORAL_STATUS_RUNNING)
        {
            // Esto no tendria que pasar nunca pero por las dudas
            kernel_log_generic(args, LOG_LEVEL_DEBUG, "[VIRTUAL_ROUND_ROBIN/TIMER] El timer sigue en ejecucion, no se puede mover el proceso <%d> a ready", pid);
            break;
        }

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "El PID: <%d> ejecuto <%ld> ms", pcb->pid, pcb->tiempo_fin->elapsed_ms);

        if (proceso_tiene_prioridad(args->kernel->algoritmoPlanificador, args->kernel->quantum, pcb->tiempo_fin->elapsed_ms))
        {
            kernel_transicion_block_ready_mayor_prioridad(args, pid);
        }
        else
        {
            kernel_transicion_block_ready(args, pid);
        }
        break;
    default:
        kernel_log_generic(args, LOG_LEVEL_ERROR, "[KERNEL/MANEJAR_READY] Transicion no reconocida");
        break;
    }
}
