#include "conexiones.h"

int crear_conexion(t_log *logger_error, char *ip, int puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;
    char puerto_str[6];

    // Convierto el puerto a string, para poder usarlo en getaddrinfo
    snprintf(puerto_str, sizeof(puerto_str), "%d", puerto);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto_str, &hints, &server_info);

    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        log_error(logger_error, "No se pudo conectar el socket cliente al servidor puerto %d!", puerto);
    }

    freeaddrinfo(server_info);

    return socket_cliente;
}

int iniciar_servidor(t_log *logger_trace, int puerto)
{
    char puerto_str[6];

    // Convierto el puerto a string, para poder usarlo en getaddrinfo
    snprintf(puerto_str, sizeof(puerto_str), "%d", puerto);

    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto_str, &hints, &servinfo);

    // Creamos el socket de escucha del servidor

    int socket_servidor = socket(servinfo->ai_family,
                                 servinfo->ai_socktype,
                                 servinfo->ai_protocol);

    // Asociamos el socket a un puerto

    bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

    // Escuchamos las conexiones entrantes

    listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(logger_trace, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperar_conexion(t_log *logger_info, int socket_servidor)
{
    int socket_cliente = accept(socket_servidor, NULL, NULL);

    log_debug(logger_info, "Se conecto un modulo en socket %d", socket_cliente);

    return socket_cliente;
}

void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
    socket_cliente = -1;
}