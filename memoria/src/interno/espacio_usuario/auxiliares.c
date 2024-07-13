#include <utils/memoria.h>

void espacio_usuario_inicializar(t_args *args)
{
    // Pido el bloque de memoria contigua
    args->memoria.espacio_usuario = malloc(args->memoria.tamMemoria);

    // Reviso que se haya podido asignar memoria
    if (args->memoria.espacio_usuario == NULL)
    {
        log_error(args->logger, "Malloc falló al asignar %d bytes para el bloque contiguo de memoria.", args->memoria.tamMemoria);
        exit(1);
    }

    // Calculo la cantidad de frames disponibles
    int frames = args->memoria.tamMemoria / args->memoria.tamPagina;

    log_debug(args->logger, "Se creo el espacio de usuario con %d bytes y %d frames disponibles.", args->memoria.tamMemoria, frames);

    // Inicializo toda la memoria en \n
    memset(args->memoria.espacio_usuario, '\n', args->memoria.tamMemoria);

    args->memoria.bytes_usados = (int *)calloc(frames, sizeof(int));
    args->memoria.bitmap_array = (int *)calloc(frames, sizeof(int));

    // Reviso que se haya podido asignar memoria
    if (args->memoria.bytes_usados == NULL || args->memoria.bitmap_array == NULL)
    {
        log_error(args->logger, "Calloc falló al asignar %ld bytes.", frames * sizeof(int));
        exit(1);
    }
}

void espacio_usuario_liberar(t_args *args)
{
    free(args->memoria.espacio_usuario);
    free(args->memoria.bytes_usados);
    free(args->memoria.bitmap_array);
    log_debug(args->logger, "Se libero el espacio de usuario.");
}

int espacio_usuario_bytes_disponibles(t_args *args)
{
    int cantidad_procesos = list_size(args->memoria.lista_procesos);

    int bytes_usados = 0;

    for (int i = 0; i < cantidad_procesos; i++)
    {
        t_proceso *proceso = buscar_proceso(args, i);

        if (proceso != NULL)
        {
            bytes_usados += tabla_paginas_bytes_ocupados(args, proceso);
        }
    }

    return args->memoria.tamMemoria - bytes_usados;
}

int espacio_usuario_liberar_dato(t_args *args, uint32_t direccion_fisica, size_t tamano)
{
    if (tamano == 0)
    {
        return -1;
    }

    // Verifico que la operación no se salga de los límites de la memoria
    if (direccion_fisica + tamano > args->memoria.tamMemoria)
    {
        log_error(args->logger, "Se intentó liberar %ld bytes del espacio de usuario, pero la dirección física %ld queda por fuera del límite de %d bytes.", tamano, direccion_fisica + tamano, args->memoria.tamMemoria);
        return -1;
    }

    // Determino los frames de inicio y fin
    uint32_t frame_inicio = espacio_usuario_obtener_frame(direccion_fisica, args->memoria.tamPagina);
    uint32_t frame_fin = espacio_usuario_obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);

    // Recorro y libero cada frame ocupado por el dato
    for (uint32_t frame = frame_inicio; frame <= frame_fin; frame++)
    {
        // Calculo los bytes a liberar en este frame
        uint32_t offset_inicio = (frame == frame_inicio) ? (direccion_fisica % args->memoria.tamPagina) : 0;
        uint32_t offset_fin = (frame == frame_fin) ? ((direccion_fisica + tamano - 1) % args->memoria.tamPagina) : (args->memoria.tamPagina - 1);

        args->memoria.bytes_usados[frame] -= (offset_fin - offset_inicio + 1);

        // Marco el frame como libre en el bitmap
        args->memoria.bitmap_array[frame] = 0;

        // Calculo la dirección física inicial del frame
        uint32_t direccion_frame = frame * args->memoria.tamPagina;

        // Borro los datos del frame
        memset((uint8_t *)args->memoria.espacio_usuario + direccion_frame + offset_inicio, '\n', offset_fin - offset_inicio + 1);
    }

    return 0;
}

uint32_t espacio_usuario_obtener_frame(uint32_t direccion_fisica, uint32_t tamPagina)
{
    return direccion_fisica / tamPagina;
}

int espacio_usuario_proximo_frame(t_args *args, size_t tamano)
{
    uint32_t tamPagina = args->memoria.tamPagina;
    uint32_t num_frames = args->memoria.tamMemoria / tamPagina;

    for (uint32_t frame = 0; frame < num_frames; frame++)
    {
        if (args->memoria.bitmap_array[frame] == 0)
        {
            // Calculo el espacio libre en este frame
            uint32_t espacio_libre = tamPagina - args->memoria.bytes_usados[frame];

            if (espacio_libre >= tamano)
            {
                // Si el espacio libre en este frame es suficiente, devuelvo el índice del frame
                return frame;
            }
        }
    }

    // Si no hay ningún frame con suficiente espacio, devuelvo -1
    return -1;
}

// Corto el char en pedazos de frame_size bytes
t_char_fragmentado *espacio_usuario_fragmentar_char(char *input, int frame_size)
{
    // Calculo el tamaño de la entrada
    int input_length = strlen(input);

    // Calculo la cantidad de fragmentos necesarios
    int cantidad = (input_length + frame_size - 1) / frame_size;

    t_char_fragmentado *result = (t_char_fragmentado *)malloc(sizeof(t_char_fragmentado));
    result->cantidad = cantidad;
    result->fragmentos = (char **)malloc(cantidad * sizeof(char *));
    result->tamanos = (int *)malloc(cantidad * sizeof(int));

    // Corto la cadena en fragmentos y los almaceno en el array
    for (int i = 0; i < cantidad; i++)
    {
        int start_index = i * frame_size;
        int fragment_length = frame_size;

        // Ajusto el tamaño del último fragmento si es necesario
        if (start_index + frame_size > input_length)
        {
            fragment_length = input_length - start_index;
        }

        // Reservo memoria para el fragmento y copio la parte correspondiente de la cadena
        result->fragmentos[i] = (char *)malloc((fragment_length + 1) * sizeof(char));
        strncpy(result->fragmentos[i], input + start_index, fragment_length);
        result->fragmentos[i][fragment_length] = '\0';
        result->tamanos[i] = fragment_length;
    }

    return result;
}