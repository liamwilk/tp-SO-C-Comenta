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
    // Verificamos que un proceso en block no tenga a esta interfaz asignada
    if (io_args->entrada_salida->pid != 0)
    {
        // Lo eliminamos

        proceso_matar(io_args->args->estados, string_itoa(io_args->entrada_salida->pid));

        // Log oficial

        kernel_log_generic(io_args->args, LOG_LEVEL_INFO, "Finaliza el proceso <%d> -  Motivo: <INVALID_INTERFACE>", pid);

        //  Avisamos a memoria
        t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
        t_kernel_memoria_finalizar_proceso *proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
        proceso->pid = pid;
        serializar_t_kernel_memoria_finalizar_proceso(&paquete, proceso);
        enviar_paquete(paquete, io_args->args->kernel->sockets.memoria);
        eliminar_paquete(paquete);
        free(proceso);
    }

    entrada_salida_remover_interfaz(io_args->args, io_args->entrada_salida->interfaz);

    free(io_args);
}

void kernel_cpu_entradasalida_no_conectada(hilos_args *args, t_kernel_cpu_entradasalida_error TIPO, char *interfaz, uint32_t pid)
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
    if (TIPO == CPU_IO_STDIN_READ)
    {
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDIN_READ);
        t_kernel_cpu_io_stdin_read *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdin_read));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no se encuentra conectada a Kernel");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_stdin_read(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
    if (TIPO == CPU_IO_FS_CREATE)
    {
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_FS_CREATE);
        t_kernel_cpu_io_fs_create *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_fs_create));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no se encuentra conectada a Kernel");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_fs_create(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
    if (TIPO == CPU_IO_FS_TRUNCATE)
    {
        // TODO: Implementar
    }
};

void kernel_cpu_entradasalida_distinto_tipo(hilos_args *args, t_kernel_cpu_entradasalida_error TIPO, char *interfaz, uint32_t pid)
{
    if (TIPO == CPU_IO_STDOUT_WRITE)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque no es del tipo IO_STDOUT.", interfaz);
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDOUT_WRITE);
        t_kernel_cpu_io_stdout_write *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdout_write));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDOUT_WRITE porque no es del tipo IO_STDOUT");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_stdout_write(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
    if (TIPO == CPU_IO_STDIN_READ)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque no es del tipo IO_STDIN.", interfaz);
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDIN_READ);
        t_kernel_cpu_io_stdin_read *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdin_read));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDIN_READ porque no es del tipo IO_STDIN");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_stdin_read(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
    if (TIPO == CPU_IO_FS_CREATE)
    {
        kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque no es del tipo DIALFS.", interfaz);
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_FS_CREATE);
        t_kernel_cpu_io_fs_create *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_fs_create));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_FS_CREATE porque no es del tipo DIALFS");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_fs_create(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
};

void kernel_cpu_entradasalida_ocupada(hilos_args *args, t_kernel_cpu_entradasalida_error TIPO, char *interfaz, uint32_t pid)
{
    kernel_log_generic(args, LOG_LEVEL_ERROR, "No se pudo enviar el paquete a la interfaz %s porque esta ocupada con otro proceso", interfaz);

    if (TIPO == CPU_IO_STDOUT_WRITE)
    {
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDOUT_WRITE);
        t_kernel_cpu_io_stdout_write *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdout_write));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDOUT_WRITE porque se encuentra ocupada con otro proceso");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_stdout_write(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
    if (TIPO == CPU_IO_STDIN_READ)
    {
        t_paquete *paquete = crear_paquete(KERNEL_CPU_IO_STDIN_READ);
        t_kernel_cpu_io_stdin_read *proceso_enviar = malloc(sizeof(t_kernel_cpu_io_stdin_read));

        proceso_enviar->pid = pid;
        proceso_enviar->resultado = 0;
        proceso_enviar->motivo = strdup("La interfaz solicitada no puede ejecutar la instruccion IO_STDIN_READ porque se encuentra ocupada con otro proceso");
        proceso_enviar->size_motivo = strlen(proceso_enviar->motivo) + 1;

        serializar_t_kernel_cpu_io_stdin_read(&paquete, proceso_enviar);
        enviar_paquete(paquete, args->kernel->sockets.cpu_dispatch);
        eliminar_paquete(paquete);
        free(proceso_enviar->motivo);
        free(proceso_enviar);
    }
};

void kernel_proximo_io_generic(hilos_args *args, t_kernel_entrada_salida *io)
{
    int proceso_en_block = list_size(args->estados->block);
    if (proceso_en_block > 0)
    {
        for (int i = 0; i < proceso_en_block; i++)
        {
            t_pcb *pcb = list_get(args->estados->block, i);

            if (pcb == NULL)
            {
                continue;
            }
            // Verificamos que este bloqueado por I/O y no por RECURSO
            char *recurso_bloqueado = recurso_buscar_pid(args->recursos, pcb->pid);
            if (recurso_bloqueado != NULL)
            {
                continue;
            }
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                continue;
            }
            if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_GENERIC)
            {
                io->ocupado = 1;
                io->pid = pcb->pid;
                t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);
                t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
                unidad->pid = pcb->pid;
                char *unidad_trabajo = list_get(pcb->proxima_io->args, 0);
                unidad->unidad_de_trabajo = atoi(unidad_trabajo);
                serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);
                enviar_paquete(paquete, io->socket);

                eliminar_paquete(paquete);
                free(unidad);
                free(pcb->proxima_io->args);
                pcb->proxima_io->tiene_proxima_io = false;
            }
        }
    }
}

void kernel_proximo_io_stdout(hilos_args *args, t_kernel_entrada_salida *io)
{
    int proceso_en_block = list_size(args->estados->block);
    if (proceso_en_block > 0)
    {
        for (int i = 0; i < proceso_en_block; i++)
        {
            t_pcb *pcb = list_get(args->estados->block, i);

            if (pcb == NULL)
            {
                continue;
            }
            // Verificamos que este bloqueado por I/O y no por RECURSO
            char *recurso_bloqueado = recurso_buscar_pid(args->recursos, pcb->pid);
            if (recurso_bloqueado != NULL)
            {
                continue;
            }
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                continue;
            }

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Proceso <%d> bloqueado por I/O en interfaz <%s>", pcb->pid, pcb->proxima_io->identificador);
            if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_STDOUT)
            {
                io->ocupado = 1;
                io->pid = pcb->pid;

                t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_STDOUT_WRITE);
                t_io_stdout_write *proceso = malloc(sizeof(t_io_stdout_write));

                char *pid = list_get(pcb->proxima_io->args, 0);
                proceso->pid = atoi(pid);
                char *resultado = list_get(pcb->proxima_io->args, 1);
                proceso->resultado = atoi(resultado);
                char *registro_direccion = list_get(pcb->proxima_io->args, 2);
                proceso->registro_direccion = atoi(registro_direccion);
                char *registro_tamanio = list_get(pcb->proxima_io->args, 3);
                proceso->registro_tamanio = atoi(registro_tamanio);
                char *marco = list_get(pcb->proxima_io->args, 4);
                proceso->marco = atoi(marco);
                char *numero_pagina = list_get(pcb->proxima_io->args, 5);
                proceso->numero_pagina = atoi(numero_pagina);
                char *direccion_fisica = list_get(pcb->proxima_io->args, 6);
                proceso->direccion_fisica = atoi(direccion_fisica);
                char *desplazamiento = list_get(pcb->proxima_io->args, 7);
                proceso->desplazamiento = atoi(desplazamiento);
                char *size_interfaz = list_get(pcb->proxima_io->args, 8);
                proceso->size_interfaz = atoi(size_interfaz);
                char *interfaz = list_get(pcb->proxima_io->args, 9);
                proceso->interfaz = strdup(interfaz);
                char *ax = list_get(pcb->proxima_io->args, 10);
                proceso->registros.ax = atoi(ax);
                char *bx = list_get(pcb->proxima_io->args, 11);
                proceso->registros.bx = atoi(bx);
                char *cx = list_get(pcb->proxima_io->args, 12);
                proceso->registros.cx = atoi(cx);
                char *dx = list_get(pcb->proxima_io->args, 13);
                proceso->registros.dx = atoi(dx);
                char *eax = list_get(pcb->proxima_io->args, 14);
                proceso->registros.eax = atoi(eax);
                char *ebx = list_get(pcb->proxima_io->args, 15);
                proceso->registros.ebx = atoi(ebx);
                char *ecx = list_get(pcb->proxima_io->args, 16);
                proceso->registros.ecx = atoi(ecx);
                char *edx = list_get(pcb->proxima_io->args, 17);
                proceso->registros.edx = atoi(edx);
                char *si = list_get(pcb->proxima_io->args, 18);
                proceso->registros.si = atoi(si);
                char *di = list_get(pcb->proxima_io->args, 19);
                proceso->registros.di = atoi(di);
                char *pc = list_get(pcb->proxima_io->args, 20);
                proceso->registros.pc = atoi(pc);

                serializar_t_io_stdout_write(&paquete, proceso);
                enviar_paquete(paquete, io->socket);

                free(pid);
                free(resultado);
                free(registro_direccion);
                free(registro_tamanio);
                free(marco);
                free(numero_pagina);
                free(direccion_fisica);
                free(desplazamiento);
                free(interfaz);
                free(size_interfaz);
                free(ax);
                free(bx);
                free(cx);
                free(dx);
                free(eax);
                free(ebx);
                free(ecx);
                free(edx);
                free(si);
                free(di);
                free(pc);
                eliminar_paquete(paquete);
                free(proceso);
                pcb->proxima_io->tiene_proxima_io = false;
            }
        }
    }
}

void kernel_proximo_io_stdin(hilos_args *args, t_kernel_entrada_salida *io)
{
    int proceso_en_block = list_size(args->estados->block);
    if (proceso_en_block > 0)
    {
        for (int i = 0; i < proceso_en_block; i++)
        {
            t_pcb *pcb = list_get(args->estados->block, i);

            if (pcb == NULL)
            {
                continue;
            }
            // Verificamos que este bloqueado por I/O y no por RECURSO
            char *recurso_bloqueado = recurso_buscar_pid(args->recursos, pcb->pid);
            if (recurso_bloqueado != NULL)
            {
                continue;
            }
            if (pcb->proxima_io->tiene_proxima_io == false)
            {
                continue;
            }

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Proceso <%d> bloqueado por I/O en interfaz <%s>", pcb->pid, pcb->proxima_io->identificador);
            if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_STDIN)
            {
                io->ocupado = 1;
                io->pid = pcb->pid;

                t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_STDIN_READ);
                t_io_stdin_read *proceso = malloc(sizeof(t_io_stdin_read));

                char *pid = list_get(pcb->proxima_io->args, 0);
                proceso->pid = atoi(pid);
                char *resultado = list_get(pcb->proxima_io->args, 1);
                proceso->resultado = atoi(resultado);
                char *registro_direccion = list_get(pcb->proxima_io->args, 2);
                proceso->registro_direccion = atoi(registro_direccion);
                char *registro_tamanio = list_get(pcb->proxima_io->args, 3);
                proceso->registro_tamanio = atoi(registro_tamanio);
                char *marco_inicial = list_get(pcb->proxima_io->args, 4);
                proceso->marco_inicial = atoi(marco_inicial);
                char *marco_final = list_get(pcb->proxima_io->args, 5);
                proceso->marco_final = atoi(marco_final);
                char *numero_pagina = list_get(pcb->proxima_io->args, 6);
                proceso->numero_pagina = atoi(numero_pagina);
                char *direccion_fisica = list_get(pcb->proxima_io->args, 7);
                proceso->direccion_fisica = atoi(direccion_fisica);
                char *desplazamiento = list_get(pcb->proxima_io->args, 8);

                char *cantidad_marcos = list_get(pcb->proxima_io->args, 9);
                proceso->cantidad_marcos = atoi(cantidad_marcos);

                char *marcos = list_get(pcb->proxima_io->args, 10);
                proceso->marcos = strdup(marcos);

                char *size_marcos = list_get(pcb->proxima_io->args, 11);
                proceso->size_marcos = atoi(size_marcos);

                proceso->desplazamiento = atoi(desplazamiento);
                char *interfaz = list_get(pcb->proxima_io->args, 12);
                proceso->interfaz = strdup(interfaz);
                char *size_interfaz = list_get(pcb->proxima_io->args, 13);
                proceso->size_interfaz = atoi(size_interfaz);
                char *ax = list_get(pcb->proxima_io->args, 14);
                proceso->registros.ax = atoi(ax);
                char *bx = list_get(pcb->proxima_io->args, 15);
                proceso->registros.bx = atoi(bx);
                char *cx = list_get(pcb->proxima_io->args, 16);
                proceso->registros.cx = atoi(cx);
                char *dx = list_get(pcb->proxima_io->args, 17);
                proceso->registros.dx = atoi(dx);
                char *eax = list_get(pcb->proxima_io->args, 18);
                proceso->registros.eax = atoi(eax);
                char *ebx = list_get(pcb->proxima_io->args, 19);
                proceso->registros.ebx = atoi(ebx);
                char *ecx = list_get(pcb->proxima_io->args, 20);
                proceso->registros.ecx = atoi(ecx);
                char *edx = list_get(pcb->proxima_io->args, 21);
                proceso->registros.edx = atoi(edx);
                char *si = list_get(pcb->proxima_io->args, 22);
                proceso->registros.si = atoi(si);
                char *di = list_get(pcb->proxima_io->args, 23);
                proceso->registros.di = atoi(di);
                char *pc = list_get(pcb->proxima_io->args, 24);
                proceso->registros.pc = atoi(pc);

                serializar_t_io_stdin_read(&paquete, proceso);
                enviar_paquete(paquete, io->socket);

                kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio el paquete a la interfaz <%s> para el proceso <%d>", io->interfaz, pcb->pid);

                eliminar_paquete(paquete);

                free(pid);
                free(resultado);
                free(registro_direccion);
                free(registro_tamanio);
                free(marco_inicial);
                free(marco_final);
                free(numero_pagina);
                free(direccion_fisica);
                free(desplazamiento);
                free(interfaz);
                free(size_interfaz);
                free(ax);
                free(bx);
                free(cx);
                free(dx);
                free(eax);
                free(ebx);
                free(ecx);
                free(edx);
                free(si);
                free(di);
                free(pc);
                free(proceso);
                free(pcb->proxima_io->args);
                pcb->proxima_io->tiene_proxima_io = false;
            }
        }
    }
}

// TODO: actualmente de FS sólo está para el create, habría que hacer de los demás también

void kernel_proximo_io_fs(hilos_args *args, t_kernel_entrada_salida *io)
{
    int proceso_en_block = list_size(args->estados->block);
    if (proceso_en_block <= 0)
    {
        return;
    }
    for (int i = 0; i < proceso_en_block; i++)
    {
        t_pcb *pcb = list_get(args->estados->block, i);
        if (pcb == NULL)
        {
            continue;
        }
        // Verificamos que este bloqueado por I/O y no por RECURSO
        char *recurso_bloqueado = recurso_buscar_pid(args->recursos, pcb->pid);
        if (recurso_bloqueado != NULL)
        {
            continue;
        }
        if (pcb->proxima_io->tiene_proxima_io == false)
        {
            continue;
        }

        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Proceso <%d> bloqueado por I/O en interfaz <%s>", pcb->pid, pcb->proxima_io->identificador);
        if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_DIALFS_CREATE)
        {
            io->ocupado = 1;
            io->pid = pcb->pid;

            t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_CREATE);
            t_entrada_salida_fs_create *proceso = malloc(sizeof(t_entrada_salida_fs_create));

            char *pid = list_get(pcb->proxima_io->args, 0);
            proceso->pid = atoi(pid);
            char *resultado = list_get(pcb->proxima_io->args, 1);
            proceso->resultado = atoi(resultado);
            char *size_interfaz = list_get(pcb->proxima_io->args, 2);
            proceso->size_interfaz = atoi(size_interfaz);
            char *nombre_archivo = list_get(pcb->proxima_io->args, 3);
            proceso->nombre_archivo = strdup(nombre_archivo);
            char *size_nombre_archivo = list_get(pcb->proxima_io->args, 4);
            proceso->size_nombre_archivo = atoi(size_nombre_archivo);
            proceso->interfaz = strdup(io->interfaz);
            serializar_t_entrada_salida_fs_create(&paquete, proceso);
            proceso->registros = *pcb->registros_cpu;
            enviar_paquete(paquete, io->socket);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio el paquete a la interfaz <%s> para el proceso <%d>", io->interfaz, pcb->pid);

            eliminar_paquete(paquete);

            free(proceso->interfaz);
            free(proceso->nombre_archivo);
            list_destroy_and_destroy_elements(pcb->proxima_io->args, free);
            pcb->proxima_io->tiene_proxima_io = false;
        }
        if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_DIALFS_TRUNCATE)
        {
            io->ocupado = 1;
            io->pid = pcb->pid;

            t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_TRUNCATE);
            t_kernel_entrada_salida_fs_truncate *proceso = malloc(sizeof(t_kernel_entrada_salida_fs_truncate));

            char *pid = list_get(pcb->proxima_io->args, 0);
            proceso->pid = atoi(pid);
            char *resultado = list_get(pcb->proxima_io->args, 1);
            proceso->resultado = atoi(resultado);
            char *size_interfaz = list_get(pcb->proxima_io->args, 2);
            proceso->size_interfaz = atoi(size_interfaz);
            char *nombre_archivo = list_get(pcb->proxima_io->args, 3);
            proceso->nombre_archivo = strdup(nombre_archivo);
            char *size_nombre_archivo = list_get(pcb->proxima_io->args, 4);
            char *tamanio_a_truncar = list_get(pcb->proxima_io->args, 5);
            proceso->tamanio_a_truncar = atoi(tamanio_a_truncar);
            proceso->size_nombre_archivo = atoi(size_nombre_archivo);
            proceso->interfaz = strdup(io->interfaz);
            serializar_t_kernel_entrada_salida_fs_truncate(&paquete, proceso);
            enviar_paquete(paquete, io->socket);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio el paquete a la interfaz <%s> para el proceso <%d>", io->interfaz, pcb->pid);

            eliminar_paquete(paquete);

            free(proceso->interfaz);
            free(proceso->nombre_archivo);
            list_destroy_and_destroy_elements(pcb->proxima_io->args, free);
            pcb->proxima_io->tiene_proxima_io = false;
        }
        if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_DIALFS_WRITE)
        {
            io->ocupado = 1;
            io->pid = pcb->pid;

            t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_WRITE);
            t_kernel_entrada_salida_fs_write *proceso = malloc(sizeof(t_kernel_entrada_salida_fs_write));

            char *pid = list_get(pcb->proxima_io->args, 0);
            proceso->pid = atoi(pid);
            char *resultado = list_get(pcb->proxima_io->args, 1);
            proceso->resultado = atoi(resultado);
            char *size_interfaz = list_get(pcb->proxima_io->args, 2);
            proceso->size_interfaz = atoi(size_interfaz);
            char *nombre_archivo = list_get(pcb->proxima_io->args, 3);
            proceso->nombre_archivo = strdup(nombre_archivo);
            char *size_nombre_archivo = list_get(pcb->proxima_io->args, 4);
            proceso->size_nombre_archivo = atoi(size_nombre_archivo);
            char *puntero_archivo = list_get(pcb->proxima_io->args, 5);
            proceso->puntero_archivo = atoi(puntero_archivo);
            char *escribir = list_get(pcb->proxima_io->args, 6);
            proceso->escribir = strdup(escribir);
            char *size_escribir = list_get(pcb->proxima_io->args, 7);
            proceso->size_escribir = atoi(size_escribir);
            proceso->interfaz = strdup(io->interfaz);
            serializar_t_kernel_entrada_salida_fs_write(&paquete, proceso);
            enviar_paquete(paquete, io->socket);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio el paquete a la interfaz <%s> para el proceso <%d>", io->interfaz, pcb->pid);

            eliminar_paquete(paquete);

            free(proceso->interfaz);
            free(proceso->nombre_archivo);
            free(proceso->escribir);
            list_destroy_and_destroy_elements(pcb->proxima_io->args, free);
            pcb->proxima_io->tiene_proxima_io = false;
        }
        if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_DIALFS_DELETE)
        {
            // Para el delete utilizamos las mismas funciones de create ya que son los mismos parametros
            io->ocupado = 1;
            io->pid = pcb->pid;

            t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_DELETE);
            t_entrada_salida_fs_create *proceso = malloc(sizeof(t_entrada_salida_fs_create));

            char *pid = list_get(pcb->proxima_io->args, 0);
            proceso->pid = atoi(pid);
            char *resultado = list_get(pcb->proxima_io->args, 1);
            proceso->resultado = atoi(resultado);
            char *size_interfaz = list_get(pcb->proxima_io->args, 2);
            proceso->size_interfaz = atoi(size_interfaz);
            char *nombre_archivo = list_get(pcb->proxima_io->args, 3);
            proceso->nombre_archivo = strdup(nombre_archivo);
            char *size_nombre_archivo = list_get(pcb->proxima_io->args, 4);
            proceso->size_nombre_archivo = atoi(size_nombre_archivo);
            proceso->interfaz = strdup(io->interfaz);
            proceso->registros = *pcb->registros_cpu;
            serializar_t_entrada_salida_fs_create(&paquete, proceso);
            proceso->registros = *pcb->registros_cpu;
            enviar_paquete(paquete, io->socket);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio el paquete a la interfaz <%s> para el proceso <%d>", io->interfaz, pcb->pid);

            eliminar_paquete(paquete);

            free(proceso->interfaz);
            free(proceso->nombre_archivo);
            list_destroy_and_destroy_elements(pcb->proxima_io->args, free);
            pcb->proxima_io->tiene_proxima_io = false;
        }
        if (strcmp(pcb->proxima_io->identificador, io->interfaz) == 0 && pcb->proxima_io->tipo == ENTRADA_SALIDA_DIALFS_READ)
        {
            io->ocupado = 1;
            io->pid = pcb->pid;

            t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_FS_READ);
            t_kernel_entrada_salida_fs_read *proceso = malloc(sizeof(t_kernel_entrada_salida_fs_read));

            char *pid = list_get(pcb->proxima_io->args, 0);
            proceso->pid = atoi(pid);
            char *resultado = list_get(pcb->proxima_io->args, 1);
            proceso->resultado = atoi(resultado);
            char *size_interfaz = list_get(pcb->proxima_io->args, 2);
            proceso->size_interfaz = atoi(size_interfaz);
            char *nombre_archivo = list_get(pcb->proxima_io->args, 3);
            proceso->nombre_archivo = strdup(nombre_archivo);
            char *size_nombre_archivo = list_get(pcb->proxima_io->args, 4);
            proceso->size_nombre_archivo = atoi(size_nombre_archivo);
            char *puntero_archivo = list_get(pcb->proxima_io->args, 5);
            proceso->puntero_archivo = atoi(puntero_archivo);
            char *direccion_fisica = list_get(pcb->proxima_io->args, 6);
            proceso->direccion_fisica = atoi(direccion_fisica);
            char *registro_tamanio = list_get(pcb->proxima_io->args, 7);
            proceso->registro_tamanio = atoi(registro_tamanio);
            char *marco = list_get(pcb->proxima_io->args, 8);
            proceso->marco = atoi(marco);
            proceso->interfaz = strdup(io->interfaz);
            proceso->registros = *pcb->registros_cpu;
            serializar_t_kernel_entrada_salida_fs_read(&paquete, proceso);
            enviar_paquete(paquete, io->socket);

            kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se envio el paquete a la interfaz <%s> para el proceso <%d>", io->interfaz, pcb->pid);

            eliminar_paquete(paquete);

            free(proceso->interfaz);
            free(proceso->nombre_archivo);
            list_destroy_and_destroy_elements(pcb->proxima_io->args, free);
            pcb->proxima_io->tiene_proxima_io = false;
        }
    }
}
