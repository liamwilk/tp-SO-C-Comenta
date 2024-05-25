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

    kernel_log_generic(args, LOG_LEVEL_INFO, "              _                 _ _ _   _____ _____");
    kernel_log_generic(args, LOG_LEVEL_INFO, "             | |               | (_) | |  _  /  ___|");
    kernel_log_generic(args, LOG_LEVEL_INFO, " _ __ ___  __| | ___  _ __   __| |_| |_| | | \\ `--. ");
    kernel_log_generic(args, LOG_LEVEL_INFO, "| '__/ _ \\/ _` |/ _ \\| '_ \\ / _` | | __| | | |`--. \\");
    kernel_log_generic(args, LOG_LEVEL_INFO, "| | |  __/ (_| | (_) | | | | (_| | | |_\\ \\_/ /\\__/ /");
    kernel_log_generic(args, LOG_LEVEL_INFO, "|_|  \\___|\\__,_|\\___/|_| |_|\\__,_|_|\\__|\\___/\\____/");
    kernel_log_generic(args, LOG_LEVEL_INFO, " ");
    kernel_log_generic(args, LOG_LEVEL_INFO, "---------------------------------------------------------------");
    kernel_log_generic(args, LOG_LEVEL_INFO, "Implementación de C-Comenta - www.faq.utnso.com.ar/tp-c-comenta");
    kernel_log_generic(args, LOG_LEVEL_INFO, "Sistemas Operativos - 1C 2024 - UTN FRBA");
    kernel_log_generic(args, LOG_LEVEL_INFO, "---------------------------------------------------------------");
    kernel_log_generic(args, LOG_LEVEL_INFO, " ");
}

void imprimir_comandos(hilos_args *args)
{
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ EJECUTAR_SCRIPT <path>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ INICIAR_PROCESO <path>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ FINALIZAR_PROCESO <PID>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ DETENER_PLANIFICACION");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ INICIAR_PLANIFICACION");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ MULTIPROGRAMACION <grado>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ PROCESO_ESTADO <PID>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ FINALIZAR");
}

void imprimir_header(hilos_args *args)
{
    imprimir_logo(args);
    kernel_log_generic(args, LOG_LEVEL_INFO, "Comandos disponibles:");
    imprimir_comandos(args);
}

void hilo_ejecutar_kernel(int socket, hilos_args *args, char *modulo, t_funcion_kernel_ptr switch_case_atencion)
{
    while (1)
    {
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Esperando paquete de %s en socket %d", modulo, socket);

        t_paquete *paquete = recibir_paquete(args->logger, &socket);

        if (paquete == NULL)
        {
            kernel_log_generic(args, LOG_LEVEL_WARNING, "%s se desconecto.", modulo);
            break;
        }

        revisar_paquete_kernel(args, paquete, modulo);

        switch_case_atencion(args->logger, paquete->codigo_operacion, args, paquete->buffer);

        eliminar_paquete(paquete);
    }
    sem_post(&args->kernel->sistema_finalizar);

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Finalizando hilo de atencion a %s", modulo);
}

void revisar_paquete_kernel(hilos_args *args, t_paquete *paquete, char *modulo)
{
    if (paquete->codigo_operacion != FINALIZAR_SISTEMA)
    {
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Paquete recibido de modulo %s", modulo);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Deserializado del paquete:");
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Codigo de operacion: %d", paquete->codigo_operacion);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size del buffer en paquete: %d", paquete->size_buffer);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Deserializado del buffer:");
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Size del stream: %d", paquete->buffer->size);
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Offset del stream: %d", paquete->buffer->offset);

        if (paquete->size_buffer != paquete->buffer->size + (2 * sizeof(uint32_t)))
        {
            kernel_log_generic(args, LOG_LEVEL_WARNING, "Error en el tamaño del buffer. Se esperaba %d y se recibio %ld", paquete->size_buffer, paquete->buffer->size + (2 * sizeof(uint32_t)));
        }
    }
    else
    {
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Paquete de finalizacion recibido de modulo %s", modulo);
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
    kernel_log_generic(args, LOG_LEVEL_INFO, "Configuraciones de Kernel");
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_ESCUCHA: %d", args->kernel->puertoEscucha);
    kernel_log_generic(args, LOG_LEVEL_INFO, "IP_MEMORIA: %s", args->kernel->ipMemoria);
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_MEMORIA: %d", args->kernel->puertoMemoria);
    kernel_log_generic(args, LOG_LEVEL_INFO, "IP_CPU: %s", args->kernel->ipCpu);
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_CPU_DISPATCH: %d", args->kernel->puertoCpuDispatch);
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_CPU_INTERRUPT: %d", args->kernel->puertoCpuInterrupt);
    kernel_log_generic(args, LOG_LEVEL_INFO, "ALGORITMO_PLANIFICACION: %s", args->kernel->algoritmoPlanificador);
    kernel_log_generic(args, LOG_LEVEL_INFO, "QUANTUM: %d", args->kernel->quantum);
    kernel_log_generic(args, LOG_LEVEL_INFO, "RECURSOS: %s", args->kernel->recursos);
    kernel_log_generic(args, LOG_LEVEL_INFO, "INSTANCIAS_RECURSOS: %s", args->kernel->instanciasRecursos);
    kernel_log_generic(args, LOG_LEVEL_INFO, "GRADO_MULTIPROGRAMACION: %d", args->kernel->gradoMultiprogramacion);
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
        kernel_log_generic(args, LOG_LEVEL_ERROR, "Tipo de socket de entrada/salida no reconocido");
        break;
    }

    if (entrada_salida == NULL)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "Error al agregar el socket de entrada/salida");
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
    entrada_salida->ocupado = 0;
    entrada_salida->pid = 0;

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

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida en el socket %d con la interfaz %s", socket, interfaz);

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
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_kernel_entrada_salida *entrada_salida = list_get(args->kernel->sockets.list_entrada_salida, *indice);

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se remueve interfaz de entrada/salida %s", interfaz);

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

t_pcb *kernel_nuevo_proceso(hilos_args *args, t_diagrama_estados *estados, t_log *logger, char *instrucciones)
{
    t_pcb *nuevaPcb = pcb_crear(logger, args->kernel->quantum);
    // TODO: esto hay que pasarlo a imprimimr_log para que no rompa la consola
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PCB] Program Counter: %d", nuevaPcb->registros_cpu->pc);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PCB] Quantum: %d", nuevaPcb->quantum);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PCB] PID: %d", nuevaPcb->pid);
    kernel_log_generic(args, LOG_LEVEL_DEBUG, "[PROCESO] Instrucciones: %s", instrucciones);

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
