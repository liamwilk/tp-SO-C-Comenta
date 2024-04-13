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
    int socket_cliente = accept(socket_servidor, NULL, NULL);
    if (socket_cliente < 0)
    {
        log_error(logger, "No se pudo conectar: %s", name);
        exit(1);
    }
    log_info(logger, "Se conecto: %s", name);
    return socket_cliente;
}

int crear_conexion(t_log *logger, char *ip, int puerto, char *modulo)
{
	struct addrinfo hints;
	struct addrinfo *server_info;
	char puerto_str[6];

	snprintf(puerto_str, sizeof(puerto_str), "%d", puerto);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto_str, &hints, &server_info);

	int socketCliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	int resultado = connect(socketCliente, server_info->ai_addr, server_info->ai_addrlen);

	// Espero a que se conecte
	while (resultado == -1)
	{
		log_info(logger, "[%s@%s:%d] No se pudo conectar. Reintentando...", modulo, ip, *puerto_str);
		sleep(1);
		resultado = connect(socketCliente, server_info->ai_addr, server_info->ai_addrlen);
	}

	log_info(logger, "[%s@%s:%d] Conectado!", modulo, ip, puerto);
	freeaddrinfo(server_info);

	return socketCliente;
}

void liberar_conexion(int socket_cliente)
{
    close(socket_cliente);
}