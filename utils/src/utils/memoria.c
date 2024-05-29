#include "memoria.h"

void memoria_hilo_ejecutar(t_args *argumentos, int socket, char *modulo, t_mem_funcion_ptr switch_case_atencion)
{
    while (1)
    {
        pthread_testcancel();

        log_debug(argumentos->logger, "Esperando paquete de Kernel en socket %d", socket);

        t_paquete *paquete = recibir_paquete(argumentos->logger, &socket);

        if (paquete == NULL)
        {
            log_warning(argumentos->logger, "%s se desconecto del socket %d.", modulo, socket);
            break;
        }

        revisar_paquete(paquete, argumentos->logger, modulo);

        switch_case_atencion(argumentos, paquete->codigo_operacion, paquete->buffer);

        eliminar_paquete(paquete);
    }
}

t_entrada_salida *agregar_entrada_salida(t_args *argumentos, t_tipo_entrada_salida type, int socket)
{
    t_entrada_salida *entrada_salida = NULL;
    switch (type)
    {
    case STDIN:
        entrada_salida = agregar_interfaz(argumentos, STDIN, socket);
        break;
    case STDOUT:
        entrada_salida = agregar_interfaz(argumentos, STDOUT, socket);
        break;
    case DIALFS:
        entrada_salida = agregar_interfaz(argumentos, DIALFS, socket);
        break;
    default:
        log_error(argumentos->logger, "Tipo de socket de entrada/salida no reconocido");
        break;
    }

    if (entrada_salida == NULL)
    {
        log_error(argumentos->logger, "Error al agregar el socket de entrada/salida");
        return NULL;
    }

    return entrada_salida;
};

// t_entrada_salida *agregar_interfaz(t_args *argumentos, t_tipo_entrada_salida tipo, int socket)
// {
//     // Asigno memoria para el socket de entrada/salida (no debo liberarla porque se guarda dentro de la lista la referencia)
//     t_entrada_salida *entrada_salida = malloc(sizeof(t_entrada_salida));

//     // Guardo el socket en el cual se conecto el modulo de entrada/salida
//     entrada_salida->socket = socket;
//     entrada_salida->tipo = tipo;
//     entrada_salida->orden = argumentos->memoria.sockets.id_entrada_salida;
//     entrada_salida->ocupado = 0;
//     entrada_salida->pid = 0;

//     // Calculo el tamaño que necesito para almacenar el identificador de la interfaz
//     int size_necesario = snprintf(NULL, 0, "Int%d", argumentos->memoria.sockets.id_entrada_salida) + 1;

//     // Reservo memoria para la interfaz (no debo liberarla porque se guarda dentro del diccionario)
//     char *interfaz = malloc(size_necesario);

//     // Imprimo sobre la variable interfaz el identificador de la interfaz
//     sprintf(interfaz, "Int%d", argumentos->memoria.sockets.id_entrada_salida);

//     // Duplico la cadena para guardarla en el TAD y poder identificar la IO (esto pide malloc y hay que liberarlo cuando se desconecta la IO)
//     entrada_salida->interfaz = strdup(interfaz);

//     int *index = malloc(sizeof(int));

//     // Agrego el TAD a la lista de entrada/salida y guardo el indice en el que se encuentra
//     *index = list_add(argumentos->memoria.lista_entrada_salida, entrada_salida);

//     // Guardo en el diccionario la key interfaz y el value indice para ubicarlo en la lista luego
//     dictionary_put(argumentos->memoria.diccionario_entrada_salida, interfaz, index);

//     log_debug(argumentos->logger, "Se conecto un modulo de entrada/salida en el socket %d con la interfaz %s", socket, interfaz);

//     argumentos->memoria.sockets.id_entrada_salida++;

//     return entrada_salida;
// }

void agregar_identificador(t_args_hilo *argumentos, char *identificador)
{
    // Duplico la cadena para guardarla en el TAD y poder identificar la IO (esto pide malloc y hay que liberarlo cuando se desconecta la IO)
    argumentos->entrada_salida->interfaz = strdup(identificador);
    argumentos->entrada_salida->identificado = true;

    int *index = malloc(sizeof(int));

    // Agrego el TAD a la lista de entrada/salida y guardo el indice en el que se encuentra
    *index = list_add(argumentos->argumentos->memoria.lista_entrada_salida, argumentos->entrada_salida);

    // Guardo en el diccionario la key socket y el value indice para ubicarlo en la lista luego
    dictionary_put(argumentos->argumentos->memoria.diccionario_entrada_salida, strdup(identificador), index);

    log_debug(argumentos->argumentos->logger, "Se conecto un modulo de entrada/salida en el socket %d", argumentos->entrada_salida->socket);
}

t_entrada_salida *agregar_interfaz(t_args *args, t_tipo_entrada_salida tipo, int socket)
{
    // Asigno memoria para el socket de entrada/salida (no debo liberarla porque se guarda dentro de la lista la referencia)
    t_entrada_salida *entrada_salida = malloc(sizeof(t_entrada_salida));

    // Guardo el socket en el cual se conecto el modulo de entrada/salida
    entrada_salida->socket = socket;
    entrada_salida->tipo = tipo;
    entrada_salida->orden = args->memoria.sockets.id_entrada_salida;
    entrada_salida->ocupado = 0;
    entrada_salida->pid = 0;
    entrada_salida->identificado = false;

    log_debug(args->logger, "Se conecto un modulo de entrada/salida en el socket %d", socket);

    args->memoria.sockets.id_entrada_salida++;
    return entrada_salida;
}

void remover_interfaz(t_args *argumentos, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(argumentos->memoria.diccionario_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede desconectar
    if (indice == NULL)
    {
        log_debug(argumentos->logger, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_entrada_salida *entrada_salida = list_get(argumentos->memoria.lista_entrada_salida, *indice);

    log_debug(argumentos->logger, "Se remueve interfaz de entrada/salida %s", interfaz);

    // Cierro el socket de la entrada/salida del lado de Kernel
    liberar_conexion(&entrada_salida->socket);

    // Elimino la interfaz del diccionario
    dictionary_remove(argumentos->memoria.diccionario_entrada_salida, interfaz);

    // Libero la memoria de la interfaz
    free(entrada_salida->interfaz);

    // Libero la memoria del TAD
    free(entrada_salida);

    // Libero el indice guardado en el diccionario
    free(indice);

    // No lo elimino de la lista porque no se puede hacer un list_remove sin reorganizar los indices. Lo dejo en la lista pero no se puede acceder a el porque está vacio. Al finalizar el programa, destruyo la estructura de la lista entera.
}

t_entrada_salida *buscar_interfaz(t_args *argumentos, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(argumentos->memoria.diccionario_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede buscar
    if (indice == NULL)
    {
        log_error(argumentos->logger, "No se encontro la interfaz %s en el diccionario de entrada/salida", interfaz);
        return NULL;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_entrada_salida *entrada_salida = list_get(argumentos->memoria.lista_entrada_salida, *indice);

    if (entrada_salida == NULL)
    {
        log_error(argumentos->logger, "No se encontro la interfaz %s en la lista de entrada/salida", interfaz);
    }

    log_debug(argumentos->logger, "Se encontro el modulo de entrada/salida en el socket %d asociado a la interfaz %s", entrada_salida->socket, interfaz);

    return entrada_salida;
}

void eliminar_procesos(t_args *argumentos)
{
    // Cuento las keys del diccionario de procesos
    int cantidad_procesos = dictionary_size(argumentos->memoria.diccionario_procesos);

    if (cantidad_procesos != 0)
    {
        // Obtengo las keys del diccionario de procesos
        t_list *lista_procesos_actuales = dictionary_keys(argumentos->memoria.diccionario_procesos);

        // Recorro la lista de keys
        for (int i = 0; i < cantidad_procesos; i++)
        {
            // Obtengo la key
            char *pid = list_get(lista_procesos_actuales, i);
            char *pid_copia = strdup(pid);

            // Busco el proceso en la lista de procesos
            t_proceso *proceso = list_get(argumentos->memoria.lista_procesos, atoi(dictionary_get(argumentos->memoria.diccionario_procesos, pid)));

            // Si el proceso no se encuentra en la lista de procesos, no se puede eliminar, aunque no debería de pasar en esta instancia. Igualmente, acá esta el caso borde.
            if (proceso == NULL)
            {
                log_error(argumentos->logger, "No se encontro el proceso con PID <%s> en la lista de procesos", pid);
                continue;
            }

            // Si el proceso no tiene instrucciones, no se puede eliminar. Otro caso borde más.
            if (list_is_empty(proceso->instrucciones))
            {
                log_error(argumentos->logger, "No se encontraron instrucciones para el proceso con PID <%s>", pid);
                continue;
            }

            // Elimino las instrucciones del proceso
            eliminar_instrucciones(argumentos, proceso->instrucciones);

            // Elimino el proceso de la lista de procesos
            // list_remove(argumentos->memoria.lista_procesos, atoi(dictionary_get(argumentos->memoria.diccionario_procesos, pid)));

            // Libero los recursos del proceso, pero no lo saco de la lista para no cambiar los indices.
            free(proceso);

            // Elimino el proceso del diccionario de procesos
            dictionary_remove(argumentos->memoria.diccionario_procesos, pid);

            log_debug(argumentos->logger, "Se elimino el proceso con PID <%s> de Memoria.", pid_copia);
            free(pid_copia);
        }
    }

    // Libero la memoria de la lista de procesos
    list_destroy(argumentos->memoria.lista_procesos);

    // Libero la memoria del diccionario de procesos
    dictionary_destroy(argumentos->memoria.diccionario_procesos);
}

void eliminar_instrucciones(t_args *argumentos, t_list *lista_instrucciones)
{

    if (!list_is_empty(lista_instrucciones))
    {
        for (int i = 0; i < list_size(lista_instrucciones); i++)
        {
            t_memoria_cpu_instruccion *instruccion = list_get(lista_instrucciones, i);
            for (int j = 0; j < instruccion->cantidad_elementos; j++)
            {
                if (instruccion->array[j] != NULL)
                {
                    log_debug(argumentos->logger, "Liberando instruccion en posicion %d: %s", j, instruccion->array[j]);
                    free(instruccion->array[j]);
                }
            }
            free(instruccion->array);
            free(instruccion);
        }
        list_destroy(lista_instrucciones);
    }
}

char *armar_ruta(char *ruta1, char *ruta2)
{
    char *ruta = malloc(strlen(ruta1) + strlen(ruta2) + 2);
    strcpy(ruta, ruta1);
    strcat(ruta, "/");
    strcat(ruta, ruta2);
    return ruta;
}

t_list *leer_instrucciones(t_args *argumentos, char *path_instrucciones, uint32_t pid)
{
    if (path_instrucciones == NULL)
    {
        log_error(argumentos->logger, "Path de instrucciones nulo. No se puede leer el archivo");
        return NULL;
    }

    // Abro el archivo de instrucciones
    FILE *file = fopen(path_instrucciones, "r");

    if (file == NULL)
    {
        log_error(argumentos->logger, "No se pudo abrir el archivo de instrucciones <%s>", path_instrucciones);
        return NULL;
    }

    // Inicializo la lista de instrucciones
    t_list *lista = list_create();

    // Inicializa a NULL para que getline asigne memoria automáticamente
    char *line = NULL;

    // Inicializa a 0 para que getline asigne memoria automáticamente
    size_t len = 0;

    // ssize_t es un tipo de dato que se usa para representar el tamaño de bloques de memoria
    ssize_t read;

    log_debug(argumentos->logger, "Leyendo archivo de instrucciones...");

    // Getline lee la linea entera, hasta el \n inclusive
    while ((read = getline(&line, &len, file)) != -1)
    {
        log_debug(argumentos->logger, "Linea leida: %s", line);

        t_memoria_cpu_instruccion *instruccion = malloc(sizeof(t_memoria_cpu_instruccion));

        if (instruccion == NULL)
        {
            log_error(argumentos->logger, "No se pudo reservar memoria para la instruccion");
            fclose(file);
            free(line);
        }

        // Separo la línea en palabras
        char **separar_linea = string_split(line, " ");

        if (separar_linea[0] == NULL)
        {
            log_error(argumentos->logger, "Instruccion vacia. Cierro el archivo y libero memoria");
            fclose(file);
            free(line);
            free(instruccion);
            return NULL;
        }

        // Cuento la cantidad de elementos en separar_linea
        uint32_t cantidad_elementos = 0;

        while (separar_linea[cantidad_elementos] != NULL)
        {
            cantidad_elementos++;
        }

        // Asigno memoria para el array de punteros en instruccion->array
        instruccion->array = malloc(cantidad_elementos * sizeof(char *));

        if (instruccion->array == NULL)
        {
            log_error(argumentos->logger, "No se pudo reservar memoria para instruccion->array");
            fclose(file);
            free(line);
            free(instruccion);
            return NULL;
        }

        instruccion->cantidad_elementos = cantidad_elementos;
        instruccion->pid = pid;

        for (int i = 0; i < cantidad_elementos; i++)
        {
            instruccion->array[i] = strdup(separar_linea[i]);
            log_debug(argumentos->logger, "Posicion %d: %s", i, instruccion->array[i]);
        }

        // Agrego la instruccion a la lista
        list_add(lista, instruccion);

        // Libero la memoria de la línea separada
        for (int i = 0; i < cantidad_elementos; i++)
        {
            free(separar_linea[i]);
        }
        free(separar_linea);
    }
    // Cierro el archivo
    fclose(file);

    free(line);

    log_debug(argumentos->logger, "Tamaño de la lista de instrucciones: %d", list_size(lista));

    return lista;
}

t_proceso *buscar_proceso(t_args *argumentos, uint32_t pid)
{
    char *pid_char = string_itoa(pid);
    char *indice = dictionary_get(argumentos->memoria.diccionario_procesos, pid_char);

    if (indice == NULL)
    {
        log_error(argumentos->logger, "No se encontro el proceso con PID <%s>", pid_char);
        return NULL;
    }

    free(pid_char);

    return list_get(argumentos->memoria.lista_procesos, atoi(indice));
}

void memoria_inicializar_config(t_args *argumentos)
{
    argumentos->memoria.puertoEscucha = config_get_int_value(argumentos->memoria.config, "PUERTO_ESCUCHA");
    argumentos->memoria.tamMemoria = config_get_int_value(argumentos->memoria.config, "TAM_MEMORIA");
    argumentos->memoria.tamPagina = config_get_int_value(argumentos->memoria.config, "TAM_PAGINA");
    argumentos->memoria.pathInstrucciones = config_get_string_value(argumentos->memoria.config, "PATH_INSTRUCCIONES");
    argumentos->memoria.retardoRespuesta = config_get_int_value(argumentos->memoria.config, "RETARDO_RESPUESTA");
}

void memoria_inicializar_argumentos(t_args *argumentos)
{
    argumentos->memoria.lista_procesos = list_create();
    argumentos->memoria.diccionario_procesos = dictionary_create();
    argumentos->memoria.diccionario_entrada_salida = dictionary_create();
    argumentos->memoria.lista_entrada_salida = list_create();
    argumentos->memoria.sockets.id_entrada_salida = 1;
    argumentos->memoria.sockets.socket_server_memoria = iniciar_servidor(argumentos->logger, argumentos->memoria.puertoEscucha);
}

void memoria_inicializar_hilos(t_args *argumentos)
{

    pthread_create(&argumentos->memoria.threads.thread_esperar_cpu, NULL, esperar_cpu, (void *)argumentos);
    pthread_join(argumentos->memoria.threads.thread_esperar_cpu, NULL);

    pthread_create(&argumentos->memoria.threads.thread_atender_cpu, NULL, atender_cpu, (void *)argumentos);
    pthread_detach(argumentos->memoria.threads.thread_atender_cpu);

    pthread_create(&argumentos->memoria.threads.thread_conectar_kernel, NULL, esperar_kernel, (void *)argumentos);
    pthread_join(argumentos->memoria.threads.thread_conectar_kernel, NULL);

    pthread_create(&argumentos->memoria.threads.thread_atender_entrada_salida, NULL, esperar_entrada_salida, (void *)argumentos);
    pthread_detach(argumentos->memoria.threads.thread_atender_entrada_salida);

    pthread_create(&argumentos->memoria.threads.thread_atender_kernel, NULL, atender_kernel, (void *)argumentos);
    pthread_join(argumentos->memoria.threads.thread_atender_kernel, NULL);
}

void memoria_inicializar(t_args *argumentos)
{
    memoria_inicializar_config(argumentos);
    memoria_inicializar_argumentos(argumentos);
    memoria_imprimir_config(argumentos);
    memoria_inicializar_hilos(argumentos);
}

void inicializar_modulo(t_args *argumentos)
{
    memoria_inicializar(argumentos);
    memoria_finalizar(argumentos);
}

void memoria_finalizar(t_args *argumentos)
{
    eliminar_procesos(argumentos);
    config_destroy(argumentos->memoria.config);
    log_destroy(argumentos->logger);
}

void memoria_imprimir_config(t_args *argumentos)
{
    log_info(argumentos->logger, "PUERTO_ESCUCHA: %d", argumentos->memoria.puertoEscucha);
    log_info(argumentos->logger, "TAM_MEMORIA: %d", argumentos->memoria.tamMemoria);
    log_info(argumentos->logger, "TAM_PAGINA: %d", argumentos->memoria.tamPagina);
    log_info(argumentos->logger, "PATH_INSTRUCCIONES: %s", argumentos->memoria.pathInstrucciones);
    log_info(argumentos->logger, "RETARDO_RESPUESTA: %d", argumentos->memoria.retardoRespuesta);
}

void *esperar_entrada_salida(void *paquete)
{
    t_args *argumentos = (t_args *)paquete;

    while (1)
    {
        int socket_cliente = conexion_socket_recibir(argumentos->memoria.sockets.socket_server_memoria);

        if (socket_cliente == -1)
        {
            log_error(argumentos->logger, "Error al esperar conexion de modulo de Entrada/Salida");
            break;
        }

        t_handshake modulo = esperar_handshake_entrada_salida(argumentos->logger, socket_cliente);

        if (modulo == ERROR)
        {
            log_error(argumentos->logger, "Error al recibir handshake de modulo de Entrada/Salida");
            liberar_conexion(&socket_cliente);
            break;
        }

        log_debug(argumentos->logger, "Conexion por handshake recibida y estrablecido con modulo de Entrada/Salida en socket %d", socket_cliente);

        pthread_t thread_atender_entrada_salida;
        t_args_hilo *hilo_args = malloc(sizeof(t_args_hilo));
        hilo_args->argumentos = argumentos;
        hilo_args->entrada_salida = NULL;

        switch (modulo)
        {
        case MEMORIA_ENTRADA_SALIDA_STDIN:
            hilo_args->entrada_salida = agregar_entrada_salida(argumentos, STDIN, socket_cliente);

            pthread_create(&thread_atender_entrada_salida, NULL, atender_entrada_salida_stdin, hilo_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        case MEMORIA_ENTRADA_SALIDA_STDOUT:
            hilo_args->entrada_salida = agregar_entrada_salida(argumentos, STDOUT, socket_cliente);

            pthread_create(&thread_atender_entrada_salida, NULL, atender_entrada_salida_stdout, hilo_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        case MEMORIA_ENTRADA_SALIDA_DIALFS:
            hilo_args->entrada_salida = agregar_entrada_salida(argumentos, DIALFS, socket_cliente);

            pthread_create(&thread_atender_entrada_salida, NULL, atender_entrada_salida_dialfs, hilo_args);
            pthread_detach(thread_atender_entrada_salida);
            break;
        default:
            log_debug(argumentos->logger, "Se conecto un modulo de entrada/salida desconocido. Cerrando...");
            liberar_conexion(&socket_cliente);
            break;
        }
    }

    pthread_exit(0);
}

void *atender_entrada_salida_dialfs(void *argumentos)
{
    t_args_hilo *io_args = (t_args_hilo *)argumentos;
    char *modulo = "DialFS";
    memoria_hilo_ejecutar_entrada_salida(io_args, modulo, switch_case_entrada_salida_dialfs);
    pthread_exit(0);
}

void *atender_kernel(void *paquete)
{
    t_args *argumentos = (t_args *)paquete;
    memoria_hilo_ejecutar(argumentos, argumentos->memoria.sockets.socket_kernel, "Kernel", switch_case_kernel);
    pthread_exit(0);
}

void *esperar_kernel(void *paquete)
{
    t_args *argumentos = (t_args *)paquete;
    conexion_recibir(argumentos->logger, argumentos->memoria.sockets.socket_server_memoria, &argumentos->memoria.sockets.socket_kernel, "Kernel", MEMORIA_KERNEL);
    pthread_exit(0);
}

void *atender_entrada_salida_stdout(void *argumentos)
{
    t_args_hilo *io_args = (t_args_hilo *)argumentos;
    char *modulo = "STDOUT";
    memoria_hilo_ejecutar_entrada_salida(io_args, modulo, switch_case_entrada_salida_stdout);
    pthread_exit(0);
}

void *atender_entrada_salida_stdin(void *argumentos)
{
    t_args_hilo *io_args = (t_args_hilo *)argumentos;
    char *modulo = "STDIN";
    memoria_hilo_ejecutar_entrada_salida(io_args, modulo, switch_case_entrada_salida_stdin);
    pthread_exit(0);
}

void *esperar_cpu(void *paquete)
{
    t_args *argumentos = (t_args *)paquete;
    conexion_recibir(argumentos->logger, argumentos->memoria.sockets.socket_server_memoria, &argumentos->memoria.sockets.socket_cpu, "CPU", MEMORIA_CPU);
    pthread_exit(0);
}

void *atender_cpu(void *paquete)
{
    t_args *argumentos = (t_args *)paquete;
    memoria_hilo_ejecutar(argumentos, argumentos->memoria.sockets.socket_cpu, "CPU", switch_case_cpu);
    pthread_exit(0);
}

void memoria_hilo_ejecutar_entrada_salida(t_args_hilo *io_args, char *modulo, t_mem_funcion_hilo_ptr switch_case_atencion)
{
    while (1)
    {

        if (io_args->entrada_salida->identificado)
        {
            log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Esperando paquete en socket %d", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden, io_args->entrada_salida->socket);
        }
        else
        {
            log_debug(io_args->argumentos->logger, "[%s/Orden %d] Esperando identificador en socket %d", modulo, io_args->entrada_salida->orden, io_args->entrada_salida->socket);
        }

        t_paquete *paquete = recibir_paquete(io_args->argumentos->logger, &io_args->entrada_salida->socket);

        if (paquete == NULL)
        {

            if (io_args->entrada_salida->identificado)
            {
                log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Se desconecto", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden);
            }
            else
            {
                log_debug(io_args->argumentos->logger, "[%s/Orden %d] Se desconecto", modulo, io_args->entrada_salida->orden);
            }

            log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Desconectado.", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden);
            break;
        }
        revisar_paquete(paquete, io_args->argumentos->logger, modulo);

        switch_case_atencion(io_args, paquete->codigo_operacion, paquete->buffer);
    }

    log_debug(io_args->argumentos->logger, "[%s/Interfaz %s/Orden %d] Cerrando hilo", modulo, io_args->entrada_salida->interfaz, io_args->entrada_salida->orden);

    remover_interfaz(io_args->argumentos, io_args->entrada_salida->interfaz);

    free(io_args);
}

void inicializar_logger(t_args *argumentos, t_log_level nivel)
{
    argumentos->logger = iniciar_logger("memoria", nivel);
}

void inicializar(t_args *argumentos, t_log_level nivel, int argc, char *argv[])
{
    inicializar_logger(argumentos, nivel);
    inicializar_config(&argumentos->memoria.config, argumentos->logger, argc, argv);
    inicializar_modulo(argumentos);
}