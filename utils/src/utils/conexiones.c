#include "conexiones.h"

int iniciar_servidor(t_log *logger, const char *name, char *ip, char *puerto)
{

    int socketServidor;

    struct addrinfo hints, *serverinfo, *p;

    // Variable de chequeo
    int check;
    bool conectionSuccess = false;

    // Inicializar hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Seteo serverinfo, y me atajo de posibles errores con gai_strerror
    if ((check = getaddrinfo(ip, puerto, &hints, &serverinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(check));
        freeaddrinfo(serverinfo);
        exit(1);
    }

    // Loopeo los nodos de serverinfo hasta que me pueda bindear a alguno
    for (p = serverinfo; p != NULL; p = p->ai_next)
    {

        // Crear socket de escucha del servidor
        socketServidor = socket(serverinfo->ai_family,
                                serverinfo->ai_socktype,
                                serverinfo->ai_protocol);

        // Si no se pudo conectar exitosamente con socket(), vuelvo a iterar
        if (socketServidor == -1)
        {
            continue;
        }

        // Bindeo del socket al puerto y control de error
        if (bind(socketServidor, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socketServidor);
            log_error(logger, "Error al bindear el socket del servidor");
            continue;
        }

        conectionSuccess = true;
        break;
    }

    // En caso de iterar todos los nodos y no poder conectarse, rompemos
    if (!conectionSuccess)
    {
        freeaddrinfo(serverinfo);
        exit(1);
    }

    // Escuchar conexiones entrantes, maximo 4096
    listen(socketServidor, SOMAXCONN);

    log_trace(logger, "%s escuchando en %s:%s \n", name, ip, puerto);

    freeaddrinfo(serverinfo);

    return socketServidor;
}

int esperar_cliente(t_log *logger, const char *name, int socket_servidor)
{
    int socket_cliente=accept(socket_servidor,NULL,NULL);
    if(socket_cliente <0){
        log_error(logger,"No se pudo conectar: %s",name);
        exit(1);
    }
    log_info(logger, "Se conecto: %s",name);
    return socket_cliente;
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
    close(*socket_cliente);
}