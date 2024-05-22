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
        .mutex_ready_exec = PTHREAD_MUTEX_INITIALIZER,
        .mutex_block_ready = PTHREAD_MUTEX_INITIALIZER,
        .mutext_new_ready = PTHREAD_MUTEX_INITIALIZER,
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

void kernel_desalojar_proceso(hilos_args *kernel_hilos_args)
{
    // Kernel desalojara el proceso SI Y SOLO SI el proceso se encuentra en la cola de exec
    // Esto es para evitar que termine el quantum y el proceso se vaya a una io, este en estado block y se haga un llamado innecesario a memoria
    usleep(kernel_hilos_args->kernel->quantum * 1000);
    kernel_transicion_exec_ready(kernel_hilos_args);
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
    log_ready(kernel_hilos_args);
    pthread_mutex_unlock(&kernel_hilos_args->estados->mutex_exec_ready);
    if (strcmp(kernel_hilos_args->kernel->algoritmoPlanificador, "RR") == 0 || strcmp(kernel_hilos_args->kernel->algoritmoPlanificador, "VRR") == 0)
    {
        // Log oficial de la catedra
        kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "PID: <%d> - Desalojado por fin de Quantum", proceso->pid);

        t_paquete *paquete = crear_paquete(KERNEL_CPU_INTERRUPCION);
        serializar_uint32_t(proceso->pid, paquete);
        enviar_paquete(paquete, kernel_hilos_args->kernel->sockets.cpu_interrupt);
        free(paquete);
    }
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

t_pcb *kernel_transicion_block_ready(hilos_io_args *io_args, char *modulo, t_entrada_salida_kernel_unidad_de_trabajo *unidad)
{
    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Se transiciona el PID %d a READY por finalizacion de I/O", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, unidad->pid);
    hilos_args *kernel_hilos_args = io_args->args;
    pthread_mutex_lock(&kernel_hilos_args->estados->mutex_block_ready);
    t_pcb *proceso = proceso_remover_block(kernel_hilos_args->estados, unidad->pid);
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
    log_ready(kernel_hilos_args);
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
    sem_destroy(&args->kernel->memoria_consola_nuevo_proceso);

    free(finalizar_kernel);
    eliminar_paquete(finalizar);
};

t_kernel_entrada_salida *entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(args->kernel->sockets.dictionary_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede buscar
    if (indice == NULL)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return NULL;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_kernel_entrada_salida *entrada_salida = list_get(args->kernel->sockets.list_entrada_salida, *indice);

    if (entrada_salida == NULL)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro la interfaz %s en la lista de entrada/salida", interfaz);
    }

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se encontro el modulo de entrada/salida en el socket %d asociado a la interfaz %s", entrada_salida->socket, interfaz);

    return entrada_salida;
}

t_pcb *kernel_transicion_new_ready(hilos_args *kernel_hilo_args)
{
    pthread_mutex_lock(&kernel_hilo_args->estados->mutext_new_ready);

    t_pcb *proceso = list_get(kernel_hilo_args->estados->new, 0);
    if (proceso == NULL)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_ERROR, "[ESTADOS/TRANSICION] Transicion de NEW a READY fallida. No hay procesos en exec.");
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutext_new_ready);
        return NULL;
    }
    if (proceso->memoria_aceptado == false)
    {
        kernel_log_generic(kernel_hilo_args, LOG_LEVEL_WARNING, "No se puede mover el proceso PID: <%d> a ready, ya que no fue aceptado por memoria", proceso->pid);
        pthread_mutex_unlock(&kernel_hilo_args->estados->mutext_new_ready);
        return NULL;
    }
    proceso_pop_new(kernel_hilo_args->estados);
    proceso_push_ready(kernel_hilo_args->estados, proceso);

    pthread_mutex_unlock(&kernel_hilo_args->estados->mutex_exec_exit);

    // log oficial de la catedra
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_INFO, "PID: <%d> - Estado Anterior: <NEW> - Estado Actual: <READY>", proceso->pid);

    // log oficial de la catedra (En procedimiento)
    log_ready(kernel_hilo_args);
    kernel_log_generic(kernel_hilo_args, LOG_LEVEL_DEBUG, "[PLANIFICADOR LARGO PLAZO] Proceso PID: <%d> movido a ready", proceso->pid);
    return proceso;
}

void log_ready(hilos_args *kernel_hilos_args)
{
    char *msg = "Cola Ready : [";
    // Iterate over ready
    for (int i = 0; i < list_size(kernel_hilos_args->estados->ready); i++)
    {
        t_pcb *pcb = list_get(kernel_hilos_args->estados->ready, i);
        char *pid = string_itoa(pcb->pid);
        msg = string_from_format("%s %s", msg, pid);
    }
    msg = string_from_format("%s ]", msg);
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "%s", msg);
}
