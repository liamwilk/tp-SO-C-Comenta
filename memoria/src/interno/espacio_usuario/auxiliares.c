#include <utils/memoria.h>

// Inicializo el espacio de usuario contiguo
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

    // bitmap_inicializar(args);
}

// Libero el espacio de usuario contiguo
void espacio_usuario_liberar(t_args *args)
{
    free(args->memoria.espacio_usuario);
    free(args->memoria.bytes_usados);
    free(args->memoria.bitmap_array);
    log_debug(args->logger, "Se libero el espacio de usuario.");
}

// Calcula la cantidad de bytes ocupados en espacio de usuario
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

// Función para liberar frames con verificación del bitmap y tamaño del dato
int espacio_usuario_liberar_dato(t_args *args, uint32_t direccion_fisica, size_t tamano)
{
    if (tamano == 0)
    {
        log_warning(args->logger, "Se intento liberar un dato de tamaño 0 bytes.");
        return -1;
    }

    // Verificar que la operación no se salga de los límites de la memoria
    if (direccion_fisica + tamano > args->memoria.tamMemoria)
    {
        log_error(args->logger, "Se intento liberar %ld bytes del espacio de usuario, pero la direccion fisica %ld queda por fuera del limite de %d bytes.", tamano, direccion_fisica + tamano, args->memoria.tamMemoria);
        return -1;
    }

    // Determino los frames de inicio y fin
    uint32_t frame_inicio = espacio_usuario_obtener_frame(direccion_fisica, args->memoria.tamPagina);
    uint32_t frame_fin = espacio_usuario_obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);

    // Recorrer y liberar cada frame ocupado por el dato
    for (uint32_t frame = frame_inicio; frame <= frame_fin; frame++)
    {
        // Calculo los bytes a liberar en este frame
        uint32_t offset_inicio, offset_fin;
        if (frame == frame_inicio)
        {
            offset_inicio = direccion_fisica % args->memoria.tamPagina;
        }
        else
        {
            offset_inicio = 0;
        }

        if (frame == frame_fin)
        {
            offset_fin = (direccion_fisica + tamano - 1) % args->memoria.tamPagina;
        }
        else
        {
            offset_fin = args->memoria.tamPagina - 1;
        }

        args->memoria.bytes_usados[frame] -= (offset_fin - offset_inicio + 1);

        // Marcar el frame como libre en el bitmap
        args->memoria.bitmap_array[frame] = 0;

        // Calcular la dirección física inicial del frame
        uint32_t direccion_frame = frame * args->memoria.tamPagina;

        // Borro los datos del frame
        memset((uint8_t *)args->memoria.espacio_usuario + direccion_frame, '\n', args->memoria.tamPagina);
    }
    return 0;
}

// Obtiene el frame correspondiente a una dirección física
uint32_t espacio_usuario_obtener_frame(uint32_t direccion_fisica, uint32_t tamPagina)
{
    return direccion_fisica / tamPagina;
}

// Busca un frame con suficiente espacio para contener un dato de tamaño especificado, y retorna la dirección física y el índice del frame
t_frame_disponible *espacio_usuario_buscar_frame(t_args *args, size_t size_buscado)
{
    if (size_buscado > args->memoria.tamPagina)
    {
        log_error(args->logger, "Se solicito buscar un espacio de %ld bytes en un solo frame, pero es mayor al tamaño de frame máximo de %d bytes.", size_buscado, args->memoria.tamPagina);
        return NULL;
    }

    t_frame_disponible *frame_disponible = malloc(sizeof(t_frame_disponible));

    frame_disponible->direccion_fisica = espacio_usuario_proxima_direccion(args, size_buscado);
    frame_disponible->frame = espacio_usuario_proximo_frame(args, size_buscado);

    if (frame_disponible->direccion_fisica != -1 && frame_disponible->frame != -1)
    {
        log_debug(args->logger, "Encontrada la dirección física %u en el frame %d con suficiente espacio para %ld bytes", frame_disponible->direccion_fisica, frame_disponible->frame, size_buscado);
    }
    else
    {
        log_error(args->logger, "No se encontró una dirección física con suficiente espacio para %zu bytes en ningun frame.", size_buscado);
        free(frame_disponible);
        return NULL;
    }

    return frame_disponible;
}

// Retorna la cantidad de bytes usados en un frame del espacio de usuario
int espacio_usuario_frame_bytes(t_args *args, uint32_t frame)
{
    return args->memoria.bytes_usados[frame];
}

// Retorna el índice del primer frame con suficiente espacio para un dato de tamaño especificado
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

// Retorna la dirección física del primer frame con suficiente espacio para un dato de tamaño especificado
int espacio_usuario_proxima_direccion(t_args *args, size_t tamano)
{
    uint32_t tamPagina = args->memoria.tamPagina;
    uint32_t num_frames = args->memoria.tamMemoria / tamPagina;

    for (uint32_t frame = 0; frame < num_frames; frame++)
    {
        // Verficio si el frame está libre
        if (args->memoria.bitmap_array[frame] == 0)
        {
            // Calculo el espacio libre en este frame
            uint32_t espacio_libre = tamPagina - args->memoria.bytes_usados[frame];

            if (espacio_libre >= tamano)
            {
                // Si el espacio libre en este frame es suficiente, devuelvo la dirección física inicial
                uint32_t direccion_fisica_inicio = frame * tamPagina + args->memoria.bytes_usados[frame];
                return (int)direccion_fisica_inicio;
            }
        }
    }
    return -1;
}

// Corto el char en pedazos de frame_size bytes
t_char_framentado *espacio_usuario_fragmentar_char(char *input, int frame_size)
{
    // Calculo el tamaño de la entrada
    int input_length = strlen(input);

    // Calculo la cantidad de fragmentos necesarios
    int cantidad = (input_length + frame_size - 1) / frame_size;

    t_char_framentado *result = (t_char_framentado *)malloc(sizeof(t_char_framentado));
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

// Libero la memoria de un t_char_framentado
void espacio_usuario_fragmentos_liberar(t_args *args, t_char_framentado *fs)
{
    for (int i = 0; i < fs->cantidad; i++)
    {
        free(fs->fragmentos[i]);
    }
    free(fs->fragmentos);
    free(fs->tamanos);
    free(fs);
}

// Imprimo un t_char_framentado
void espacio_usuario_fragmentos_imprimir(t_args *args, t_char_framentado *fs)
{
    for (int i = 0; i < fs->cantidad; i++)
    {
        log_debug(args->logger, "Cadena %d: %s (%d bytes)", i, fs->fragmentos[i], fs->tamanos[i]);
    }
}