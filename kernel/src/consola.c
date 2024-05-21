#include "consola.h"
#include <utils/procesos.h>

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

t_pcb *buscar_proceso(t_diagrama_estados *estados, uint32_t pid)
{
    t_pcb *pcb = proceso_buscar_new(estados, pid);
    if (pcb == NULL)
    {
        pcb = proceso_buscar_ready(estados, pid);
        if (pcb == NULL)
        {
            pcb = proceso_buscar_exec(estados, pid);
            if (pcb == NULL)
            {
                pcb = proceso_buscar_block(estados, pid);
            }
        }
    }
    return pcb;
}

void imprimir_logo(hilos_args *args)
{

    log_generic(args, LOG_LEVEL_INFO, "              _                 _ _ _   _____ _____");
    log_generic(args, LOG_LEVEL_INFO, "             | |               | (_) | |  _  /  ___|");
    log_generic(args, LOG_LEVEL_INFO, " _ __ ___  __| | ___  _ __   __| |_| |_| | | \\ `--. ");
    log_generic(args, LOG_LEVEL_INFO, "| '__/ _ \\/ _` |/ _ \\| '_ \\ / _` | | __| | | |`--. \\");
    log_generic(args, LOG_LEVEL_INFO, "| | |  __/ (_| | (_) | | | | (_| | | |_\\ \\_/ /\\__/ /");
    log_generic(args, LOG_LEVEL_INFO, "|_|  \\___|\\__,_|\\___/|_| |_|\\__,_|_|\\__|\\___/\\____/");
    log_generic(args, LOG_LEVEL_INFO, " ");
    log_generic(args, LOG_LEVEL_INFO, "---------------------------------------------------------------");
    log_generic(args, LOG_LEVEL_INFO, "Implementación de C-Comenta - www.faq.utnso.com.ar/tp-c-comenta");
    log_generic(args, LOG_LEVEL_INFO, "Sistemas Operativos - 1C 2024 - UTN FRBA");
    log_generic(args, LOG_LEVEL_INFO, "---------------------------------------------------------------");
    log_generic(args, LOG_LEVEL_INFO, " ");
    // printf("              _                 _ _ _   _____ _____\n");
    // printf("             | |               | (_) | |  _  /  ___|\n");
    // printf(" _ __ ___  __| | ___  _ __   __| |_| |_| | | \\ `--. \n");
    // printf("| '__/ _ \\/ _` |/ _ \\| '_ \\ / _` | | __| | | |`--. \\\n");
    // printf("| | |  __/ (_| | (_) | | | | (_| | | |_\\ \\_/ /\\__/ /\n");
    // printf("|_|  \\___|\\__,_|\\___/|_| |_|\\__,_|_|\\__|\\___/\\____/ \n");
    // printf("\n");
    // printf("---------------------------------------------------------------\n");
    // printf("Implementación de C-Comenta - www.faq.utnso.com.ar/tp-c-comenta\n");
    // printf("Sistemas Operativos - 1C 2024 - UTN FRBA\n");
    // printf("---------------------------------------------------------------\n");
    // printf("\n");
}

void imprimir_comandos(hilos_args *args)
{
    log_generic(args, LOG_LEVEL_INFO, "└─ EJECUTAR_SCRIPT <path>");
    log_generic(args, LOG_LEVEL_INFO, "└─ INICIAR_PROCESO <path>");
    log_generic(args, LOG_LEVEL_INFO, "└─ FINALIZAR_PROCESO <PID>");
    log_generic(args, LOG_LEVEL_INFO, "└─ DETENER_PLANIFICACION");
    log_generic(args, LOG_LEVEL_INFO, "└─ INICIAR_PLANIFICACION");
    log_generic(args, LOG_LEVEL_INFO, "└─ MULTIPROGRAMACION <grado>");
    log_generic(args, LOG_LEVEL_INFO, "└─ PROCESO_ESTADO <PID>");
    log_generic(args, LOG_LEVEL_INFO, "└─ FINALIZAR");

    // printf("└─ EJECUTAR_SCRIPT <path>\n");
    // printf("└─ INICIAR_PROCESO <path>\n");
    // printf("└─ FINALIZAR_PROCESO <PID>\n");
    // printf("└─ DETENER_PLANIFICACION\n");
    // printf("└─ INICIAR_PLANIFICACION\n");
    // printf("└─ MULTIPROGRAMACION <grado>\n");
    // printf("└─ PROCESO_ESTADO <PID>\n");
    // printf("└─ FINALIZAR\n");
}

void imprimir_header(hilos_args *args)
{
    imprimir_logo(args);
    log_generic(args, LOG_LEVEL_INFO, "Comandos disponibles:");
    imprimir_comandos(args);
}

void log_generic_rl(hilos_args *args, t_log_level nivel, const char *mensaje)
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
        log_error(args->logger, "Nivel de log inválido en log_generic_rl");
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

void log_generic(hilos_args *args, t_log_level nivel, const char *mensaje, ...)
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
        log_generic_rl(args, nivel, mensaje_formateado);

        // Liberar la memoria del mensaje formateado
        free(mensaje_formateado);
    }
    else
    {
        log_error(args->logger, "Error al reservar memoria para imprimir log");
    }

    va_end(args_list);
}

void hilo_ejecutar_kernel(int socket, hilos_args *args, char *modulo, t_funcion_kernel_ptr switch_case_atencion)
{
    while (1)
    {
        log_generic(args, LOG_LEVEL_DEBUG, "Esperando paquete de %s en socket %d", modulo, socket);

        t_paquete *paquete = recibir_paquete(args->logger, &socket);

        if (paquete == NULL)
        {
            log_generic(args, LOG_LEVEL_WARNING, "%s se desconecto.", modulo);
            break;
        }

        revisar_paquete_kernel(args, paquete, modulo);

        switch_case_atencion(args->logger, paquete->codigo_operacion, args, paquete->buffer);

        eliminar_paquete(paquete);
    }
    sem_post(&args->kernel->sistema_finalizar);

    log_generic(args, LOG_LEVEL_DEBUG, "Finalizando hilo de atencion a %s", modulo);
}

void revisar_paquete_kernel(hilos_args *args, t_paquete *paquete, char *modulo)
{
    if (paquete->codigo_operacion != FINALIZAR_SISTEMA)
    {
        log_generic(args, LOG_LEVEL_DEBUG, "Paquete recibido de modulo %s", modulo);
        log_generic(args, LOG_LEVEL_DEBUG, "Deserializado del paquete:");
        log_generic(args, LOG_LEVEL_DEBUG, "Codigo de operacion: %d", paquete->codigo_operacion);
        log_generic(args, LOG_LEVEL_DEBUG, "Size del buffer en paquete: %d", paquete->size_buffer);
        log_generic(args, LOG_LEVEL_DEBUG, "Deserializado del buffer:");
        log_generic(args, LOG_LEVEL_DEBUG, "Size del stream: %d", paquete->buffer->size);
        log_generic(args, LOG_LEVEL_DEBUG, "Offset del stream: %d", paquete->buffer->offset);

        if (paquete->size_buffer != paquete->buffer->size + (2 * sizeof(uint32_t)))
        {
            log_generic(args, LOG_LEVEL_WARNING, "Error en el tamaño del buffer. Se esperaba %d y se recibio %ld", paquete->size_buffer, paquete->buffer->size + (2 * sizeof(uint32_t)));
        }
    }
    else
    {
        log_generic(args, LOG_LEVEL_DEBUG, "Paquete de finalizacion recibido de modulo %s", modulo);
    }
}

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

void kernel_log(hilos_args *args)
{
    // printf("Configuraciones de Kernel\n");
    // printf("PUERTO_ESCUCHA: %d\n", args->kernel->puertoEscucha);
    // printf("IP_MEMORIA: %s\n", args->kernel->ipMemoria);
    // printf("PUERTO_MEMORIA: %d\n", args->kernel->puertoMemoria);
    // printf("IP_CPU: %s\n", args->kernel->ipCpu);
    // printf("PUERTO_CPU_DISPATCH: %d\n", args->kernel->puertoCpuDispatch);
    // printf("PUERTO_CPU_INTERRUPT: %d\n", args->kernel->puertoCpuInterrupt);
    // printf("ALGORITMO_PLANIFICACION: %s\n", args->kernel->algoritmoPlanificador);
    // printf("QUANTUM: %d\n", args->kernel->quantum);
    // printf("RECURSOS: %s\n", args->kernel->recursos);
    // printf("INSTANCIAS_RECURSOS: %s\n", args->kernel->instanciasRecursos);
    // printf("GRADO_MULTIPROGRAMACION: %d\n", args->kernel->gradoMultiprogramacion);

    log_generic(args, LOG_LEVEL_INFO, "Configuraciones de Kernel");
    log_generic(args, LOG_LEVEL_INFO, "PUERTO_ESCUCHA: %d", args->kernel->puertoEscucha);
    log_generic(args, LOG_LEVEL_INFO, "IP_MEMORIA: %s", args->kernel->ipMemoria);
    log_generic(args, LOG_LEVEL_INFO, "PUERTO_MEMORIA: %d", args->kernel->puertoMemoria);
    log_generic(args, LOG_LEVEL_INFO, "IP_CPU: %s", args->kernel->ipCpu);
    log_generic(args, LOG_LEVEL_INFO, "PUERTO_CPU_DISPATCH: %d", args->kernel->puertoCpuDispatch);
    log_generic(args, LOG_LEVEL_INFO, "PUERTO_CPU_INTERRUPT: %d", args->kernel->puertoCpuInterrupt);
    log_generic(args, LOG_LEVEL_INFO, "ALGORITMO_PLANIFICACION: %s", args->kernel->algoritmoPlanificador);
    log_generic(args, LOG_LEVEL_INFO, "QUANTUM: %d", args->kernel->quantum);
    log_generic(args, LOG_LEVEL_INFO, "RECURSOS: %s", args->kernel->recursos);
    log_generic(args, LOG_LEVEL_INFO, "INSTANCIAS_RECURSOS: %s", args->kernel->instanciasRecursos);
    log_generic(args, LOG_LEVEL_INFO, "GRADO_MULTIPROGRAMACION: %d", args->kernel->gradoMultiprogramacion);
}

t_kernel_sockets kernel_sockets_agregar(hilos_args *args, KERNEL_SOCKETS type, int socket)
{
    switch (type)
    {
    case MEMORIA:
        args->kernel->sockets.memoria = socket;
        break;
    case CPU_DISPATCH:
        args->kernel->sockets.cpu_dispatch = socket;
        break;
    case CPU_INTERRUPT:
        args->kernel->sockets.cpu_interrupt = socket;
        break;
    case SERVER:
        args->kernel->sockets.server = socket;
        break;
    default:
        break;
    }
    return args->kernel->sockets;
};

char *kernel_sockets_agregar_entrada_salida(hilos_args *args, KERNEL_SOCKETS type, int socket)
{
    t_kernel_entrada_salida *entrada_salida = NULL;
    switch (type)
    {
    case ENTRADA_SALIDA_GENERIC:
        entrada_salida = entrada_salida_agregar_interfaz(args, ENTRADA_SALIDA_GENERIC, socket);
        break;
    case ENTRADA_SALIDA_STDIN:
        entrada_salida = entrada_salida_agregar_interfaz(args, ENTRADA_SALIDA_STDIN, socket);
        break;
    case ENTRADA_SALIDA_STDOUT:
        entrada_salida = entrada_salida_agregar_interfaz(args, ENTRADA_SALIDA_STDOUT, socket);
        break;
    case ENTRADA_SALIDA_DIALFS:
        entrada_salida = entrada_salida_agregar_interfaz(args, ENTRADA_SALIDA_DIALFS, socket);
        break;
    default:
        log_generic(args, LOG_LEVEL_ERROR, "Tipo de socket de entrada/salida no reconocido");
        break;
    }

    if (entrada_salida == NULL)
    {
        log_generic(args, LOG_LEVEL_ERROR, "Error al agregar el socket de entrada/salida");
        return NULL;
    }

    return entrada_salida->interfaz;
};

t_kernel_entrada_salida *entrada_salida_agregar_interfaz(hilos_args *args, KERNEL_SOCKETS tipo, int socket)
{
    // Asigno memoria para el socket de entrada/salida (no debo liberarla porque se guarda dentro de la lista la referencia)
    t_kernel_entrada_salida *entrada_salida = malloc(sizeof(t_kernel_entrada_salida));

    // Guardo el socket en el cual se conecto el modulo de entrada/salida
    entrada_salida->socket = socket;
    entrada_salida->tipo = tipo;
    entrada_salida->orden = args->kernel->sockets.id_entrada_salida;

    // Calculo el tamaño que necesito para almacenar el identificador de la interfaz
    int size_necesario = snprintf(NULL, 0, "Int%d", args->kernel->sockets.id_entrada_salida) + 1;

    // Reservo memoria para la interfaz (no debo liberarla porque se guarda dentro del diccionario)
    char *interfaz = malloc(size_necesario);

    // Imprimo sobre la variable interfaz el identificador de la interfaz
    sprintf(interfaz, "Int%d", args->kernel->sockets.id_entrada_salida);

    // Duplico la cadena para guardarla en el TAD y poder identificar la IO (esto pide malloc y hay que liberarlo cuando se desconecta la IO)
    entrada_salida->interfaz = strdup(interfaz);

    int *index = malloc(sizeof(int));

    // Agrego el TAD a la lista de entrada/salida y guardo el indice en el que se encuentra
    *index = list_add(args->kernel->sockets.list_entrada_salida, entrada_salida);

    // Guardo en el diccionario la key interfaz y el value indice para ubicarlo en la lista luego
    dictionary_put(args->kernel->sockets.dictionary_entrada_salida, interfaz, index);

    log_generic(args, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida en el socket %d con la interfaz %s", socket, interfaz);

    args->kernel->sockets.id_entrada_salida++;

    return entrada_salida;
}

void entrada_salida_remover_interfaz(hilos_args *args, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(args->kernel->sockets.dictionary_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede desconectar
    if (indice == NULL)
    {
        log_generic(args, LOG_LEVEL_ERROR, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_kernel_entrada_salida *entrada_salida = list_get(args->kernel->sockets.list_entrada_salida, *indice);

    log_generic(args, LOG_LEVEL_DEBUG, "Se remueve interfaz de entrada/salida %s", interfaz);

    // Cierro el socket de la entrada/salida del lado de Kernel
    liberar_conexion(&entrada_salida->socket);

    // Elimino la interfaz del diccionario
    dictionary_remove(args->kernel->sockets.dictionary_entrada_salida, interfaz);

    // Libero la memoria de la interfaz
    free(entrada_salida->interfaz);

    // Libero la memoria del TAD
    free(entrada_salida);

    // Libero el indice guardado en el diccionario
    free(indice);

    // No lo elimino de la lista porque no se puede hacer un list_remove sin reorganizar los indices. Lo dejo en la lista pero no se puede acceder a el porque está vacio. Al finalizar el programa, destruyo la estructura de la lista entera.
}

t_kernel_entrada_salida *entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(args->kernel->sockets.dictionary_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede buscar
    if (indice == NULL)
    {
        log_generic(args, LOG_LEVEL_ERROR, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return NULL;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_kernel_entrada_salida *entrada_salida = list_get(args->kernel->sockets.list_entrada_salida, *indice);

    if (entrada_salida == NULL)
    {
        log_generic(args, LOG_LEVEL_ERROR, "No se encontro la interfaz %s en la lista de entrada/salida", interfaz);
    }

    log_generic(args, LOG_LEVEL_DEBUG, "Se encontro el modulo de entrada/salida en el socket %d asociado a la interfaz %s", entrada_salida->socket, interfaz);

    return entrada_salida;
}

t_pcb *kernel_nuevo_proceso(hilos_args *args, t_diagrama_estados *estados, t_log *logger, char *instrucciones)
{
    t_pcb *nuevaPcb = pcb_crear(logger, args->kernel->quantum);
    // TODO: esto hay que pasarlo a imprimimr_log para que no rompa la consola
    log_generic(args, LOG_LEVEL_DEBUG, "[PCB] Program Counter: %d", nuevaPcb->registros_cpu->pc);
    log_generic(args, LOG_LEVEL_DEBUG, "[PCB] Quantum: %d", nuevaPcb->quantum);
    log_generic(args, LOG_LEVEL_DEBUG, "[PCB] PID: %d", nuevaPcb->pid);
    log_generic(args, LOG_LEVEL_DEBUG, "[PROCESO] Instrucciones: %s", instrucciones);

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
    proceso_push_new(estados, nuevaPcb);

    return nuevaPcb;
}

/** FUNCIONES DE CONSOLA**/

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
        log_generic(args, LOG_LEVEL_DEBUG, "El hilo planificador se encontraba bloqueado. Lo desbloqueo para poder finalizarlo.");
        sem_post(&args->kernel->planificador_iniciar);
    }

    // Espero a que termine el hilo planificador
    sem_wait(&args->kernel->sistema_finalizar);
    log_generic(args, LOG_LEVEL_DEBUG, "Hilo planificador finalizado");

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

            log_generic(args, LOG_LEVEL_DEBUG, "Se desconectó la interfaz de entrada salida %s", interfaz_copia);
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

    log_generic(args, LOG_LEVEL_DEBUG, "Esperando a que se cierren los modulos: Memoria, CPU Dispatch, CPU Interrupt y Atencion de Entrada/Salida.");

    for (int i = 0; i < cantidad_modulos; i++)
    {
        log_generic(args, LOG_LEVEL_DEBUG, "Esperando a que se cierre un modulo del sistema.");
        sem_wait(&args->kernel->sistema_finalizar);
        log_generic(args, LOG_LEVEL_DEBUG, "Un modulo del sistema termino de cerrar.");
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

t_pcb *kernel_transicion_exec_exit(t_diagrama_estados *estados)
{
    pthread_mutex_lock(&estados->mutex_exec_exit);

    t_pcb *proceso = proceso_pop_exec(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_exit(estados, proceso);

    pthread_mutex_unlock(&estados->mutex_exec_exit);
    return proceso;
};

t_pcb *
kernel_transicion_block_ready(t_diagrama_estados *estados, t_log *logger)
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
        // log_generic(logger, LOG_LEVEL_INFO, "PID: <%d> - Desalojado por fin de Quantum", proceso->pid);
        //  Enviar señal de fin de quantum
        //  TODO: Falta agregar el motivo del interrupt en el paquete
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