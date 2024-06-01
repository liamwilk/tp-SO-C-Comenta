#include <utils/memoria.h>

void tabla_paginas_inicializar(t_args *args, t_proceso *proceso)
{
    int paginas = args->memoria.tamMemoria / args->memoria.tamPagina;
    proceso->tabla_paginas = list_create();

    if (proceso->tabla_paginas == NULL)
    {
        log_error(args->logger, "Error al crear la tabla de páginas para el proceso %d", proceso->pid);
        return;
    }

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

    log_info(args->logger, "Creación tabla de páginas: PID: <%d> - Tamaño: <%d>", proceso->pid, list_size(proceso->tabla_paginas));
}

void tabla_paginas_liberar(t_args *argumentos, t_proceso *proceso)
{
    for(int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);
        if(pagina != NULL) {
            bitmap_marcar_libre(argumentos, argumentos->memoria.bitmap, pagina->marco);
        }
    }

    log_info(argumentos->logger, "Destrucción tabla de páginas: PID: <%d> - Tamaño: <%d>", proceso->pid, list_size(proceso->tabla_paginas));
    list_destroy_and_destroy_elements(proceso->tabla_paginas, free);
}

void tabla_paginas_liberar_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina)
{
    if (numero_pagina >= list_size(proceso->tabla_paginas)) {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
        return;
    }

    t_pagina *pagina = list_get(proceso->tabla_paginas, numero_pagina);
    
    if(pagina == NULL) {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
        return;
    }

    bitmap_marcar_libre(argumentos, argumentos->memoria.bitmap, pagina->marco);

    pagina->marco = 0;
    pagina->validez = 0;
    pagina->bytes = 0;
    pagina->offset = 0;

    log_info(argumentos->logger, "Liberación de tabla de páginas: PID: <%d> - Página: <%d>", proceso->pid, numero_pagina);
}

int tabla_paginas_acceder_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina)
{
    if (numero_pagina >= list_size(proceso->tabla_paginas)) {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
        return -1;
    }

    t_pagina *pagina = list_get(proceso->tabla_paginas, numero_pagina);
    int marco = -1;

    if (pagina != NULL && pagina->validez == 1) {
        marco = pagina->marco;
    }

    if (marco == -1) {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, proceso->pid);
        return -1;
    }

    log_info(argumentos->logger, "Acceso a tabla de páginas: PID: <%d> - Página: <%d> - Marco: <%d>", proceso->pid, numero_pagina, marco);
    
    return marco;
}

void tabla_paginas_asignar_pagina(t_args *argumentos, t_proceso *proceso, uint32_t numero_pagina, uint32_t numero_marco)
{
    if (numero_pagina >= list_size(proceso->tabla_paginas)) {
        log_error(argumentos->logger, "La página %d no existe en la tabla de páginas del proceso %d", numero_pagina, pid);
        return;
    }

    t_pagina *pagina = list_get(proceso->tabla_paginas, numero_pagina);

    if (pagina != NULL) {
        pagina->marco = numero_marco;
        pagina->validez = 1;
        bitmap_marcar_ocupado(argumentos, argumentos->memoria.bitmap, numero_marco);

        log_info(argumentos->logger, "Asignación de tabla de páginas: PID: <%d> - Página: <%d> - Marco: <%d>", proceso->pid, numero_pagina, numero_marco);
    } else {
        log_error(argumentos->logger, "No se pudo asignar la página %d del proceso %d", numero_pagina, pid);
    }
}

int obtener_numero_pagina(uint32_t direccion_fisica, uint32_t tam_pagina)
{
    return direccion_fisica / tam_pagina;
}