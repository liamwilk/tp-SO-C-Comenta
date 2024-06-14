#include "conexiones.h"

int crear_conexion(t_log *logger, char *ip, int puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;
    const int enable = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char *puerto_str = string_itoa(puerto);

    getaddrinfo(ip, puerto_str, &hints, &server_info);

    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (setsockopt(socket_cliente, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        log_error(logger, "Error en setsockopt(SO_REUSEADDR)");
        freeaddrinfo(server_info);
        free(puerto_str);
        close(socket_cliente);
        return -1;
    }

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) < 0)
    {
        log_error(logger, "Error al conectar con el servidor");
        freeaddrinfo(server_info);
        free(puerto_str);
        close(socket_cliente);
        return -1;
    }

    freeaddrinfo(server_info);
    free(puerto_str);

    return socket_cliente;
}

int iniciar_servidor(t_log *logger, int puerto)
{
    char *puerto_str = string_itoa(puerto);
    struct addrinfo hints, *servinfo;
    const int enable = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, puerto_str, &hints, &servinfo);

    int socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    if (socket_servidor == -1)
    {
        log_error(logger, "Error al crear el socket servidor");
        freeaddrinfo(servinfo);
        free(puerto_str);
        return EXIT_FAILURE;
    }

    if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        log_error(logger, "Error en setsockopt(SO_REUSEADDR)");
        freeaddrinfo(servinfo);
        free(puerto_str);
        close(socket_servidor);
        return EXIT_FAILURE;
    }

    if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        log_error(logger, "Error al hacer bind en el socket servidor");
        freeaddrinfo(servinfo);
        free(puerto_str);
        close(socket_servidor);
        return EXIT_FAILURE;
    }

    if (listen(socket_servidor, SOMAXCONN) == -1)
    {
        log_error(logger, "Error al hacer listen en el socket servidor");
        freeaddrinfo(servinfo);
        free(puerto_str);
        close(socket_servidor);
        return EXIT_FAILURE;
    }

    log_debug(logger, "Servidor escuchando en puerto %d", puerto);

    freeaddrinfo(servinfo);
    free(puerto_str);

    return socket_servidor;
}

int conexion_socket_recibir(int socket_servidor)
{
    int socket_cliente = accept(socket_servidor, NULL, NULL);
    return socket_cliente;
}

void liberar_conexion(int *socket_cliente)
{
    if (socket_cliente != NULL && *socket_cliente != -1)
    {
        close(*socket_cliente);
        *socket_cliente = -1;
    }
}