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
