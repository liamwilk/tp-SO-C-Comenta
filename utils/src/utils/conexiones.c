#include "conexiones.h"

{

    int socketServidor;

    struct addrinfo hints, *serverinfo;

    // Inicializar hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &serverinfo);

    // Crear socket de escucha del servidor
    socketServidor = socket(serverinfo->ai_family,
                            serverinfo->ai_socktype,
                            serverinfo->ai_protocol);

    // Asociar socket a un puerto
    bind(socketServidor, serverinfo->ai_addr, serverinfo->ai_addrlen);

    // Escuchar conexiones entrantes
    listen(socketServidor, SOMAXCONN);

    freeaddrinfo(serverinfo);
    log_trace(logger, "%s escuchando en %s:%s \n", name, ip, puerto);

    return socketServidor;
}

int esperar_cliente(t_log *logger, const char *name, int socket_servidor)
{
    // TODO
    return 0;
}

int crear_conexion(t_log *logger, const char *server_name, char *ip, char *puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;

    // Se inicializa hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe addrinfo
    getaddrinfo(ip, puerto, &hints, &server_info);

    // Crea un socket con la informacion recibida (del primero, suficiente)
    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    // Caso en el que falle la creación del socket
    if (socket_cliente == -1)
    {
        log_error(logger, "Error creando el socket para %s:%s", ip, puerto);
        return 0;
    }

    // Caso en el que falle la conexión
    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        log_error(logger, "Error al conectar (a %s)\n", server_name);
        freeaddrinfo(server_info);
        return 0;
    }
    else
        log_info(logger, "Cliente conectado en %s:%s (a %s)\n", ip, puerto, server_name);

    freeaddrinfo(server_info); // Se libera memoria

    return socket_cliente;
}

void liberar_conexion(int *socket_cliente)
{
    // TODO
}