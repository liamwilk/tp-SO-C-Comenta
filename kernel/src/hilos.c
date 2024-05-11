#include "hilos.h"

/*----CONSOLA INTERACTIVA----*/

void *hilos_atender_consola(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    imprimir_header();

    char *linea = NULL;
    char *current_dir = getcwd(NULL, 0);
    char prompt[PATH_MAX + 3];

    if (current_dir)
    {
        snprintf(prompt, sizeof(prompt), "\n%s> ", current_dir);
        free(current_dir);
    }
    else
    {
        snprintf(prompt, sizeof(prompt), "\n> ");
    }

    while (!hiloArgs->kernel_orden_apagado)
    {
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
                imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);

                // char *pathInstrucciones = separar_linea[1];
                //  ejecutar_sript(pathInstrucciones, hiloArgs->kernel, hiloArgs->estados, hiloArgs->logger);
                break;
            }
            else
            {
                imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "No se proporciono un path. Vuelva a intentar");
                break;
            }
        }
        case INICIAR_PROCESO:
        {
            if (separar_linea[1] == 0)
            {
                imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "No se proporciono un path. Vuelva a intentar");
                break;
            }
            else
            {
                imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
                char *pathInstrucciones = separar_linea[1];
                kernel_nuevo_proceso((hiloArgs->kernel), hiloArgs->estados, hiloArgs->logger, pathInstrucciones);
                break;
            }
        }
        case FINALIZAR_PROCESO:
        {
            if (separar_linea[1] == 0)
            {
                imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "Falta proporcionar un numero de PID para eliminar. Vuelva a intentar.");
                break;
            }
            else
            {
                imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
                bool existe = proceso_matar(hiloArgs->estados, separar_linea[1]);
                int pidReceived = atoi(separar_linea[1]);

                // TODO: Hay que eliminar el proceso de la cola  pertinente aca solo lo busca

                if (!existe)
                {
                    imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "El PID <%d> no existe", pidReceived);
                    break;
                }

                imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se elimino el proceso <%d>", pidReceived);

                t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
                t_kernel_memoria_finalizar_proceso *proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
                proceso->pid = pidReceived;
                serializar_t_kernel_memoria_finalizar_proceso(&paquete, proceso);
                enviar_paquete(paquete, hiloArgs->kernel->sockets.memoria);

                eliminar_paquete(paquete);
                free(proceso);
                break;
            }
        }
        case DETENER_PLANIFICACION:
        {
            int iniciar_planificador_valor;
            sem_getvalue(&hiloArgs->kernel->iniciar_planificador, &iniciar_planificador_valor);

            if (iniciar_planificador_valor == 0)
            {
                imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "No se puede detener la planificacion si no está iniciada");
                break;
            }

            imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s", separar_linea[0]);
            hilo_planificador_detener(hiloArgs);
            break;
        }
        case INICIAR_PLANIFICACION:
        {
            imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s", separar_linea[0]);

            // hilo_planificador_iniciar(hiloArgs);

            sem_post(&hiloArgs->kernel->iniciar_planificador);
            sem_post(&hiloArgs->kernel->iniciar_algoritmo);

            // planificacion_largo_plazo(hiloArgs->kernel, hiloArgs->estados, hiloArgs->logger);
            // planificacion_corto_plazo(hiloArgs->kernel, hiloArgs->estados, hiloArgs->logger);
            break;
        }
        case MULTIPROGRAMACION:
        {
            if (separar_linea[1] == 0)
            {
                imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "No se proporciono un numero para el grado de multiprogramacion. Vuelva a intentar");
                break;
            }
            else
            {
                imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
                int grado_multiprogramacion = atoi(separar_linea[1]);
                hiloArgs->kernel->gradoMultiprogramacion = grado_multiprogramacion;
                imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se cambio el grado de multiprogramacion a %d", grado_multiprogramacion);
                break;
            }
        }
        case PROCESO_ESTADO:
        {
            if (separar_linea[1] == 0)
            {
                imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "No se proporciono un PID para consultar el proceso. Vuelva a intentar");
                break;
            }
            else
            {
                imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
                break;
            }
        }
        case FINALIZAR_CONSOLA:
        {
            imprimir_log(hiloArgs, LOG_LEVEL_INFO, "Se solicito el apagado del Sistema Operativo");
            hiloArgs->kernel_orden_apagado = 1;

            /* TODO: Acá hay que limpiar hilos_args entero menos la parte de kernel
             que se necesita para enviar el mensaje de finalizar a los modulos. Luego si, finalizar el kernel. Luego limpiar la parte de kernel dentro de kernel_finalizar

            Desbloqueo el proceso hilo_planificador para que lea la señal de finalizacion de kernel_orden_apagado
            */

            int iniciar_planificador_valor;
            int iniciar_algoritmo_valor;

            sem_getvalue(&hiloArgs->kernel->iniciar_planificador, &iniciar_planificador_valor);
            sem_getvalue(&hiloArgs->kernel->iniciar_algoritmo, &iniciar_algoritmo_valor);

            if (iniciar_planificador_valor == 0)
            { // Si es cero, esta bloqueado el proceso. Tengo que desbloquearlo para apagar el sistema
                sem_post(&hiloArgs->kernel->iniciar_planificador);
            }

            if (iniciar_algoritmo_valor == 0)
            { // Si es cero, esta bloqueado el proceso. Tengo que desbloquearlo para apagar el sistema
                sem_post(&hiloArgs->kernel->iniciar_algoritmo);
            }

            sem_destroy(&hiloArgs->kernel->iniciar_planificador);
            sem_destroy(&hiloArgs->kernel->iniciar_algoritmo);

            imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Finalizando consola.");
            printf("\n");
            kernel_finalizar(hiloArgs->kernel);
            break;
        }
        default:
        {
            imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "Comando no reconocido. Vuelva a intentar");
            imprimir_comandos();
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

void hilos_planificador_inicializar(hilos_args *args, pthread_t thread_planificador)
{
    pthread_create(&thread_planificador, NULL, hilo_planificador, args);
    pthread_detach(thread_planificador);
}

void *hilo_planificador(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    switch (determinar_algoritmo(hiloArgs))
    {
    case FIFO:
    {
        imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Llegue al case de FIFO.");
        imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Algoritmo de planificacion: %s", hiloArgs->kernel->algoritmoPlanificador);
        while (1)
        {
            sem_wait(&hiloArgs->kernel->iniciar_algoritmo);

            // Si tengo que salir del planificador, rompo el bucle
            if (obtener_key_finalizacion_hilo(hiloArgs))
            {
                imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Finalizando planificador.");
                break;
            }

            // Si tengo que pausar, salto al proximo ciclo con continue y espero que vuelvan a activar el planificador
            if (!obtener_key_finalizacion_algoritmo(hiloArgs))
            {
                imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Planificacion FIFO pausada.");
                continue;
            }

            // Con esto me aseguro que el proximo se ejecute hasta que se reciba la señal de frenar
            sem_post(&hiloArgs->kernel->iniciar_algoritmo);

            imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Planificacion FIFO continua.");

            sleep(10);

            /* TODO: Manejar solamente la lógica de mover procesos de las colas de estados desde este hilo.

            Todo lo que sea de comunicación con otros módulos, como Memoria, CPU, IO, etc, se debe hacer en los hilos de esos módulos como respuesta a mensajes que se reciban a partir de un flujo activado por la ejecucción de comandos desde consola interactiva.

            El hilo_planificador es un orquestador interno de Kernel, no debe tener comunicación directa con los módulos externos. Cualquier cosa preguntame.
            */
        }
        break;
    }
    case RR:
    {
        imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Llegue al case de FIFO.");
        imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Algoritmo de planificacion: %s", hiloArgs->kernel->algoritmoPlanificador);
        while (1)
        {
            sem_wait(&hiloArgs->kernel->iniciar_algoritmo);

            // Si tengo que salir del planificador, rompo el bucle
            if (obtener_key_finalizacion_hilo(hiloArgs))
            {
                break;
            }

            // Si tengo que pausar, salto al proximo ciclo con continue y espero que vuelvan a activar el planificador
            if (!obtener_key_finalizacion_algoritmo(hiloArgs))
            {
                imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Planificacion FIFO pausada.");
                continue;
            }

            // Con esto me aseguro que el proximo se ejecute hasta que se reciba la señal de frenar
            sem_post(&hiloArgs->kernel->iniciar_algoritmo);

            imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Planificacion FIFO continua.");

            sleep(10);

            /* TODO: Manejar solamente la lógica de mover procesos de las colas de estados desde este hilo.

            Todo lo que sea de comunicación con otros módulos, como Memoria, CPU, IO, etc, se debe hacer en los hilos de esos módulos como respuesta a mensajes que se reciban a partir de un flujo activado por la ejecucción de comandos desde consola interactiva.

            El hilo_planificador es un orquestador interno de Kernel, no debe tener comunicación directa con los módulos externos. Cualquier cosa preguntame.
            */
        }
        break;
    }
    case VRR:
    {
        imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Llegue al case de FIFO.");
        imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Algoritmo de planificacion: %s", hiloArgs->kernel->algoritmoPlanificador);
        while (1)
        {
            sem_wait(&hiloArgs->kernel->iniciar_algoritmo);

            // Si tengo que salir del planificador, rompo el bucle
            if (obtener_key_finalizacion_hilo(hiloArgs))
            {
                break;
            }

            // Si tengo que pausar, salto al proximo ciclo con continue y espero que vuelvan a activar el planificador
            if (!obtener_key_finalizacion_algoritmo(hiloArgs))
            {
                imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Planificacion FIFO pausada.");
                continue;
            }

            // Con esto me aseguro que el proximo se ejecute hasta que se reciba la señal de frenar
            sem_post(&hiloArgs->kernel->iniciar_algoritmo);

            imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Planificacion FIFO continua.");

            sleep(10);

            /* TODO: Manejar solamente la lógica de mover procesos de las colas de estados desde este hilo.

            Todo lo que sea de comunicación con otros módulos, como Memoria, CPU, IO, etc, se debe hacer en los hilos de esos módulos como respuesta a mensajes que se reciban a partir de un flujo activado por la ejecucción de comandos desde consola interactiva.

            El hilo_planificador es un orquestador interno de Kernel, no debe tener comunicación directa con los módulos externos. Cualquier cosa preguntame.
            */
        }
        break;
    }
    default:
    {
        imprimir_log(hiloArgs, LOG_LEVEL_ERROR, "Algoritmo de planificacion no reconocido.");
        break;
    }
    }
    imprimir_log(hiloArgs, LOG_LEVEL_DEBUG, "Planificador finalizado.");
    pthread_exit(0);
}

void hilo_planificador_detener(hilos_args *hiloArgs)
{
    hiloArgs->kernel->continuar_planificador = false;
}

void hilo_planificador_iniciar(hilos_args *hiloArgs)
{
    hiloArgs->kernel->continuar_planificador = true;
}

int obtener_key_finalizacion_hilo(hilos_args *args)
{
    return args->kernel_orden_apagado;
}

int obtener_key_finalizacion_algoritmo(hilos_args *args)
{
    return args->kernel->continuar_planificador;
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

    kernel_sockets_agregar(hiloArgs->kernel, MEMORIA, socket);

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

void switch_case_memoria(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case MEMORIA_KERNEL_NUEVO_PROCESO:
    {
        t_memoria_kernel_proceso *proceso = deserializar_t_memoria_kernel_proceso(buffer);

        log_debug(logger, "Recibí la respuesta de Memoria acerca de la solicitud de nuevo proceso");
        log_debug(logger, "PID: %d", proceso->pid);
        log_debug(logger, "Cantidad de instrucciones: %d", proceso->cantidad_instrucciones);
        log_debug(logger, "Leido: %d", proceso->leido);
        if (proceso->leido)
        {
            // Enviar a cpu los registros
            // t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);
            t_pcb *pcb = proceso_buscar_new(args->estados, proceso->pid);
            // serializar_t_registros_cpu(&paquete, pcb->pid, pcb->registros_cpu);
            // enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
            // Buscar proceso
            pcb->memoria_aceptado = true;
            log_debug(logger, "Proceso PID:<%d> aceptado en memoria", pcb->pid);
            log_info(logger, "Se crea el proceso <%d> en NEW", pcb->pid);
        }
        else
        {
            log_error(logger, "Proceso PID:<%d> rechazado en memoria", proceso->pid);
            // Eliminar proceso de la cola de new
            proceso_revertir(args->estados, proceso->pid);
            log_warning(logger, "Proceso PID:<%d> eliminado de kernel", proceso->pid);
        };
        free(proceso);
        break;
    }
    default:
    {
        log_warning(args->logger, "[Memoria] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.memoria);
        break;
    }
    }
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

    kernel_sockets_agregar(hiloArgs->kernel, CPU_DISPATCH, socket);

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

    kernel_sockets_agregar(hiloArgs->kernel, CPU_INTERRUPT, socket);

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
    hilo_ejecutar_kernel(hiloArgs->kernel->sockets.cpu_interrupt, hiloArgs, "CPU Interrupt", switch_case_cpu_interrupt);
    pthread_exit(0);
};

void switch_case_cpu_dispatch(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case PLACEHOLDER:
        // Placeholder
        break;
    default:
    {
        log_warning(args->logger, "[CPU Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_dispatch);
        break;
    }
    }
}

void switch_case_cpu_interrupt(t_log *logger, t_op_code codigo_operacion, hilos_args *args, t_buffer *buffer)
{
    switch (codigo_operacion)
    {
    case CPU_KERNEL_PROCESO:
    {
        t_cpu_kernel_proceso *proceso = deserializar_t_cpu_kernel_proceso(buffer);

        log_debug(logger, "Recibí la respuesta de CPU acerca de la ejecucion de un proceso");
        log_debug(logger, "PID: %d", proceso->pid);
        log_debug(logger, "Cantidad de instrucciones ejecutadas: %d", proceso->registros->pc);
        log_debug(logger, "Ejecutado completo: %d", proceso->ejecutado);

        // TODO: En CPU implementar lo siguiente: ejecutado = 1 para ejecutado completo, 0 para interrumpido y pendiente, -1 para error.

        if (proceso->ejecutado)
        {
            log_debug(logger, "Proceso PID:<%d> ejecutado completo.", proceso->pid);

            log_debug(logger, "Le doy notificacion a Memoria para que elimine el proceso PID:<%d>", proceso->pid);

            t_paquete *paquete_finalizar = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
            t_kernel_memoria_finalizar_proceso *finalizar_proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
            finalizar_proceso->pid = proceso->pid;
            serializar_t_kernel_memoria_finalizar_proceso(&paquete_finalizar, finalizar_proceso);
            enviar_paquete(paquete_finalizar, args->kernel->sockets.memoria);
            eliminar_paquete(paquete_finalizar);
        }
        else
        {
            // Mover proceso a ready
            t_pcb *pcb = proceso_buscar_new(args->estados, proceso->pid);
            pcb->memoria_aceptado = false;
            proceso_push_ready(args->estados, pcb);
            log_info(logger, "Se mueve el proceso <%d> a READY", proceso->pid);
        }

        free(proceso);
        break;
    }
    case CPU_KERNEL_IO_GEN_SLEEP:
    {
        t_cpu_kernel_io_gen_sleep *sleep = deserializar_t_cpu_kernel_io_gen_sleep(buffer);

        log_debug(logger, "Recibí la solicitud de CPU para activar IO_GEN_SLEEP en la Interfaz %s por %d unidades de trabajo asociado al PID %d", sleep->interfaz, sleep->tiempo, sleep->pid);

        if (args->kernel->sockets.entrada_salida_generic == 0)
        {
            log_error(args->logger, "No se pudo enviar el paquete a IO Generic porque aun no se conecto.");
            break;
        }

        // TODO: Implementar mapeo de logica de IDs de IO a sockets de IO. Actualmente solo se envia a IO Generic.

        log_debug(args->logger, "Se envia un sleep de 10 segundos a IO Generic ID %s", sleep->interfaz);
        t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);

        t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
        unidad->pid = sleep->pid;
        unidad->unidad_de_trabajo = sleep->tiempo;

        serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);

        enviar_paquete(paquete, args->kernel->sockets.entrada_salida_generic);
        log_debug(args->logger, "Se envio el paquete a IO Generic");

        eliminar_paquete(paquete);
        free(unidad);

        break;
    }
    default:
    {
        log_warning(args->logger, "[CPU Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
        liberar_conexion(&args->kernel->sockets.cpu_interrupt);
        break;
    }
    }
}

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
            log_error(hiloArgs->logger, "Error al esperar conexion de modulo de Entrada/Salida");
            break;
        }

        t_handshake modulo = esperar_handshake_entrada_salida(hiloArgs->logger, socket_cliente);

        if (modulo == ERROR)
        {
            log_error(hiloArgs->logger, "Error al recibir handshake de modulo de Entrada/Salida");
            liberar_conexion(&socket_cliente);
            break;
        }

        hilos_io_args *io_args = malloc(sizeof(hilos_io_args));
        io_args->args = hiloArgs;
        io_args->socket = socket_cliente;

        switch (modulo)
        {
        case KERNEL_ENTRADA_SALIDA_GENERIC:
            log_debug(hiloArgs->logger, "Se conecto un modulo generico de entrada/salida");
            pthread_create(&hiloArgs->kernel->threads.thread_atender_entrada_salida_generic, NULL, hilos_atender_entrada_salida_generic, io_args);
            pthread_detach(hiloArgs->kernel->threads.thread_atender_entrada_salida_generic);
            break;
        case KERNEL_ENTRADA_SALIDA_STDIN:
            log_debug(hiloArgs->logger, "Se conecto un modulo de entrada/salida STDIN");
            pthread_create(&hiloArgs->kernel->threads.thread_atender_entrada_salida_stdin, NULL, hilos_atender_entrada_salida_stdin, io_args);
            pthread_detach(hiloArgs->kernel->threads.thread_atender_entrada_salida_stdin);
            break;
        case KERNEL_ENTRADA_SALIDA_STDOUT:
            log_debug(hiloArgs->logger, "Se conecto un modulo de entrada/salida STDOUT");
            pthread_create(&hiloArgs->kernel->threads.thread_atender_entrada_salida_stdout, NULL, hilos_atender_entrada_salida_stdout, io_args);
            pthread_detach(hiloArgs->kernel->threads.thread_atender_entrada_salida_stdout);
            break;
        case KERNEL_ENTRADA_SALIDA_DIALFS:
            log_debug(hiloArgs->logger, "Se conecto un modulo de entrada/salida DIALFS");
            pthread_create(&hiloArgs->kernel->threads.thread_atender_entrada_salida_dialfs, NULL, hilos_atender_entrada_salida_dialfs, io_args);
            pthread_detach(hiloArgs->kernel->threads.thread_atender_entrada_salida_dialfs);
            break;
        default:
            log_error(hiloArgs->logger, "Se conecto un modulo de entrada/salida desconocido");
            liberar_conexion(&socket_cliente);
            break;
        }
    }
    pthread_exit(0);
};

void *hilos_atender_entrada_salida_generic(void *args)
{
    char *modulo = "I/O Generic";

    hilos_io_args *io_args = (hilos_io_args *)args;

    io_args->args->kernel->sockets.entrada_salida_generic = io_args->socket;

    log_debug(io_args->args->logger, "%s conectado en socket %d", modulo, io_args->socket);

    while (1)
    {
        log_debug(io_args->args->logger, "Esperando paquete de %s en socket %d", modulo, io_args->socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, io_args->socket);

        if (paquete == NULL)
        {
            log_info(io_args->args->logger, "%s se desconecto del socket %d.", modulo, io_args->socket);
            break;
        }
        revisar_paquete(paquete, io_args->args->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case ENTRADA_SALIDA_KERNEL_IO_GEN_SLEEP:
        {
            t_entrada_salida_kernel_unidad_de_trabajo *unidad = deserializar_t_entrada_salida_kernel_unidad_de_trabajo(paquete->buffer);

            // Este mensaje es solo de efecto, no contiene ningun buffer de datos
            log_debug(io_args->args->logger, "Se recibio un mensaje de %s con la respuesta a sleep del PID %d:  %d", modulo, unidad->pid, unidad->terminado);

            free(unidad);
            break;
        }
        default:
        {
            log_warning(io_args->args->logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo);
            liberar_conexion(&io_args->socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }
    free(io_args);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_stdin(void *args)
{
    char *modulo = "I/O STDIN";

    hilos_io_args *io_args = (hilos_io_args *)args;

    io_args->args->kernel->sockets.entrada_salida_stdin = io_args->socket;

    log_debug(io_args->args->logger, "%s conectado en socket %d", modulo, io_args->socket);

    while (1)
    {
        log_debug(io_args->args->logger, "Esperando paquete de %s en socket %d", modulo, io_args->socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, io_args->socket);

        if (paquete == NULL)
        {
            log_info(io_args->args->logger, "%s se desconecto del socket %d.", modulo, io_args->socket);
            break;
        }

        revisar_paquete(paquete, io_args->args->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            /*
            La logica
            */
            break;
        }
        default:
        {
            log_warning(io_args->args->logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo);
            liberar_conexion(&io_args->socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }
    free(io_args);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_stdout(void *args)
{
    char *modulo = "I/O STDOUT";

    hilos_io_args *io_args = (hilos_io_args *)args;

    io_args->args->kernel->sockets.entrada_salida_stdout = io_args->socket;

    log_debug(io_args->args->logger, "%s conectado en socket %d", modulo, io_args->socket);

    while (1)
    {
        log_debug(io_args->args->logger, "Esperando paquete de %s en socket %d", modulo, io_args->socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, io_args->socket);

        if (paquete == NULL)
        {
            log_info(io_args->args->logger, "%s se desconecto del socket %d.", modulo, io_args->socket);
            break;
        }

        revisar_paquete(paquete, io_args->args->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            /*
            La logica
            */
            break;
        }
        default:
        {
            log_warning(io_args->args->logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo);
            liberar_conexion(&io_args->socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }
    free(io_args);
    pthread_exit(0);
}

void *hilos_atender_entrada_salida_dialfs(void *args)
{
    char *modulo = "I/O DialFS";

    hilos_io_args *io_args = (hilos_io_args *)args;

    io_args->args->kernel->sockets.entrada_salida_dialfs = io_args->socket;

    log_debug(io_args->args->logger, "%s conectado en socket %d", modulo, io_args->socket);

    while (1)
    {
        log_debug(io_args->args->logger, "Esperando paquete de %s en socket %d", modulo, io_args->socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, io_args->socket);

        if (paquete == NULL)
        {
            log_info(io_args->args->logger, "%s se desconecto del socket %d.", modulo, io_args->socket);
            break;
        }

        revisar_paquete(paquete, io_args->args->logger, modulo);

        switch (paquete->codigo_operacion)
        {
        case PLACEHOLDER:
        {
            /*
            La logica
            */
            break;
        }
        default:
        {
            log_warning(io_args->args->logger, "[%s] Se recibio un codigo de operacion desconocido. Cierro hilo", modulo);
            liberar_conexion(&io_args->socket);
            break;
        }
        }
        eliminar_paquete(paquete);
    }
    free(io_args);
    pthread_exit(0);
}