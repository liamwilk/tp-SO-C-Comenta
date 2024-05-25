#include "hilos.h"

/*----CONSOLA INTERACTIVA----*/

void *hilos_atender_consola(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    imprimir_header(hiloArgs);

    const char *username = getenv("USER");
    if (username == NULL)
    {
        struct passwd *pw = getpwuid(getuid());
        username = pw->pw_name;
    }

    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
    }

    const char *home_dir = getenv("HOME");
    if (home_dir != NULL && strstr(cwd, home_dir) == cwd)
    {
        size_t home_len = strlen(home_dir);
        memmove(cwd + 1, cwd + home_len, strlen(cwd) - home_len + 1);
        cwd[0] = '~';
    }

    char prompt[PATH_MAX + HOST_NAME_MAX + 50];
    const char *green_bold = "\001\x1b[1;32m\002";
    const char *blue_bold = "\001\x1b[1;34m\002";
    const char *reset = "\001\x1b[0m\002";

    snprintf(prompt, sizeof(prompt), "%s%s@%s%s:%s%s%s$ ", green_bold, username, hostname, reset, blue_bold, cwd, reset);

    char *linea = NULL;

    while (hiloArgs->kernel_orden_apagado)
    {
        sem_wait(&hiloArgs->kernel->log_lock);
        pthread_mutex_lock(&hiloArgs->kernel->lock);
        rl_set_prompt("");
        rl_replace_line("", 0);
        pthread_mutex_unlock(&hiloArgs->kernel->lock);
        rl_redisplay();
        sem_post(&hiloArgs->kernel->log_lock);

        linea = readline(prompt);
        if (linea && *linea)
        {
            add_history(linea);
        }
        char **separar_linea = string_split(linea, " ");
        switch (obtener_operacion(separar_linea[0]))
        {
        case EJECUTAR_SCRIPT:
        {
            if (!separar_linea[1] == 0)
            {
                kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);

                char *pathInstrucciones = separar_linea[1];
                ejecutar_script(pathInstrucciones, hiloArgs);
                break;
            }
            else
            {
                kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "No se proporciono un path. Vuelva a intentar");
                break;
            }
        }
        case INICIAR_PROCESO:
            iniciar_proceso(separar_linea, hiloArgs);
            break;
        case FINALIZAR_PROCESO:
            finalizar_proceso(separar_linea, hiloArgs);
            break;
        case DETENER_PLANIFICACION:
        {
            detener_planificacion(separar_linea, hiloArgs);
            break;
        }
        case INICIAR_PLANIFICACION:
        {
            iniciar_planificacion(separar_linea, hiloArgs);
            break;
        }
        case MULTIPROGRAMACION:
        {
            multiprogramacion(separar_linea, hiloArgs);
            break;
        }
        case PROCESO_ESTADO:
        {
            procesos_estados(hiloArgs);
            break;
        }
        case FINALIZAR_CONSOLA:
        {
            finalizar_consola(separar_linea, hiloArgs);
            break;
        }
        default:
        {
            break;
        }
        }
        free(separar_linea);
        free(linea);
    }
    pthread_exit(0);
}

void hilos_consola_inicializar(hilos_args *args, pthread_t thread_atender_consola)
{
    pthread_create(&thread_atender_consola, NULL, hilos_atender_consola, args);
    pthread_join(thread_atender_consola, NULL);
}

/*----PLANIFICACION----*/

void hilos_planificador_inicializar(hilos_args *args, pthread_t thread_planificador)
{
    pthread_create(&thread_planificador, NULL, hilo_planificador, args);
    pthread_detach(thread_planificador);
}

void *hilo_planificador(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    while (1)
    {
        sem_wait(&hiloArgs->kernel->planificador_hilo);

        if (!obtener_key_finalizacion_hilo(hiloArgs))
        {
            break;
        }

        kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Algoritmo de planificacion: %s", hiloArgs->kernel->algoritmoPlanificador);

        while (obtener_key_finalizacion_hilo(hiloArgs))
        {
            sem_wait(&hiloArgs->kernel->planificador_iniciar);

            if (!obtener_key_finalizacion_hilo(hiloArgs))
            {
                break;
            }

            // Si tengo que pausar, salto al proximo ciclo con continue y espero que vuelvan a activar el planificador
            if (obtener_key_detencion_algoritmo(hiloArgs))
            {
                kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Planificacion %s pausada.", hiloArgs->kernel->algoritmoPlanificador);
                continue;
            }

            if (list_size(hiloArgs->estados->new) > 0)
            {
                planificacion_largo_plazo(hiloArgs);
            }

            switch (determinar_algoritmo(hiloArgs))
            {
            case FIFO:
            {
                fifo(hiloArgs);
                break;
            }
            case RR:
            {
                round_robin(hiloArgs);
                break;
            }
            case VRR:
            {
                // TODO: algoritmo
                break;
            }
            default:
            {
                kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Algoritmo de planificacion no reconocido.");
                break;
            }
            }
        }
        sem_post(&hiloArgs->kernel->sistema_finalizar);
        pthread_exit(0);
    }
    return NULL;
}

int obtener_key_finalizacion_hilo(hilos_args *args)
{
    int i;
    pthread_mutex_lock(&args->kernel->lock);
    i = args->kernel_orden_apagado;
    pthread_mutex_unlock(&args->kernel->lock);
    return i;
}

int obtener_key_detencion_algoritmo(hilos_args *args)
{
    int i;
    pthread_mutex_lock(&args->kernel->lock);
    i = args->kernel->detener_planificador;
    pthread_mutex_unlock(&args->kernel->lock);
    return i;
}

t_algoritmo determinar_algoritmo(hilos_args *args)
{
    if (strcmp(args->kernel->algoritmoPlanificador, "FIFO") == 0)
    {
        return FIFO;
    }
    else if (strcmp(args->kernel->algoritmoPlanificador, "RR") == 0)
    {
        return RR;
    }
    else if (strcmp(args->kernel->algoritmoPlanificador, "VRR") == 0)
    {
        return VRR;
    }
    return -1;
}

/*----MEMORIA----*/

void hilos_memoria_inicializar(hilos_args *args, pthread_t thread_conectar_memoria, pthread_t thread_atender_memoria)
{
    pthread_create(&thread_conectar_memoria, NULL, hilos_conectar_memoria, args);
    pthread_join(thread_conectar_memoria, NULL);
    pthread_create(&thread_atender_memoria, NULL, hilos_atender_memoria, args);
    pthread_detach(thread_atender_memoria);
}

void *hilos_conectar_memoria(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    int socket = crear_conexion(hiloArgs->kernel->ipMemoria, hiloArgs->kernel->puertoMemoria);

    if (socket == -1)
    {
        pthread_exit(0);
    }

    kernel_sockets_agregar(hiloArgs, MEMORIA, socket);

    t_handshake resultado = crear_handshake(hiloArgs->logger, socket, MEMORIA_KERNEL, "Memoria");

    if (resultado != CORRECTO)
    {
        liberar_conexion(&socket);
        pthread_exit(0);
    }

    log_debug(hiloArgs->logger, "Conectado a Memoria en socket %d", socket);
    pthread_exit(0);
}

void *hilos_atender_memoria(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    hilo_ejecutar_kernel(hiloArgs->kernel->sockets.memoria, hiloArgs, "Memoria", switch_case_memoria);
    pthread_exit(0);
}

/*----CPU----*/

void hilos_cpu_inicializar(hilos_args *args, pthread_t thread_conectar_cpu_dispatch, pthread_t thread_atender_cpu_dispatch, pthread_t thread_conectar_cpu_interrupt, pthread_t thread_atender_cpu_interrupt)
{
    pthread_create(&thread_conectar_cpu_dispatch, NULL, hilos_conectar_cpu_dispatch, args);
    pthread_join(thread_conectar_cpu_dispatch, NULL);
    pthread_create(&thread_atender_cpu_dispatch, NULL, hilos_atender_cpu_dispatch, args);
    pthread_detach(thread_atender_cpu_dispatch);

    pthread_create(&thread_conectar_cpu_interrupt, NULL, hilos_conectar_cpu_interrupt, args);
    pthread_join(thread_conectar_cpu_interrupt, NULL);
    pthread_create(&thread_atender_cpu_interrupt, NULL, hilos_atender_cpu_interrupt, args);
    pthread_detach(thread_atender_cpu_interrupt);
}

void *hilos_conectar_cpu_dispatch(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    int socket = crear_conexion(hiloArgs->kernel->ipCpu, hiloArgs->kernel->puertoCpuDispatch);

    if (socket == -1)
    {
        pthread_exit(0);
    }

    kernel_sockets_agregar(hiloArgs, CPU_DISPATCH, socket);

    t_handshake resultado = crear_handshake(hiloArgs->logger, socket, CPU_DISPATCH_KERNEL, "CPU Dispatch");

    if (resultado != CORRECTO)
    {
        liberar_conexion(&socket);
        pthread_exit(0);
    }

    log_debug(hiloArgs->logger, "Conectado a CPU por Dispatch en socket %d", socket);
    pthread_exit(0);
};

void *hilos_conectar_cpu_interrupt(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    int socket = crear_conexion(hiloArgs->kernel->ipCpu, hiloArgs->kernel->puertoCpuInterrupt);

    if (socket == -1)
    {
        pthread_exit(0);
    }

    kernel_sockets_agregar(hiloArgs, CPU_INTERRUPT, socket);

    t_handshake resultado = crear_handshake(hiloArgs->logger, socket, CPU_INTERRUPT_KERNEL, "CPU Interrupt");

    if (resultado != CORRECTO)
    {
        liberar_conexion(&socket);
        pthread_exit(0);
    }

    log_debug(hiloArgs->logger, "Conectado a CPU por Interrupt en socket %d", socket);
    pthread_exit(0);
};

void *hilos_atender_cpu_dispatch(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    hilo_ejecutar_kernel(hiloArgs->kernel->sockets.cpu_dispatch, hiloArgs, "CPU Dispatch", switch_case_cpu_dispatch);
    pthread_exit(0);
};

void *hilos_atender_cpu_interrupt(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;

    // // Esto se usa para testear interrupt
    // sleep(20);
    // t_kernel_cpu_interrupcion *interrupcion = malloc(sizeof(t_kernel_cpu_interrupcion));
    // interrupcion->pid = 1;
    // t_paquete *paquete = crear_paquete(KERNEL_CPU_INTERRUPCION);
    // serializar_t_kernel_cpu_interrupcion(&paquete, interrupcion);
    // enviar_paquete(paquete, hiloArgs->kernel->sockets.cpu_interrupt);
    // free(interrupcion);

    hilo_ejecutar_kernel(hiloArgs->kernel->sockets.cpu_interrupt, hiloArgs, "CPU Interrupt", switch_case_cpu_interrupt);
    pthread_exit(0);
};

/*----IO----*/

void hilos_io_inicializar(hilos_args *args, pthread_t thread_esperar_entrada_salida)
{
    pthread_create(&thread_esperar_entrada_salida, NULL, hilos_esperar_entrada_salida, args);
    pthread_detach(thread_esperar_entrada_salida);
}

void *hilos_esperar_entrada_salida(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;

    while (1)
    {
        int socket_cliente = conexion_socket_recibir(hiloArgs->kernel->sockets.server);

        if (socket_cliente == -1)
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Error al esperar conexion de modulo de Entrada/Salida");
            break;
        }

        t_handshake modulo = esperar_handshake_entrada_salida(hiloArgs->logger, socket_cliente);

        if (modulo == ERROR)
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Error al recibir handshake de modulo de Entrada/Salida");
            liberar_conexion(&socket_cliente);
            break;
        }

        kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Conexion por handshake recibida y establecida con modulo de Entrada/Salida");

        pthread_t thread_atender_entrada_salida;
        hilos_io_args *io_args = malloc(sizeof(hilos_io_args));
        io_args->args = hiloArgs;
        io_args->entrada_salida = NULL;
        char *interfaz = NULL;

        switch (modulo)
        {
        case KERNEL_ENTRADA_SALIDA_GENERIC:
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida generico con socket %d", socket_cliente);
            interfaz = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_GENERIC, socket_cliente);
            io_args->entrada_salida = entrada_salida_buscar_interfaz(hiloArgs, interfaz);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_generic, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        case KERNEL_ENTRADA_SALIDA_STDIN:
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida STDIN con socket %d", socket_cliente);
            interfaz = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_STDIN, socket_cliente);
            io_args->entrada_salida = entrada_salida_buscar_interfaz(hiloArgs, interfaz);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_stdin, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        case KERNEL_ENTRADA_SALIDA_STDOUT:
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida STDOUT con socket %d", socket_cliente);
            interfaz = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_STDOUT, socket_cliente);
            io_args->entrada_salida = entrada_salida_buscar_interfaz(hiloArgs, interfaz);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_stdout, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        case KERNEL_ENTRADA_SALIDA_DIALFS:
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida DialFS con socket %d", socket_cliente);
            interfaz = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_DIALFS, socket_cliente);
            io_args->entrada_salida = entrada_salida_buscar_interfaz(hiloArgs, interfaz);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_dialfs, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        default:
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Se conecto un modulo de entrada/salida desconocido. Cerrando...");
            liberar_conexion(&socket_cliente);
            break;
        }
    }
    sem_post(&hiloArgs->kernel->sistema_finalizar);
    pthread_exit(0);
};

void *hilos_atender_entrada_salida_generic(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;

    char *modulo = "I/O Generic";
    char *interfaz = io_args->entrada_salida->interfaz;
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Conectado en socket %d", modulo, interfaz, orden, *socket);

    while (1)
    {
        kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, interfaz, orden, *socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, socket);

        if (paquete == NULL)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, interfaz, orden);
            break;
        }

        kernel_revisar_paquete(paquete, io_args->args, modulo);

        switch (paquete->codigo_operacion)
        {
        case ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP:
        {
            t_entrada_salida_kernel_unidad_de_trabajo *unidad = deserializar_t_entrada_salida_kernel_unidad_de_trabajo(paquete->buffer);

            if (unidad->terminado)
            {
                io_args->entrada_salida->ocupado = 0;
                kernel_transicion_block_ready(io_args, modulo, unidad);
                sem_post(&io_args->args->kernel->planificador_iniciar);
            }

            free(unidad);
            break;
        }
        default:
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/Interfaz %s/Orden %d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, interfaz, orden);
            liberar_conexion(socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, interfaz, orden);

    entrada_salida_remover_interfaz(io_args->args, interfaz);

    free(io_args);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_stdin(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;

    char *modulo = "I/O STDIN";
    char *interfaz = io_args->entrada_salida->interfaz;
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Conectado en socket %d", modulo, interfaz, orden, *socket);

    while (1)
    {
        kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, interfaz, orden, *socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, socket);

        if (paquete == NULL)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, interfaz, orden);
            break;
        }
        revisar_paquete(paquete, io_args->args->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            // Placeholder
            break;
        }
        default:
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/Interfaz %s/Orden %d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, interfaz, orden);
            liberar_conexion(socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, interfaz, orden);

    entrada_salida_remover_interfaz(io_args->args, interfaz);

    free(io_args);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_stdout(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;

    char *modulo = "I/O STDOUT";
    char *interfaz = io_args->entrada_salida->interfaz;
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Conectado en socket %d", modulo, interfaz, orden, *socket);

    while (1)
    {
        kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, interfaz, orden, *socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, socket);

        if (paquete == NULL)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, interfaz, orden);
            break;
        }
        revisar_paquete(paquete, io_args->args->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            // Placeholder
            break;
        }
        default:
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/Interfaz %s/Orden %d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, interfaz, orden);
            liberar_conexion(socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, interfaz, orden);

    entrada_salida_remover_interfaz(io_args->args, interfaz);

    free(io_args);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_dialfs(void *args)
{
    hilos_io_args *io_args = (hilos_io_args *)args;

    char *modulo = "I/O DialFS";
    char *interfaz = io_args->entrada_salida->interfaz;
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Conectado en socket %d", modulo, interfaz, orden, *socket);

    while (1)
    {
        kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, interfaz, orden, *socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, socket);

        if (paquete == NULL)
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, interfaz, orden);
            break;
        }
        revisar_paquete(paquete, io_args->args->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            // Placeholder
            break;
        }
        default:
        {
            kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/Interfaz %s/Orden %d] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo, interfaz, orden);
            liberar_conexion(socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, interfaz, orden);

    entrada_salida_remover_interfaz(io_args->args, interfaz);

    free(io_args);
    pthread_exit(0);
}
