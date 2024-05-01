#include "hilos.h"

/*----CONSOLA INTERACTIVA----*/

void *hilos_atender_consola(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    consola_iniciar(hiloArgs->logger, hiloArgs->kernel, hiloArgs->estados, hiloArgs->kernel_orden_apagado);
    pthread_exit(0);
}

void hilos_consola_inicializar(hilos_args *args, pthread_t thread_atender_consola)
{
    pthread_create(&thread_atender_consola, NULL, hilos_atender_consola, args);
    pthread_join(thread_atender_consola, NULL);
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

    kernel_sockets_agregar(hiloArgs->kernel, MEMORIA, socket);

    t_handshake resultado = crear_handshake(hiloArgs->logger, socket, MEMORIA_KERNEL, "Memoria");

    if (resultado != CORRECTO)
    {
        liberar_conexion(socket);
        pthread_exit(0);
    }

    log_debug(hiloArgs->logger, "Conectado a Memoria en socket %d", socket);
    pthread_exit(0);
}

void *hilos_atender_memoria(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    int socket = hiloArgs->kernel->sockets.memoria;
    while (*(hiloArgs->kernel_orden_apagado))
    {
        log_debug(hiloArgs->logger, "Esperando paquete de Memoria en socket %d", socket);
        t_paquete *paquete = recibir_paquete(hiloArgs->logger, socket);

        switch (paquete->codigo_operacion)
        {
        case MEMORIA_KERNEL_NUEVO_PROCESO:
        {
            revisar_paquete(paquete, hiloArgs->logger, *hiloArgs->kernel_orden_apagado, "Memoria");
            t_memoria_kernel_proceso *proceso = deserializar_t_memoria_kernel_proceso(paquete->buffer);

            log_debug(hiloArgs->logger, "Recibí la respuesta de Memoria acerca de la solicitud de nuevo proceso");
            log_debug(hiloArgs->logger, "PID: %d", proceso->pid);
            log_debug(hiloArgs->logger, "Cantidad de instrucciones: %d", proceso->cantidad_instruccions);
            log_debug(hiloArgs->logger, "Leido: %d", proceso->leido);
            if (proceso->leido)
            {
                // Enviar a cpu los registros
                t_paquete *paquete = crear_paquete(RECIBIR_REGISTROS_CPU);
                // Obtener registros desde la cola de estados
                t_pcb pcb = proceso_buscar_new(hiloArgs->estados->new, proceso->pid);
                serializar_t_registros_cpu(&paquete, pcb.registros_cpu);
                enviar_paquete(paquete, hiloArgs->kernel->sockets.cpu_dispatch);
            };
            free(proceso);
            break;
        }
        default:
            liberar_conexion(socket);
            pthread_exit(0);
            break;
        }

        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
    }

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

    kernel_sockets_agregar(hiloArgs->kernel, CPU_DISPATCH, socket);

    t_handshake resultado = crear_handshake(hiloArgs->logger, socket, CPU_DISPATCH_KERNEL, "CPU Dispatch");

    if (resultado != CORRECTO)
    {
        liberar_conexion(socket);
        pthread_exit(0);
    }

    log_debug(hiloArgs->logger, "Conectado a CPU por Dispatch en socket %d", socket);
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

    kernel_sockets_agregar(hiloArgs->kernel, CPU_INTERRUPT, socket);

    t_handshake resultado = crear_handshake(hiloArgs->logger, socket, CPU_INTERRUPT_KERNEL, "CPU Interrupt");

    if (resultado != CORRECTO)
    {
        liberar_conexion(socket);
        pthread_exit(0);
    }

    log_debug(hiloArgs->logger, "Conectado a CPU por Interrupt en socket %d", socket);
    pthread_exit(0);
};

void *hilos_atender_cpu_dispatch(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    int socket = hiloArgs->kernel->sockets.cpu_dispatch;
    while (*hiloArgs->kernel_orden_apagado)
    {
        log_debug(hiloArgs->logger, "Esperando paquete de CPU Dispatch en socket %d", socket);
        t_paquete *paquete = recibir_paquete(hiloArgs->logger, socket);

        switch (paquete->codigo_operacion)
        {
        case MENSAJE:
            revisar_paquete(paquete, hiloArgs->logger, *hiloArgs->kernel_orden_apagado, "Dispatch");
            // Placeholder
            break;
        default:
            liberar_conexion(socket);
            pthread_exit(0);
            break;
        }
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
    }

    pthread_exit(0);
};

void *hilos_atender_cpu_interrupt(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    int socket = hiloArgs->kernel->sockets.cpu_interrupt;
    while (*hiloArgs->kernel_orden_apagado)
    {
        log_debug(hiloArgs->logger, "Esperando paquete de CPU Interrupt en socket %d", socket);
        t_paquete *paquete = recibir_paquete(hiloArgs->logger, socket);

        switch (paquete->codigo_operacion)
        {
        case MENSAJE:
            revisar_paquete(paquete, hiloArgs->logger, *hiloArgs->kernel_orden_apagado, "Interrupt");
            // Placeholder
            break;
        default:
            liberar_conexion(socket);
            pthread_exit(0);
            break;
        }
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
    }

    pthread_exit(0);
};

/*----IO----*/

void hilos_io_inicializar(hilos_args *args, pthread_t thread_esperar_entrada_salida)
{
    pthread_create(&thread_esperar_entrada_salida, NULL, hilos_esperar_entradasalida, args);
    pthread_detach(thread_esperar_entrada_salida);
}

void *hilos_esperar_entradasalida(void *args)
{
    hilos_args *hiloArgs = (hilos_args *)args;
    while (*hiloArgs->kernel_orden_apagado)
    {
        int socket_cliente = esperar_conexion(hiloArgs->logger, hiloArgs->kernel->sockets.server);

        if (socket_cliente == -1)
        {
            break;
        }

        t_handshake resultado = esperar_handshake(hiloArgs->logger, socket_cliente, KERNEL_ENTRADA_SALIDA, "Entrada/Salida");

        if (resultado != CORRECTO)
        {
            liberar_conexion(socket_cliente);
            break;
        }

        pthread_t hilo;
        hilos_io_args *io_args = malloc(sizeof(hilos_io_args));
        io_args->args = hiloArgs;
        io_args->socket = socket_cliente;
        pthread_create(&hilo, NULL, hilos_atender_entradasalida, io_args);
        pthread_detach(hilo);
    }

    pthread_exit(0);
};

void *hilos_atender_entradasalida(void *args)
{

    hilos_io_args *io_args = (hilos_io_args *)args;
    log_debug(io_args->args->logger, "I/O conectado en socket %d", io_args->socket);
    do
    {
        log_debug(io_args->args->logger, "Esperando paquete de I/O en socket %d", io_args->socket);
        t_paquete *paquete = recibir_paquete(io_args->args->logger, io_args->socket);
        switch (paquete->codigo_operacion)
        {
        case MENSAJE:
            revisar_paquete(paquete, io_args->args->logger, *io_args->args->kernel_orden_apagado, "I/O");
            /*
            La logica
            */
            break;
        default:
            liberar_conexion(io_args->socket);
            pthread_exit(0);
            break;
        }

        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);

    } while (*io_args->args->kernel_orden_apagado);

    pthread_exit(0);
}
