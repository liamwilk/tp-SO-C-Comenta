#include "conexiones.h"

int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto){

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

int esperar_cliente(t_log* logger, const char* name, int socket_servidor){
    //TODO
    return 0;
}

int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto){
    //TODO
    return 0;
}

void liberar_conexion(int* socket_cliente){
    //TODO
}