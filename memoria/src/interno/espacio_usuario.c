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

    log_debug(args->logger, "Se creo el espacio de usuario con %d bytes y %d frames disponibles.", args->memoria.tamMemoria,frames);

    // Inicializo toda la memoria en 0xFF
    memset(args->memoria.espacio_usuario, 0xFF, args->memoria.tamMemoria);

    args->memoria.bytes_usados = (int *)calloc(frames, sizeof(int));
    
    // Reviso que se haya podido asignar memoria
    if (args->memoria.bytes_usados == NULL)
    {
        log_error(args->logger, "Calloc falló al asignar %ld bytes para el array de bytes usados por frame.", frames * sizeof(int));
        exit(1);
    }

    bitmap_inicializar(args);
}

// Libero el espacio de usuario contiguo
void espacio_usuario_liberar(t_args *args)
{
    free(args->memoria.espacio_usuario);
    log_debug(args->logger, "Se libero %d bytes del espacio de usuario.", args->memoria.tamMemoria);
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

// Obtiene el frame correspondiente a una dirección física
uint32_t espacio_usuario_obtener_frame(uint32_t direccion_fisica, uint32_t tamPagina)
{
    return direccion_fisica / tamPagina;
}

// Función para escribir datos con verificación del bitmap y la dirección específica
void espacio_usuario_escribir_dato(t_args *args, uint32_t direccion_fisica, void *dato, size_t tamano)
{
    if (direccion_fisica + tamano > args->memoria.tamMemoria)
    {
        log_error(args->logger, "Se intento escribir en la direccion fisica %d, pero el limite del espacio de usuario es %d.", direccion_fisica, args->memoria.tamMemoria);
        return;
    }

    // Verificar si ya hay datos en la dirección física
    uint8_t *destino = (uint8_t *)args->memoria.espacio_usuario + direccion_fisica;
    for (size_t i = 0; i < tamano; i++)
    {
        if (destino[i] != 0xFF)
        { 
            log_warning(args->logger, "Sobrescribiendo datos en la dirección física %ld.", direccion_fisica + i);
            break;
        }
    }

    // Obtengo los frames de inicio y fin
    uint32_t frame_inicio = espacio_usuario_obtener_frame(direccion_fisica, args->memoria.tamPagina);
    uint32_t frame_fin = espacio_usuario_obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);

    // Marco los frames como ocupados y actualizo el array de bytes usados
    for (uint32_t frame = frame_inicio; frame <= frame_fin; frame++)
    {
        bitmap_marcar_ocupado(args, args->memoria.bitmap, frame);

        // Calculo los bytes a actualizar en este frame
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

        args->memoria.bytes_usados[frame] += (offset_fin - offset_inicio + 1);
    }

    // Notifico que se escribio el dato
    log_debug(args->logger, "Se escribio el dato de %ld bytes partiendo de la dirección física %d (%d -> %ld)", tamano, direccion_fisica, direccion_fisica, direccion_fisica + tamano - 1);

    // Escribo los datos
    memcpy(destino, dato, tamano);
}

// Función para liberar frames con verificación del bitmap y tamaño del dato
void espacio_usuario_liberar_dato(t_args *args, uint32_t direccion_fisica, size_t tamano)
{
    // Verificar que la operación no se salga de los límites de la memoria
    if (direccion_fisica + tamano > args->memoria.tamMemoria)
    {
        log_error(args->logger, "Se intento liberar %ld bytes del espacio de usuario, pero la direccion fisica %ld queda por fuera del limite de %d bytes.", tamano, direccion_fisica + tamano, args->memoria.tamMemoria);
        return;
    }

    // Calcular los frames que ocupan el dato basado en la dirección física y el tamaño
    uint32_t frame_inicio = direccion_fisica / args->memoria.tamPagina;
    uint32_t frame_fin = (direccion_fisica + tamano - 1) / args->memoria.tamPagina;

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

        // Verificar si el frame está ocupado
        if (bitmap_frame_libre(args->memoria.bitmap, frame))
        {
            log_warning(args->logger, "El frame %u ya está libre. No hace falta liberarlo.", frame);
            continue;
        }

        // Marcar el frame como libre en el bitmap
        bitmap_marcar_libre(args, args->memoria.bitmap, frame);

        log_debug(args->logger, "Se liberó el frame %u en el bitmap y en espacio de usuario.", frame);

        // Calcular la dirección física inicial del frame
        uint32_t direccion_frame = frame * args->memoria.tamPagina;

        // Reemplazar el contenido del frame por 0xFF
        memset((uint8_t *)args->memoria.espacio_usuario + direccion_frame, 0xFF, args->memoria.tamPagina);
    }
}

// Retorna el índice del primer frame con suficiente espacio para un dato de tamaño especificado
int espacio_usuario_proximo_frame(t_args *args, size_t tamano)
{
    uint32_t tamPagina = args->memoria.tamPagina;
    uint32_t num_frames = args->memoria.tamMemoria / tamPagina;

    for (uint32_t frame = 0; frame < num_frames; frame++)
    {
        // Calculo el espacio libre en este frame
        uint32_t espacio_libre = tamPagina - args->memoria.bytes_usados[frame];

        if (espacio_libre >= tamano)
        {
            // Si el espacio libre en este frame es suficiente, devuelvo el índice del frame
            return frame;
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
        // Calculo el espacio libre en este frame
        uint32_t espacio_libre = tamPagina - args->memoria.bytes_usados[frame];

        if (espacio_libre >= tamano)
        {
            // Si el espacio libre en este frame es suficiente, devuelvo la dirección física inicial
            uint32_t direccion_fisica_inicio = frame * tamPagina + args->memoria.bytes_usados[frame];
            return (int) direccion_fisica_inicio;
        }
    }

    return -1;
}

// Escribir un entero
void espacio_usuario_escribir_int(t_args *args, uint32_t direccion_fisica, int valor)
{
    espacio_usuario_escribir_dato(args, direccion_fisica, &valor, sizeof(int));
}

// Escribir un uint32_t
void espacio_usuario_escribir_uint32(t_args *args, uint32_t direccion_fisica, uint32_t valor)
{
    espacio_usuario_escribir_dato(args, direccion_fisica, &valor, sizeof(uint32_t));
}

// Escribir un flotante
void espacio_usuario_escribir_float(t_args *args, uint32_t direccion_fisica, float valor)
{
    espacio_usuario_escribir_dato(args, direccion_fisica, &valor, sizeof(float));
}

// Escribir una cadena
void espacio_usuario_escribir_char(t_args *args, uint32_t direccion_fisica, const char *cadena)
{
    espacio_usuario_escribir_dato(args, direccion_fisica, (void *)cadena, strlen(cadena));
}

// Escribir un "algo" genérico
void espacio_usuario_escribir_generic(t_args *args, uint32_t direccion_fisica, void *estructura, size_t tamano_estructura)
{
    espacio_usuario_escribir_dato(args, direccion_fisica, estructura, tamano_estructura);
}

// Leer un dato genérico
void espacio_usuario_leer_dato(t_args *args, uint32_t direccion_fisica, void *destino, size_t tamano)
{
    // Reviso que la dirección física esté dentro de los límites del espacio de usuario
    if (direccion_fisica + tamano > args->memoria.tamMemoria)
    {
        log_error(args->logger, "Se intento leer fuera de los límites del espacio de usuario.");
        return;
    }

    // Obtengo la dirección de origen
    uint8_t *origen = (uint8_t *)args->memoria.espacio_usuario + direccion_fisica;

    // Reviso que la dirección física tenga datos
    for (size_t i = 0; i < tamano; i++)
    {
        if (origen[i] == 0xFF)
        {
            log_warning(args->logger, "Se intento leer la dirección física %lu, pero está vacía.", direccion_fisica + i);
            return;
        }
    }

    // Determino los frames de inicio y fin
    uint32_t frame_inicio = espacio_usuario_obtener_frame(direccion_fisica, args->memoria.tamPagina);
    uint32_t frame_fin = espacio_usuario_obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);

    // Alerto si es que la lectura abarca más de un frame
    if (frame_inicio != frame_fin)
    {
        log_debug(args->logger, "La lectura de %ld bytes desde la direccion fisica %d (%ld -> %ld) comienza en el frame %d hasta el %d.",tamano,direccion_fisica, tamano, tamano + direccion_fisica,frame_inicio, frame_fin);
    }

    // Copio los datos
    memcpy(destino, origen, tamano);
}

// Leer un entero
int espacio_usuario_leer_int(t_args *args, uint32_t direccion_fisica)
{
    int valor;
    espacio_usuario_leer_dato(args, direccion_fisica, &valor, sizeof(int));
    return valor;
}

// Leer un uint32_t
uint32_t espacio_usuario_leer_uint32(t_args *args, uint32_t direccion_fisica)
{
    uint32_t valor;
    espacio_usuario_leer_dato(args, direccion_fisica, &valor, sizeof(uint32_t));
    return valor;
}

// Leer un flotante
float espacio_usuario_leer_float(t_args *args, uint32_t direccion_fisica)
{
    float valor;
    espacio_usuario_leer_dato(args, direccion_fisica, &valor, sizeof(float));
    return valor;
}

// Leer un char*
char* espacio_usuario_leer_char(t_args *args, uint32_t direccion_fisica, size_t tamano_max)
{
    int bytes_totales = tamano_max+1;
    char *destino = malloc(bytes_totales);
    espacio_usuario_leer_dato(args, direccion_fisica, destino, tamano_max);
    destino[bytes_totales-1] = '\0';
    return destino;
}

// Leer un "algo" genérico
void espacio_usuario_leer_generic(t_args *args, uint32_t direccion_fisica, void *estructura, size_t tamano_estructura)
{
    espacio_usuario_leer_dato(args, direccion_fisica, estructura, tamano_estructura);
}