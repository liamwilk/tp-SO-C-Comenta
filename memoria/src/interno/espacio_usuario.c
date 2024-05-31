#include "espacio_usuario.h"

// Inicializo el espacio de usuario contiguo
void espacio_usuario_inicializar_contiguo(t_args *args)
{
    // Pido el bloque de memoria contigua
    args->memoria.espacio_usuario = malloc(args->memoria.tamMemoria);

    // Reviso que se haya podido asignar memoria
    if (args->memoria.espacio_usuario == NULL)
    {
        log_error(args->logger, "Malloc falló al asignar %d bytes para el bloque contiguo de memoria.", args->memoria.tamMemoria);
        exit(1);
    }

    log_debug(args->logger, "Se creo el espacio de usuario con %d bytes.", args->memoria.tamMemoria);

    // Inicializo toda la memoria en 0xFF
    memset(args->memoria.espacio_usuario, 0xFF, args->memoria.tamMemoria);

    args->memoria.bytes_usados = (int *)malloc(args->memoria.tamMemoria * sizeof(int));

    // Reviso que se haya podido asignar memoria
    if (args->memoria.bytes_usados == NULL)
    {
        log_error(args->logger, "Malloc falló al asignar %ld bytes para el array de bytes usados por frame.", args->memoria.tamMemoria * sizeof(int));
        exit(1);
    }

    // Inicializo el array de bytes usados por frame en 0
    for (int i = 0; i < args->memoria.tamMemoria; i++)
    {
        args->memoria.bytes_usados[i] = 0;
    }

    espacio_usuario_inicializar_bitmap(args);
}

// Libero el espacio de usuario contiguo
void espacio_usuario_liberar_contiguo(t_args *args)
{
    free(args->memoria.espacio_usuario);
    log_debug(args->logger, "Se libero %d bytes del espacio de usuario.", args->memoria.tamMemoria);
}

// Inicializo el bitmap
void espacio_usuario_inicializar_bitmap(t_args *args)
{
    // Calcular el tamaño del bitmap
    size_t tamano_bitmap = (args->memoria.tamMemoria / args->memoria.tamPagina + 7) / 8;

    // Asignar memoria para el bitmap
    args->memoria.bitmap_data = malloc(tamano_bitmap);

    // Reviso que se haya podido asignar memoria para el bitmap
    if (args->memoria.bitmap_data == NULL)
    {
        log_error(args->logger, "Malloc falló al asignar %zu bytes para el bitmap.", tamano_bitmap);
        espacio_usuario_liberar_contiguo(args);
        exit(1);
    }

    // Inicializar el bitmap en 0
    memset(args->memoria.bitmap_data, 0, tamano_bitmap);

    // Crear el bitmap
    args->memoria.bitmap = bitarray_create_with_mode(args->memoria.bitmap_data, tamano_bitmap, LSB_FIRST);

    // Reviso que se haya podido crear el bitmap
    if (args->memoria.bitmap == NULL)
    {
        log_error(args->logger, "No se pudo crear el bitmap para el espacio de usuario.");
        free(args->memoria.bitmap_data);
        espacio_usuario_liberar_contiguo(args);
        exit(1);
    }

    log_debug(args->logger, "Se creo el bitmap para el espacio de usuario.");

    // TODO: Quitar estos casos de prueba.

    char *cadena = "CURSADA DE SISTEMAS OPERATIVOS 1c 2024";
    log_debug(args->logger, "Tamaño de la cadena: %zu", strlen(cadena) + 1);
    escribir_char(args, 0, cadena);

    char cadena_leida1[strlen(cadena) + 1];
    leer_char(args, 0, cadena_leida1, strlen(cadena) + 1);
    log_debug(args->logger,"Cadena leída: %s", cadena_leida1);
    
    // espacio_usuario_liberar(args, 0, strlen(cadena) + 1);

    // log_debug(args->logger,"Tamaño de la cadena: %zu", strlen(cadena)+1);
    // escribir_char(args, 39, cadena);

    // char cadena_leida2[strlen(cadena) + 1];
    // leer_char(args, 39, cadena_leida2, strlen(cadena) + 1);
    // log_debug(args->logger,"Cadena leída: %s", cadena_leida2);

    // espacio_usuario_liberar(args, 39, strlen(cadena) + 1);

    size_t tamano_buscado = 9;
    uint32_t direccion_fisica_con_espacio = buscar_direccion_fisica_con_espacio(args, tamano_buscado);
    int marco_con_espacio = buscar_marco_con_espacio(args, tamano_buscado);

    if (direccion_fisica_con_espacio != -1)
    {
        log_debug(args->logger, "Encontrada la dirección física %u en el marco %d con suficiente espacio para %ld bytes", direccion_fisica_con_espacio,marco_con_espacio, tamano_buscado);
    }
    else
    {
        log_error(args->logger, "No se encontró una dirección física con suficiente espacio para %zu bytes.", tamano_buscado);
    }

    log_debug(args->logger,"Tamaño de la cadena: %zu", strlen(cadena)+1);
    escribir_char(args, 39, cadena);

    char cadena_leida2[strlen(cadena) + 1];
    leer_char(args, 39, cadena_leida2, strlen(cadena) + 1);
    log_debug(args->logger,"Cadena leída: %s", cadena_leida2);

    // espacio_usuario_liberar(args, 39, strlen(cadena) + 1);

    tamano_buscado = 2;
    direccion_fisica_con_espacio = buscar_direccion_fisica_con_espacio(args, tamano_buscado);
    marco_con_espacio = buscar_marco_con_espacio(args, tamano_buscado);

    if (direccion_fisica_con_espacio != -1)
    {
        log_debug(args->logger, "Encontrada la dirección física %u en el marco %d con suficiente espacio para %ld bytes", direccion_fisica_con_espacio,marco_con_espacio, tamano_buscado);
    }
    else
    {
        log_error(args->logger, "No se encontró una dirección física con suficiente espacio para %zu bytes.", tamano_buscado);
    }

}

// Libero el bitmap
void espacio_usuario_liberar_bitmap(t_args *args)
{
    bitarray_destroy(args->memoria.bitmap);
    log_debug(args->logger, "Se libero el bitmap del espacio de usuario.");
}

// Obtiene el frame correspondiente a una dirección física
uint32_t obtener_frame(uint32_t direccion_fisica, uint32_t tamPagina)
{
    return direccion_fisica / tamPagina;
}

// Marca un frame como usado en el bitmap
void marcar_frame_usado(t_args *args, t_bitarray *bitmap, uint32_t frame)
{
    if (!bitarray_test_bit(bitmap, frame))
    {
        bitarray_set_bit(bitmap, frame);
        log_debug(args->logger, "Se marcó el frame %d como ocupado en el bitmap.", frame);
    }
}

// Marca un frame como libre en el bitmap
void liberar_frame(t_bitarray *bitmap, uint32_t frame)
{
    bitarray_clean_bit(bitmap, frame);
}

// Verifica si un frame está libre en el bitmap
bool frame_esta_libre(t_bitarray *bitmap, uint32_t frame)
{
    return !bitarray_test_bit(bitmap, frame);
}

// Función para escribir datos con verificación del bitmap y la dirección específica
void espacio_usuario_escribir(t_args *args, uint32_t direccion_fisica, void *dato, size_t tamano)
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
        { // Suponiendo que la memoria vacía está inicializada a 0
            log_warning(args->logger, "Sobrescribiendo datos en la dirección física %ld.", direccion_fisica + i);
            break;
        }
    }

    // Obtengo los frames de inicio y fin
    uint32_t frame_inicio = obtener_frame(direccion_fisica, args->memoria.tamPagina);
    uint32_t frame_fin = obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);

    // Marco los frames como ocupados y actualizo el array de bytes usados
    for (uint32_t frame = frame_inicio; frame <= frame_fin; frame++)
    {
        marcar_frame_usado(args, args->memoria.bitmap, frame);

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
    log_debug(args->logger, "Se escribió el dato de tamaño %ld en la dirección física %d.", tamano, direccion_fisica);

    // Escribo los datos
    memcpy(destino, dato, tamano);
}

// Función para liberar frames con verificación del bitmap y tamaño del dato
void espacio_usuario_liberar(t_args *args, uint32_t direccion_fisica, size_t tamano)
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
        if (frame_esta_libre(args->memoria.bitmap, frame))
        {
            log_warning(args->logger, "El frame %u ya está libre. No hace falta liberarlo.", frame);
            continue;
        }

        // Marcar el frame como libre en el bitmap
        liberar_frame(args->memoria.bitmap, frame);

        log_debug(args->logger, "Se liberó el frame %u en el bitmap y en espacio de usuario.", frame);

        // Calcular la dirección física inicial del frame
        uint32_t direccion_frame = frame * args->memoria.tamPagina;

        // Reemplazar el contenido del frame por 0xFF
        memset((uint8_t *)args->memoria.espacio_usuario + direccion_frame, 0xFF, args->memoria.tamPagina);
    }
}

int buscar_marco_con_espacio(t_args *args, size_t tamano)
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

uint32_t buscar_direccion_fisica_con_espacio(t_args *args, size_t tamano)
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
            return direccion_fisica_inicio;
        }
    }

    return -1;
}

// Escribir un entero
void escribir_int(t_args *args, uint32_t direccion_fisica, int valor)
{
    espacio_usuario_escribir(args, direccion_fisica, &valor, sizeof(int));
}

// Escribir un uint32_t
void escribir_uint32(t_args *args, uint32_t direccion_fisica, uint32_t valor)
{
    espacio_usuario_escribir(args, direccion_fisica, &valor, sizeof(uint32_t));
}

// Escribir un flotante
void escribir_float(t_args *args, uint32_t direccion_fisica, float valor)
{
    espacio_usuario_escribir(args, direccion_fisica, &valor, sizeof(float));
}

// Escribir una cadena
void escribir_char(t_args *args, uint32_t direccion_fisica, const char *cadena)
{
    espacio_usuario_escribir(args, direccion_fisica, (void *)cadena, strlen(cadena) + 1);
}

// Escribir un "algo" genérico
void escribir_generic(t_args *args, uint32_t direccion_fisica, void *estructura, size_t tamano_estructura)
{
    espacio_usuario_escribir(args, direccion_fisica, estructura, tamano_estructura);
}

// Leer un dato genérico
void espacio_usuario_leer(t_args *args, uint32_t direccion_fisica, void *destino, size_t tamano)
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
    uint32_t frame_inicio = obtener_frame(direccion_fisica, args->memoria.tamPagina);
    uint32_t frame_fin = obtener_frame(direccion_fisica + tamano - 1, args->memoria.tamPagina);

    // Alerto si es que la lectura abarca más de un frame
    if (frame_inicio != frame_fin)
    {
        log_warning(args->logger, "La lectura atraviesa más de un frame de memoria: desde el %d hasta el %d.", frame_inicio, frame_fin);
    }

    // Copio los datos
    memcpy(destino, origen, tamano);
}

// Leer un entero
int leer_int(t_args *args, uint32_t direccion_fisica)
{
    int32_t valor;
    espacio_usuario_leer(args, direccion_fisica, &valor, sizeof(int));
    return valor;
}

// Leer un uint32_t
uint32_t leer_uint32(t_args *args, uint32_t direccion_fisica)
{
    uint32_t valor;
    espacio_usuario_leer(args, direccion_fisica, &valor, sizeof(uint32_t));
    return valor;
}

// Leer un flotante
float leer_float(t_args *args, uint32_t direccion_fisica)
{
    float valor;
    espacio_usuario_leer(args, direccion_fisica, &valor, sizeof(float));
    return valor;
}

// Leer un char*
void leer_char(t_args *args, uint32_t direccion_fisica, char *destino, size_t tamano_max)
{
    espacio_usuario_leer(args, direccion_fisica, destino, tamano_max);

    // Asegurarse de que la cadena esté terminada en nulo
    destino[tamano_max - 1] = '\0';
}

// Leer un "algo" genérico
void leer_generic(t_args *args, uint32_t direccion_fisica, void *estructura, size_t tamano_estructura)
{
    espacio_usuario_leer(args, direccion_fisica, estructura, tamano_estructura);
}