#include <utils/memoria.h>

/* LOGS:
Creación / destrucción de Tabla de Páginas: “PID: <PID> - Tamaño: <CANTIDAD_PAGINAS>”
Acceso a Tabla de Páginas: “PID: <PID> - Pagina: <PAGINA> - Marco: <MARCO>”
*/

void memoria_crear_tabla_paginas(t_args *argumentos, uint32_t pid)
{
    t_proceso *proceso = buscar_proceso(argumentos, pid);

    int cantidad_max_paginas = argumentos->memoria.tamMemoria / argumentos->memoria.tamPagina;

    if (proceso == NULL)
    {
        return;
    }

    proceso->tabla_paginas = list_create();

    for (int i = 0; i < cantidad_max_paginas; i++)
    {

        t_pagina *pagina = malloc(sizeof(t_pagina));
        pagina->numero_pagina = i;
        pagina->numero_marco = -1; // -1 indica que la página no está en un frame
        pagina->en_uso = false;
        list_add(proceso->tabla_paginas, pagina);
    }

    log_info(argumentos->logger, "PID: %d - Tamaño: %d", proceso->pid, list_size(proceso->tabla_paginas));
}

void memoria_destruir_tabla_paginas(t_args *argumentos, uint32_t pid)
{
    t_proceso *proceso = buscar_proceso(argumentos, pid);

    if (proceso == NULL)
    {
        return;
    }
    log_info(argumentos->logger, "PID: %d - Tamaño: %d", proceso->pid, list_size(proceso->tabla_paginas));
    list_destroy_and_destroy_elements(proceso->tabla_paginas, free);
}

int memoria_acceder_tabla_paginas(t_args *argumentos, uint32_t pid, int numero_pagina)
{
    t_proceso *proceso = buscar_proceso(argumentos, pid);
    int marco = 0; // Inicializado en 0, debería buscar la página en la tabla de páginas y devolver el marco

    if (proceso == NULL)
    {
        return -1;
    }

    if (proceso->tabla_paginas == NULL)
    {
        log_error(argumentos->logger, "No se encontró la tabla de páginas del proceso con PID: %d", proceso->pid);
        return -1;
    }

    for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);

        if (pagina->numero_pagina == numero_pagina)
        {
            marco = pagina->numero_marco;
            break;
        }
    }

    log_info(argumentos->logger, "PID: %d - Pagina: %d - Marco: %d", proceso->pid, numero_pagina, marco);
    return marco;
}
