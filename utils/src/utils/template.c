#include "template.h"

void hilo_ejecutar(t_log *logger, int socket, char *modulo, t_funcion_ptr switch_case_atencion)
{
    while (1)
    {
        pthread_testcancel();

        log_debug(logger, "Esperando paquete de Kernel en socket %d", socket);

        t_paquete *paquete = recibir_paquete(logger, &socket);

        if (paquete == NULL)
        {
            log_debug(logger, "%s se desconecto del socket %d.", modulo, socket);
            break;
        }

        revisar_paquete(paquete, logger, modulo);

        switch_case_atencion(logger, paquete->codigo_operacion, paquete->buffer);

        eliminar_paquete(paquete);
    }
}

void conexion_crear(t_log *logger, char *ip, int puerto, int *socket_modulo, char *modulo, t_handshake codigo_esperado)
{
    *socket_modulo = crear_conexion(logger, ip, puerto);

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
