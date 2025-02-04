#include "hilos.h"

/*----CONSOLA INTERACTIVA----*/

void *hilos_atender_consola(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    imprimir_header(hiloArgs);

    char *comandos_archivo_historial = "comandos.log";
    int comandos_archivo_limite = 20;

    // Configurar la función de autocompletado
    rl_attempted_completion_function = autocompletado;

    // Limitar el tamaño del historial a HISTORY_LIMIT
    stifle_history(comandos_archivo_limite);

    // Leer el historial desde el archivo
    read_history(comandos_archivo_historial);

    // Registrar el manejador de señales para SIGWINCH
    registrar_manejador_senales();

    char *linea = NULL;

    while (hiloArgs->kernel_orden_apagado)
    {
        reiniciar_prompt(hiloArgs);
        char *prompt = generar_prompt();
        linea = readline(prompt);
        free(prompt);
        if (linea && *linea)
        {
            add_history(linea);
        }

        if (linea == NULL || *linea == '\0')
        {
            free(linea);
            continue;
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
        {
            iniciar_proceso(separar_linea, hiloArgs);
            break;
        }
        case FINALIZAR_PROCESO:
        {
            finalizar_proceso(separar_linea, hiloArgs);
            break;
        }
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
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Comando no reconocido: %s", separar_linea[0]);
            break;
        }
        }
        //  Libero el string_split de separar_linea
        for (int i = 0; separar_linea[i] != NULL; i++)
        {
            free(separar_linea[i]);
        }
        free(separar_linea);
        free(linea);
    }

    write_history(comandos_archivo_historial);
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

        planificacion_corto_plazo(hiloArgs);
    }
    sem_post(&hiloArgs->kernel->sistema_finalizar);
    pthread_exit(0);
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
    int socket = crear_conexion(hiloArgs->logger, hiloArgs->kernel->ipMemoria, hiloArgs->kernel->puertoMemoria);

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
    kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Conectado a Memoria en socket %d", socket);
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
    int socket = crear_conexion(hiloArgs->logger, hiloArgs->kernel->ipCpu, hiloArgs->kernel->puertoCpuDispatch);

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
    kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Conectado a CPU por Dispatch en socket %d", socket);
    pthread_exit(0);
};

void *hilos_conectar_cpu_interrupt(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    int socket = crear_conexion(hiloArgs->logger, hiloArgs->kernel->ipCpu, hiloArgs->kernel->puertoCpuInterrupt);

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
    kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Conectado a CPU por Interrupt en socket %d", socket);
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
    // TODO: Ver esto, si lo dejamos en 1 el hilo no se apaga nunca
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

        switch (modulo)
        {
        case KERNEL_ENTRADA_SALIDA_GENERIC:
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida generico con socket %d", socket_cliente);
            io_args->entrada_salida = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_GENERIC, socket_cliente);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_generic, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        }
        case KERNEL_ENTRADA_SALIDA_STDIN:
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida STDIN con socket %d", socket_cliente);
            io_args->entrada_salida = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_STDIN, socket_cliente);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_stdin, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        }
        case KERNEL_ENTRADA_SALIDA_STDOUT:
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida STDOUT con socket %d", socket_cliente);
            io_args->entrada_salida = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_STDOUT, socket_cliente);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_stdout, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        }
        case KERNEL_ENTRADA_SALIDA_DIALFS:
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida DialFS con socket %d", socket_cliente);
            io_args->entrada_salida = kernel_sockets_agregar_entrada_salida(hiloArgs, ENTRADA_SALIDA_DIALFS, socket_cliente);

            pthread_create(&thread_atender_entrada_salida, NULL, hilos_atender_entrada_salida_dialfs, io_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        }
        default:
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Se conecto un modulo de entrada/salida desconocido. Cerrando...");
            liberar_conexion(&socket_cliente);
            break;
        }
        }
    }

    sem_post(&hiloArgs->kernel->sistema_finalizar);
    pthread_exit(0);
};
