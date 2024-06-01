#include <utils/memoria.h>

void tabla_paginas_inicializar(t_args *args, t_proceso *proceso)
{
    int paginas = args->memoria.tamMemoria / args->memoria.tamPagina;
    proceso->tabla_paginas = list_create();

    for (int i = 0; i < paginas; i++)
    {
        /* 
        La tabla de páginas es una lista de páginas. Cada página tiene un marco y un bit de validez.
        
        Para verificar la validez, se verifica contra el bit de validez.
        Si vale 0, es inválida. Si vale 1, es válida.
        No importa lo que haya escrito en el marco, si el bit de validez es 0, no se puede acceder a la página porque el marco indicado ya no es parte del proceso.
        
        El índice de la lista de páginas es el número de página, de 0 a n-1 */

        t_pagina *pagina = malloc(sizeof(t_pagina));
        pagina->marco = 0;
        pagina->validez = 0;
        list_add(proceso->tabla_paginas, pagina);
    }

    log_info(args->logger, "Creación tabla de páginas: PID: <%d> - Tamaño: <%d>", proceso->pid, list_size(proceso->tabla_paginas));
}

void tabla_paginas_liberar(t_args *argumentos, t_proceso *proceso)
{
    for(int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);
        bitmap_marcar_libre(argumentos, argumentos->memoria.bitmap, pagina->marco);
        espacio_usuario_liberar_dato(argumentos, pagina->marco, argumentos->memoria.tamPagina);
    }

    log_info(argumentos->logger, "Destruccion tabla de páginas: PID: <%d> - Tamaño: <%d>", proceso->pid, list_size(proceso->tabla_paginas));
    list_destroy_and_destroy_elements(proceso->tabla_paginas, free);
}

uint32_t tabla_paginas_acceder(t_args *argumentos, uint32_t pid, uint32_t numero_pagina)
{
    t_proceso *proceso = buscar_proceso(argumentos, pid);

    if (proceso == NULL)
    {
        return -1;
    }

    int marco = -1;

    for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);

        if (i == numero_pagina && pagina->validez == 1)
        {
            marco = pagina->marco;
            break;
        }
    }

    if (marco == -1)
    {
        log_error(argumentos->logger, "La pagina %d no existe en la tabla de paginas del proceso %d", numero_pagina, proceso->pid);
        return -1;
    }

    log_info(argumentos->logger, "Acceso a tabla de páginas: PID: <%d> - Pagina: <%d> - Marco: <%d>", proceso->pid, numero_pagina, marco);
    
    return marco;
}

void tabla_paginas_asignar(t_args *argumentos, uint32_t pid, uint32_t numero_pagina, uint32_t numero_marco)
{
    t_proceso *proceso = buscar_proceso(argumentos, pid);

    if (proceso == NULL)
    {
        return;
    }

    for (int i = 0; i < list_size(proceso->tabla_paginas); i++)
    {
        t_pagina *pagina = list_get(proceso->tabla_paginas, i);

        if (i == numero_pagina)
        {
            pagina->marco = numero_marco;
            pagina->validez = 1;
            bitmap_marcar_ocupado(argumentos, argumentos->memoria.bitmap, numero_marco);
            break;
        }
    }

    log_info(argumentos->logger, "Asignación de tabla de páginas: PID: <%d> - Pagina: <%d> - Marco: <%d>", proceso->pid, numero_pagina, numero_marco);
}

int obtener_numero_pagina(uint32_t direccion_fisica, uint32_t tam_pagina)
{
    return direccion_fisica / tam_pagina;
}