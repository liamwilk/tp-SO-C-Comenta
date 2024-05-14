#include "template.h"

void hilo_ejecutar(t_log *logger, int socket, char *modulo, t_funcion_ptr switch_case_atencion)
{
    while (1)
    {
        pthread_testcancel();

        log_debug(logger, "Esperando paquete de Kernel en socket %d", socket);

        t_paquete *paquete = recibir_paquete(logger, socket);

        if (paquete == NULL)
        {
            log_warning(logger, "%s se desconecto del socket %d.", modulo, socket);
            break;
        }

        revisar_paquete(paquete, logger, modulo);

        switch_case_atencion(logger, paquete->codigo_operacion, paquete->buffer);

        eliminar_paquete(paquete);
    }
}

void hilo_ejecutar_kernel(int socket, hilos_args *args, char *modulo, t_funcion_kernel_ptr switch_case_atencion)
{
    while (1)
    {
        imprimir_log(args, LOG_LEVEL_DEBUG, "Esperando paquete de Kernel en socket %d", socket);

        t_paquete *paquete = recibir_paquete(args->logger, socket);

        if (paquete == NULL)
        {
            imprimir_log(args, LOG_LEVEL_WARNING, "%s se deconecto del socket %d", modulo, socket);
            break;
        }

        revisar_paquete(paquete, args->logger, modulo);

        switch_case_atencion(args->logger, paquete->codigo_operacion, args, paquete->buffer);

        eliminar_paquete(paquete);
    }
    imprimir_log(args, LOG_LEVEL_DEBUG, "Finalizando hilo de atencion a %s", modulo);
    usleep(500);
    fflush(stdout);
    sem_post(&args->kernel->sistema_finalizar);
}

void conexion_crear(t_log *logger, char *ip, int puerto, int *socket_modulo, char *modulo, t_handshake codigo_esperado)
{
    *socket_modulo = crear_conexion(ip, puerto);

    if (*socket_modulo == -1)
    {
        log_error(logger, "Error al crear conexion con %s", modulo);
        pthread_exit(0);
    }

    t_handshake resultado = crear_handshake(logger, *socket_modulo, codigo_esperado, modulo);

    if (resultado != CORRECTO)
    {
        log_error(logger, "Error al crear handshake con %s", modulo);
        liberar_conexion(socket_modulo);
        pthread_exit(0);
    }

    log_debug(logger, "Conectado a %s en socket %d", modulo, *socket_modulo);
}

void conexion_recibir(t_log *logger, int socket_servidor, int *socket_modulo, char *modulo, t_handshake codigo_esperado)
{

    *socket_modulo = conexion_socket_recibir(socket_servidor);

    if (*socket_modulo == -1)
    {
        log_error(logger, "Error al recibir conexion de %s", modulo);
        pthread_exit(0);
    }

    t_handshake resultado = conexion_handshake_recibir(logger, *socket_modulo, codigo_esperado, modulo);

    if (resultado != CORRECTO)
    {
        log_error(logger, "Error al recibir handshake de %s", modulo);
        liberar_conexion(socket_modulo);
        pthread_exit(0);
    }

    log_debug(logger, "%s conectado en socket %d", modulo, *socket_modulo);
}

void imprimir_log_simple(sem_t *sem, t_log *logger, t_log_level nivel, const char *mensaje)
{
    sem_wait(sem);

    // Guardar el estado actual de readline
    int saved_point = rl_point;
    char *saved_line = rl_copy_text(0, rl_end);

    // Limpiar la línea actual
    rl_save_prompt();
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();

    // Imprimir el mensaje de log usando la función de log correspondiente
    switch (nivel)
    {
    case LOG_LEVEL_TRACE:
        log_trace(logger, "%s", mensaje);
        break;
    case LOG_LEVEL_DEBUG:
        log_debug(logger, "%s", mensaje);
        break;
    case LOG_LEVEL_INFO:
        log_info(logger, "%s", mensaje);
        break;
    case LOG_LEVEL_WARNING:
        log_warning(logger, "%s", mensaje);
        break;
    case LOG_LEVEL_ERROR:
        log_error(logger, "%s", mensaje);
        break;
    default:
        log_error(logger, "Nivel de log inválido en imprimir_log_simple");
        break;
    }

    // Restaurar el estado de readline
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_restore_prompt();
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    rl_redisplay();

    sem_post(sem);
    free(saved_line);
}

void imprimir_log(hilos_args *args, t_log_level nivel, const char *mensaje, ...)
{
    va_list args_list;
    va_start(args_list, mensaje);

    // Calcular el tamaño necesario para el mensaje formateado
    int needed_size = vsnprintf(NULL, 0, mensaje, args_list) + 1;

    // Crear un buffer para el mensaje formateado
    char *mensaje_formateado = malloc(needed_size);

    if (mensaje_formateado != NULL)
    {
        // Reiniciar 'args_list' para usarla de nuevo
        va_end(args_list);
        va_start(args_list, mensaje);

        // Formatear el mensaje
        vsnprintf(mensaje_formateado, needed_size, mensaje, args_list);

        // Llamar a la función que imprime el mensaje simple
        imprimir_log_simple(&args->kernel->log_lock, args->logger, nivel, mensaje_formateado);

        // Liberar la memoria del mensaje formateado
        free(mensaje_formateado);
    }
    else
    {
        log_error(args->logger, "Error al reservar memoria para imprimir log");
    }

    va_end(args_list);
}
