#include "entradasalida.h"

void entrada_salida_procesar_rechazado(hilos_io_args *argumentos, char *identificador)
{
    // Duplico la cadena para guardarla en el TAD y poder identificar la IO (esto pide malloc y hay que liberarlo cuando se desconecta la IO)

    free(argumentos->entrada_salida->interfaz);
    argumentos->entrada_salida->interfaz = strdup(identificador);
    argumentos->entrada_salida->identificado = false;

    int *index = malloc(sizeof(int));

    // Agrego el TAD a la lista de entrada/salida y guardo el indice en el que se encuentra para despues poder sacarlo
    *index = list_add(argumentos->args->kernel->sockets.list_entrada_salida, argumentos->entrada_salida);

    // Guardo en el diccionario la key socket y el value indice para ubicarlo en la lista luego
    dictionary_put(argumentos->args->kernel->sockets.dictionary_entrada_salida, strdup(identificador), index);
}

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
    entrada_salida->interfaz = strdup("No identificado");
    entrada_salida->identificado = false;
    entrada_salida->valido = true;

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida en el socket %d", socket);

    args->kernel->sockets.id_entrada_salida++;
    return entrada_salida;
}

void entrada_salida_remover_interfaz(hilos_args *args, char *identificador)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(args->kernel->sockets.dictionary_entrada_salida, identificador);

    // Si no se encuentra la interfaz en el diccionario, no se puede desconectar
    if (indice == NULL)
    {
        return;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_kernel_entrada_salida *entrada_salida = list_get(args->kernel->sockets.list_entrada_salida, *indice);

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se remueve interfaz de entrada/salida %s", entrada_salida->interfaz);

    // Cierro el socket de la entrada/salida del lado de Kernel
    liberar_conexion(&entrada_salida->socket);

    // Elimino la interfaz del diccionario
    dictionary_remove(args->kernel->sockets.dictionary_entrada_salida, identificador);

    // Libero la memoria de la interfaz
    free(entrada_salida->interfaz);

    // Libero la memoria del TAD
    free(entrada_salida);

    // Libero el indice guardado en el diccionario
    free(indice);

    // No lo elimino de la lista porque no se puede hacer un list_remove sin reorganizar los indices. Lo dejo en la lista pero no se puede acceder a el porque está vacio. Al finalizar el programa, destruyo la estructura de la lista entera.
}

void entrada_salida_agregar_identificador(hilos_io_args *argumentos, char *identificador)
{
    // Duplico la cadena para guardarla en el TAD y poder identificar la IO (esto pide malloc y hay que liberarlo cuando se desconecta la IO)

    free(argumentos->entrada_salida->interfaz);
    argumentos->entrada_salida->interfaz = strdup(identificador);
    argumentos->entrada_salida->identificado = true;

    int *index = malloc(sizeof(int));

    // Agrego el TAD a la lista de entrada/salida y guardo el indice en el que se encuentra
    *index = list_add(argumentos->args->kernel->sockets.list_entrada_salida, argumentos->entrada_salida);

    // Guardo en el diccionario la key socket y el value indice para ubicarlo en la lista luego
    dictionary_put(argumentos->args->kernel->sockets.dictionary_entrada_salida, strdup(identificador), index);

    kernel_log_generic(argumentos->args, LOG_LEVEL_DEBUG, "Se conecto un modulo de entrada/salida en el socket %d", argumentos->entrada_salida->socket);
}

void kernel_interrumpir_io(hilos_args *args, uint32_t pid, char *motivo)
{
    t_paquete *paquete = crear_paquete(KERNEL_IO_INTERRUPCION);
    t_kernel_io_interrupcion interrupcion = {.pid = pid, .motivo = motivo, .len_motivo = strlen(motivo) + 1};
    serializar_t_kernel_io_interrupcion(&paquete, &interrupcion);

    t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(args, pid);

    io->ocupado = 0;
    io->pid = 0;

    enviar_paquete(paquete, io->socket);
    eliminar_paquete(paquete);
}

t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(args->kernel->sockets.dictionary_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede buscar
    if (indice == NULL)
    {
        return NULL;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_kernel_entrada_salida *entrada_salida = list_get(args->kernel->sockets.list_entrada_salida, *indice);

    if (entrada_salida == NULL)
    {
        return NULL;
    }

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se encontro el modulo de entrada/salida en el socket %d asociado a la interfaz %s", entrada_salida->socket, interfaz);

    return entrada_salida;
}
t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz_pid(hilos_args *args, uint32_t pid)
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

void kernel_entradasalida_rechazo(hilos_io_args *io_args, char *modulo, char *identificador)
{
    entrada_salida_procesar_rechazado(io_args, "no identificada");
    kernel_log_generic(io_args->args, LOG_LEVEL_WARNING, "[%s/%d] Se rechazo identificacion, identificador %s ocupado. Cierro hilo.", modulo, io_args->entrada_salida->orden, identificador);
    io_args->entrada_salida->valido = false;
    io_args->args->kernel->sockets.id_entrada_salida--;
    avisar_rechazo_identificador(io_args->entrada_salida->socket);
};

void avisar_rechazo_identificador(int socket)
{
    t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IDENTIFICACION_RECHAZO);
    t_entrada_salida_identificacion *identificacion = malloc(sizeof(t_entrada_salida_identificacion));
    identificacion->identificador = "Rechazo";
    identificacion->size_identificador = strlen(identificacion->identificador) + 1;
    serializar_t_entrada_salida_identificacion(&paquete, identificacion);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}

t_kernel_entrada_salida *kernel_sockets_agregar_entrada_salida(hilos_args *args, KERNEL_SOCKETS type, int socket)
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

    return entrada_salida;
};

void hilos_ejecutar_entrada_salida(hilos_io_args *io_args, char *modulo, t_funcion_kernel_io_prt switch_case_atencion)
{
    int orden = io_args->entrada_salida->orden;
    int *socket = &io_args->entrada_salida->socket;

    while (1)
    {
        if (io_args->entrada_salida->valido)
        {
            if (io_args->entrada_salida->identificado)
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/%s/Orden %d] Esperando paquete en socket %d", modulo, io_args->entrada_salida->interfaz, orden, *socket);
            }
            else
            {
                kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/%d] Esperando identificador en socket %d", modulo, orden, *socket);
            }
        }

        t_paquete *paquete = recibir_paquete(io_args->args->logger, socket);

        if (paquete == NULL)
        {

            if (io_args->entrada_salida->valido)
            {
                if (io_args->entrada_salida->identificado)
                {
                    kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%s/%d] Se desconecto.", modulo, io_args->entrada_salida->interfaz, orden);
                }
                else
                {
                    kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "[%s/%d] Se desconecto.", modulo, orden);
                }
            }
            break;
        }

        kernel_revisar_paquete(paquete, io_args->args, modulo);

        switch_case_atencion(io_args, modulo, paquete->codigo_operacion, paquete->buffer);

        eliminar_paquete(paquete);
    }

    kernel_log_generic(io_args->args, LOG_LEVEL_DEBUG, "[%s/%s/%d] Cerrando hilo", modulo, io_args->entrada_salida->interfaz, orden);

    entrada_salida_remover_interfaz(io_args->args, io_args->entrada_salida->interfaz);

    free(io_args);
}

void kernel_cpu_entradasalida_no_conectada(hilos_args *args, t_kernel_cpu_entradasalida_no_conectada TIPO, char *interfaz, uint32_t pid)
{
    kernel_log_generic(args, LOG_LEVEL_ERROR, "Aviso a CPU que no se pudo enviar el paquete a la interfaz <%s> porque no está conectada", interfaz);
    if (TIPO == CPU_IO_STDOUT_WRITE)
    {
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDOUT_WRITE);
        t_kernel_cpu_io_stdout_write *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdout_write));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no se encuentra conectada a Kernel");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_stdout_write(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
};