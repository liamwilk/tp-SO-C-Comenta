#include <utils/memoria.h>

// Inicializa la tabla de páginas del proceso
void tabla_paginas_inicializar(t_args *args, t_proceso *proceso)
{
    pthread_mutex_init(&proceso->mutex_tabla_paginas, NULL);

    int paginas = args->memoria.tamMemoria / args->memoria.tamPagina;

    // Creo la tabla de paginas
    proceso->tabla_paginas = list_create();

    for (int i = 0; i < paginas; i++)
    {
        t_pagina *pagina = malloc(sizeof(t_pagina));

        if (pagina == NULL)
        {
            log_error(args->logger, "Error al asignar memoria para página %d del proceso %d", i, proceso->pid);
            tabla_paginas_liberar(args, proceso);
            return;
        }

        pagina->marco = 0;
        pagina->validez = 0;
        pagina->bytes = 0;
        pagina->offset = 0;

        list_add(proceso->tabla_paginas, pagina);
    }

    if (proceso->tabla_paginas == NULL)
    {
        log_error(args->logger, "Error al crear la tabla de páginas para el proceso %d", proceso->pid);
        return;
    }

    log_info(args->logger, "Creación tabla de páginas: PID: <%d> - Tamaño: <%d>", proceso->pid, list_size(proceso->tabla_paginas));
}

// Libera la tabla de páginas del proceso
void tabla_paginas_liberar(t_args *argumentos, t_proceso *proceso)
{
    for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);
        if (pagina != NULL)
        {
            argumentos->memoria.bitmap_array[pagina->marco] = 0;
        }
    }

    log_info(argumentos->logger, "Destrucción tabla de páginas: PID: <%d> - Tamaño: <%d>", proceso->pid, list_size(proceso->tabla_paginas));
    pthread_mutex_destroy(&proceso->mutex_tabla_paginas);
    list_destroy_and_destroy_elements(proceso->tabla_paginas, free);
}

// Asigna el marco a la página del proceso, y marca el marco como ocupado en el bitmap
t_pagina *tabla_paginas_asignar_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina, uint32_t frame)
{
    t_pagina *pagina = list_get(proceso->tabla_paginas, numero_pagina);

    if (pagina == NULL)
    {
        log_error(argumentos->logger, "Error al asignar memoria para página %d del proceso %d", list_size(proceso->tabla_paginas) + 1, proceso->pid);
        tabla_paginas_liberar(argumentos, proceso);
        return NULL;
    }

    if (frame == -1)
    {
        log_error(argumentos->logger, "No hay frames disponibles en espacio de usuario para asignar a la página <%d> del proceso <%d>", list_size(proceso->tabla_paginas) + 1, proceso->pid);
        return NULL;
    }

    pagina->marco = frame;
    pagina->validez = 1;
    pagina->bytes = 0;
    pagina->offset = 0;

    argumentos->memoria.bitmap_array[pagina->marco] = 1;

    log_info(argumentos->logger, "Asignación de tabla de páginas: PID: <%d> - Página: <%d> - Marco: <%d>", proceso->pid, list_size(proceso->tabla_paginas) - 1, pagina->marco);

    return pagina;
}

// Libera la página, y borra los datos del frame en la memoria
int tabla_paginas_liberar_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina)
{
    log_info(argumentos->logger, "Liberación de tabla de páginas: PID: <%d> - Página: <%d>", proceso->pid, numero_pagina);

    if (numero_pagina >= list_size(proceso->tabla_paginas))
    {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
        return -1;
    }

    t_pagina *pagina = list_get(proceso->tabla_paginas, numero_pagina);

    if (pagina == NULL)
    {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
        return -1;
    }

    int direccion_fisica = pagina->marco * argumentos->memoria.tamPagina;

    espacio_usuario_liberar_dato(argumentos, direccion_fisica, argumentos->memoria.tamPagina);

    pagina->marco = 0;
    pagina->validez = 0;
    pagina->bytes = 0;
    pagina->offset = 0;

    return 0;
}

// Retorna el marco de la página, si es que existe
int tabla_paginas_acceder_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina)
{
    t_pagina *pagina = list_get(proceso->tabla_paginas, numero_pagina);

    if (pagina == NULL)
    {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
        return -1;
    }

    if (pagina != NULL && pagina->validez == 1)
    {
        log_info(argumentos->logger, "Acceso a tabla de páginas: PID: <%d> - Página: <%d> - Marco: <%d>", proceso->pid, numero_pagina, pagina->marco);
        return pagina->marco;
    }
    else
    {
        log_error(argumentos->logger, "La página %d no es válida en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
    }
    return -1;
}

// Redimensiona la tabla de páginas del proceso, solamente marcando los frames ocupados o libres
int tabla_paginas_resize(t_args *args, t_proceso *proceso, uint32_t bytes_nuevos)
{
    pthread_mutex_lock(&proceso->mutex_tabla_paginas);

    int frames_nuevos = (bytes_nuevos + args->memoria.tamPagina - 1) / args->memoria.tamPagina;
    int bytes_actuales = tabla_paginas_bytes_ocupados(args, proceso);
    int frames_actuales = tabla_paginas_frames_ocupados(args, proceso);

    // Verifico si es necesario redimensionar la tabla de páginas
    if (bytes_actuales == bytes_nuevos)
    {
        log_info(args->logger, "No es necesario redimensionar la tabla de páginas del proceso <%d> porque ya tiene esa cantidad de bytes.", proceso->pid);
        pthread_mutex_unlock(&proceso->mutex_tabla_paginas);
        return 0;
    }

    if (frames_actuales > frames_nuevos) // Libero frames
    {
        for (int i = frames_actuales - 1; i >= frames_nuevos; i--)
        {
            t_pagina *pagina = list_get(proceso->tabla_paginas, i);
            if (pagina != NULL && pagina->validez == 1)
            {
                tabla_paginas_liberar_pagina(args, proceso, i);
            }
        }
    }
    else // Amplio frames
    {
        for (int i = frames_actuales; i < frames_nuevos; i++)
        {
            int proximo_frame = espacio_usuario_proximo_frame(args, args->memoria.tamPagina);

            if(args->memoria.bitmap_array[proximo_frame] == 1)
            {
                log_error(args->logger, "El frame %d ya esta ocupado", proximo_frame);
                pthread_mutex_unlock(&proceso->mutex_tabla_paginas);
                return -1;
            }

            tabla_paginas_asignar_pagina(args, proceso, i, proximo_frame);
        }
    }

    pthread_mutex_unlock(&proceso->mutex_tabla_paginas);

    if (tabla_paginas_bytes_ocupados(args, proceso) == args->memoria.tamMemoria)
    {
        log_warning(args->logger, "El proceso <%d> monopolizo la memoria.", proceso->pid);
    }
    return 1;
}

// Retorna la cantidad de bytes ocupados por el proceso
int tabla_paginas_bytes_ocupados(t_args *args, t_proceso *proceso)
{
    int bytes = 0;

    if (proceso->tabla_paginas != NULL)
    {
        for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
        {
            t_pagina *pagina = list_get(proceso->tabla_paginas, i);

            if (pagina != NULL && pagina->validez == 1)
            {
                bytes += args->memoria.tamPagina;
            }
        }
    }

    return bytes;
}

// Retorna la cantidad de frames ocupados por el proceso
int tabla_paginas_frames_ocupados(t_args *args, t_proceso *proceso)
{
    int frames = 0;

    for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);

        if (pagina != NULL && pagina->validez == 1)
        {
            frames++;
        }
    }

    return frames;
}

// Retorna el marco y los bytes de la página que se encuentra en el marco pasado por parámetro, y el offset sobre el cual estan escritos los bytes
t_frame_bytes *tabla_paginas_frame_bytes(t_args *args, t_proceso *proceso, uint32_t numero_marco)
{
    for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);

        if (pagina != NULL && pagina->validez == 1 && pagina->marco == numero_marco)
        {
            t_frame_bytes *frame_bytes = malloc(sizeof(t_frame_bytes));
            frame_bytes->marco = pagina->marco;
            frame_bytes->bytes = pagina->bytes;
            frame_bytes->offset = pagina->offset;
            return frame_bytes;
        }
    }

    return NULL;
}

// Retorna el número de página a partir de la dirección física
int obtener_numero_pagina(uint32_t direccion_fisica, uint32_t tam_pagina)
{
    return direccion_fisica / tam_pagina;
}