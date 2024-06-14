#include "consola.h"

void kernel_log_generic(hilos_args *args, t_log_level nivel, const char *mensaje, ...)
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
        kernel_log_generic_rl(args, nivel, mensaje_formateado);

        // Liberar la memoria del mensaje formateado
        free(mensaje_formateado);
    }
    else
    {
        log_error(args->logger, "Error al reservar memoria para imprimir log");
    }

    va_end(args_list);
}

void kernel_log_generic_rl(hilos_args *args, t_log_level nivel, const char *mensaje)
{
    sem_wait(&args->kernel->log_lock);

    pthread_mutex_lock(&args->kernel->lock);

    // Guardar el estado actual de readline
    int saved_point = rl_point;
    char *saved_line = rl_copy_text(0, rl_end);

    // Limpiar la línea actual
    rl_save_prompt();
    rl_set_prompt("");
    rl_replace_line("", 0);

    pthread_mutex_unlock(&args->kernel->lock);

    // Muestro devuelta el prompt de readline
    rl_redisplay();

    // Imprimir el mensaje de log usando la función de log correspondiente
    switch (nivel)
    {
    case LOG_LEVEL_TRACE:
        log_trace(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_DEBUG:
        log_debug(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_INFO:
        log_info(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_WARNING:
        log_warning(args->logger, "%s", mensaje);
        break;
    case LOG_LEVEL_ERROR:
        log_error(args->logger, "%s", mensaje);
        break;
    default:
        log_error(args->logger, "Nivel de log inválido en kernel_log_generic_rl");
        break;
    }

    pthread_mutex_lock(&args->kernel->lock);

    // Restaurar el estado de readline
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_restore_prompt();
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;

    pthread_mutex_unlock(&args->kernel->lock);

    // Vuelvo a mostrar el prompt de readline
    rl_redisplay();

    // Libero el semaforo de log
    sem_post(&args->kernel->log_lock);
    free(saved_line);
}

char *generar_prompt()
{
    // Obtengo el nombre de usuario desde la variable de entorno "USER".
    const char *username = getenv("USER");
    if (username == NULL)
    {
        // Si "USER" no está definida, obtengo el nombre de usuario a partir del UID.
        struct passwd *pw = getpwuid(getuid());
        username = pw->pw_name;
    }

    // Obtengo el nombre del host
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    // Obtengo el directorio de trabajo actual (cwd)
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        // Manejo el error si falló la obtención del cwd.
        perror("getcwd() error");
        return NULL;
    }

    // Obtengo el directorio home del usuario.
    const char *home_dir = getenv("HOME");
    char primera_letra = '\0';

    // Si el cwd comienza con el home_dir, reemplazar el home_dir con "~".
    if (home_dir != NULL && strstr(cwd, home_dir) == cwd)
    {
        size_t home_len = strlen(home_dir);
        memmove(cwd + 1, cwd + home_len, strlen(cwd) - home_len + 1);
        cwd[0] = '~';

        // Guardar la primera letra del directorio que sigue al home.
        if (cwd[1] != '\0' && cwd[1] != '/')
        {
            primera_letra = cwd[1];
        }
        else if (cwd[2] != '\0')
        {
            char *next_dir = strchr(cwd + 2, '/');
            if (next_dir != NULL)
            {
                primera_letra = cwd[2];
            }
        }
    }

    // Obtengo el ancho de la terminal
    int ancho_terminal = obtener_ancho_terminal();
    if (ancho_terminal == -1)
    {
        // Valor predeterminado en caso de error
        ancho_terminal = 80;
    }
    // Calculo el margen extra basado en un porcentaje del ancho de la terminal
    double margen_porcentaje = 0.50;
    int margen_extra = (int)(ancho_terminal * margen_porcentaje);

    // Calcular la longitud máxima permitida para el prompt
    int prompt_max_length = ancho_terminal - 10 - margen_extra;

    // Ajusto el cwd si es demasiado largo
    if (strlen(cwd) > prompt_max_length)
    {
        char *last_slash = strrchr(cwd, '/');
        if (last_slash != NULL && last_slash != cwd)
        {
            // Muevo el contenido después del último slash a un buffer temporal
            char temp[PATH_MAX / 2];
            strncpy(temp, last_slash + 1, sizeof(temp) - 1);

            // Me aseguro la terminación nula del buffer temporal
            temp[sizeof(temp) - 1] = '\0';

            // Me aseguro que la longitud total no exceda el tamaño del buffer cwd
            size_t max_temp_length = sizeof(cwd) - 4;
            if (strlen(temp) > max_temp_length)
            {
                temp[max_temp_length] = '\0';
            }

            // Construyo la nueva cadena cwd con la primera letra y el contenido del buffer temporal
            snprintf(cwd, sizeof(cwd), "~/%c/%s", primera_letra, temp);
        }
        else
        {
            // Si no hay un slash, construyo una cadena indicando el directorio padre
            snprintf(cwd, sizeof(cwd), "~/../%s", last_slash + 1);
        }
    }

    // Defino los códigos de color para el prompt

    const char *green_bold = "\001\x1b[1;32m\002";
    const char *blue_bold = "\001\x1b[1;34m\002";
    const char *reset = "\001\x1b[0m\002";

    // Reservo memoria para el prompt completo
    char *prompt = malloc(PATH_MAX + HOST_NAME_MAX + 50);
    if (prompt == NULL)
    {
        // Manejo el error si falla la asignación de memoria
        perror("malloc() error");
        return NULL;
    }

    // Construyo el prompt
    snprintf(prompt, PATH_MAX + HOST_NAME_MAX + 50, "%s%s@%s%s:%s%s%s$ ", green_bold, username, hostname, reset, blue_bold, cwd, reset);

    return prompt;
}

char *autocompletado_instruccion(const char *input_text, int state)
{
    // Lista de comandos disponibles para autocompletar
    static char *comandos[] = {
        "EJECUTAR_SCRIPT",
        "INICIAR_PROCESO",
        "FINALIZAR_PROCESO",
        "DETENER_PLANIFICACION",
        "INICIAR_PLANIFICACION",
        "MULTIPROGRAMACION",
        "PROCESO_ESTADO",
        "FINALIZAR",
        NULL};

    static int command_index, input_length;
    char *command;

    // Inicialización en la primera llamada (state == 0)
    if (state == 0)
    {
        command_index = 0;                 // Reinicia el índice de los comandos
        input_length = strlen(input_text); // Longitud del texto ingresado
    }

    // Recorre la lista de comandos buscando coincidencias
    while ((command = comandos[command_index++]))
    {
        // Devuelve el comando si coincide con el texto ingresado
        if (strncmp(command, input_text, input_length) == 0)
        {
            return strdup(command); // Retorna una copia del comando encontrado
        }
    }

    return NULL; // No se encontraron más comandos coincidentes
}

char *autocompletado_argumento(const char *input_text, int state)
{
    static DIR *dir;
    static struct dirent *entry;
    static int input_length;

    // Inicializa en la primera llamada (state == 0)
    if (state == 0)
    {
        if (dir)
        {
            closedir(dir);
        }
        dir = opendir(".");
        input_length = strlen(input_text);
    }

    // Recorre el directorio buscando coincidencias
    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, input_text, input_length) == 0)
        {
            // Retorna una copia del nombre del archivo/directorio encontrado
            return strdup(entry->d_name);
        }
    }

    // No se encontraron más archivos/directorios coincidentes
    closedir(dir);
    dir = NULL;
    return NULL;
}

char **autocompletado(const char *text, int start, int end)
{
    // Determinar si es un comando o un parametro
    if (start == 0)
    {
        // Completando un comando
        rl_completion_append_character = ' ';
        return rl_completion_matches(text, autocompletado_instruccion);
    }
    else
    {
        // Completando un parámetro (archivo/directorio)
        rl_completion_append_character = '\0';
        return rl_completion_matches(text, autocompletado_argumento);
    }
}

t_consola_operacion obtener_operacion(char *funcion)
{
    char *operacionesStrings[] = {
        "PROCESO_ESTADO",
        "EJECUTAR_SCRIPT",
        "INICIAR_PROCESO",
        "MULTIPROGRAMACION",
        "FINALIZAR_PROCESO",
        "FINALIZAR",
        "DETENER_PLANIFICACION",
        "INICIAR_PLANIFICACION",
    };
    for (int i = 0; i < TOPE_ENUM_CONSOLA; i++)
    {
        if (strcmp(operacionesStrings[i], funcion) == 0)
        {
            return i;
        }
    }
    return TOPE_ENUM_CONSOLA;
}

void actualizar_prompt(int signal)
{
    // Borro el prompt anterior
    rl_set_prompt("");
    rl_replace_line("", 0);
    rl_redisplay();

    // Genero el nuevo prompt
    rl_set_prompt(generar_prompt());
    rl_on_new_line();
    rl_redisplay();
}

int obtener_ancho_terminal()
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
    {
        perror("ioctl");
        return -1;
    }
    return w.ws_col;
}

void reiniciar_prompt(hilos_args *hiloArgs)
{
    sem_wait(&hiloArgs->kernel->log_lock);
    pthread_mutex_lock(&hiloArgs->kernel->lock);
    rl_set_prompt("");
    rl_replace_line("", 0);
    pthread_mutex_unlock(&hiloArgs->kernel->lock);
    rl_redisplay();
    sem_post(&hiloArgs->kernel->log_lock);
}

void kernel_log_ready(hilos_args *kernel_hilos_args, bool prioritaria)
{
    char *msg = prioritaria == false ? "Cola Ready : [" : "Cola Ready Mayor Prioridad : [";
    t_list *listaARecorrer = prioritaria == false ? kernel_hilos_args->estados->ready : kernel_hilos_args->estados->ready_mayor_prioridad;
    // Iterate over ready
    for (int i = 0; i < list_size(listaARecorrer); i++)
    {
        t_pcb *pcb = list_get(listaARecorrer, i);
        char *pid = string_itoa(pcb->pid);
        msg = string_from_format("%s %s", msg, pid);
    }
    msg = string_from_format("%s ]", msg);
    kernel_log_generic(kernel_hilos_args, LOG_LEVEL_INFO, "%s", msg);
}

void imprimir_logo(hilos_args *args)
{

    kernel_log_generic(args, LOG_LEVEL_INFO, "              _                 _ _ _   _____ _____");
    kernel_log_generic(args, LOG_LEVEL_INFO, "             | |               | (_) | |  _  /  ___|");
    kernel_log_generic(args, LOG_LEVEL_INFO, " _ __ ___  __| | ___  _ __   __| |_| |_| | | \\ `--. ");
    kernel_log_generic(args, LOG_LEVEL_INFO, "| '__/ _ \\/ _` |/ _ \\| '_ \\ / _` | | __| | | |`--. \\");
    kernel_log_generic(args, LOG_LEVEL_INFO, "| | |  __/ (_| | (_) | | | | (_| | | |_\\ \\_/ /\\__/ /");
    kernel_log_generic(args, LOG_LEVEL_INFO, "|_|  \\___|\\__,_|\\___/|_| |_|\\__,_|_|\\__|\\___/\\____/");
    kernel_log_generic(args, LOG_LEVEL_INFO, " ");
    kernel_log_generic(args, LOG_LEVEL_INFO, "---------------------------------------------------------------");
    kernel_log_generic(args, LOG_LEVEL_INFO, "Implementación de C-Comenta - www.faq.utnso.com.ar/tp-c-comenta");
    kernel_log_generic(args, LOG_LEVEL_INFO, "Sistemas Operativos - 1C 2024 - UTN FRBA");
    kernel_log_generic(args, LOG_LEVEL_INFO, "---------------------------------------------------------------");
    kernel_log_generic(args, LOG_LEVEL_INFO, " ");
}

void imprimir_comandos(hilos_args *args)
{
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ EJECUTAR_SCRIPT <path>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ INICIAR_PROCESO <path>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ FINALIZAR_PROCESO <PID>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ DETENER_PLANIFICACION");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ INICIAR_PLANIFICACION");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ MULTIPROGRAMACION <grado>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ PROCESO_ESTADO <PID>");
    kernel_log_generic(args, LOG_LEVEL_INFO, "└─ FINALIZAR");
}

void imprimir_header(hilos_args *args)
{
    imprimir_logo(args);
    kernel_log_generic(args, LOG_LEVEL_INFO, "Comandos disponibles:");
    imprimir_comandos(args);
}

void kernel_log(hilos_args *args)
{
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_ESCUCHA: %d", args->kernel->puertoEscucha);
    kernel_log_generic(args, LOG_LEVEL_INFO, "IP_MEMORIA: %s", args->kernel->ipMemoria);
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_MEMORIA: %d", args->kernel->puertoMemoria);
    kernel_log_generic(args, LOG_LEVEL_INFO, "IP_CPU: %s", args->kernel->ipCpu);
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_CPU_DISPATCH: %d", args->kernel->puertoCpuDispatch);
    kernel_log_generic(args, LOG_LEVEL_INFO, "PUERTO_CPU_INTERRUPT: %d", args->kernel->puertoCpuInterrupt);
    kernel_log_generic(args, LOG_LEVEL_INFO, "ALGORITMO_PLANIFICACION: %s", args->kernel->algoritmoPlanificador);
    kernel_log_generic(args, LOG_LEVEL_INFO, "QUANTUM: %d", args->kernel->quantum);
    kernel_log_generic(args, LOG_LEVEL_INFO, "RECURSOS: %s", args->kernel->recursos);
    kernel_log_generic(args, LOG_LEVEL_INFO, "INSTANCIAS_RECURSOS: %s", args->kernel->instanciasRecursos);
    kernel_log_generic(args, LOG_LEVEL_INFO, "GRADO_MULTIPROGRAMACION: %d", args->kernel->gradoMultiprogramacion);
}
void kernel_revisar_paquete(t_paquete *paquete, hilos_args *args, char *modulo)
{
    if (paquete->codigo_operacion != FINALIZAR_SISTEMA)
    {
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Paquete recibido de modulo %s", modulo);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Deserializado del paquete:");
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Codigo de operacion: %d", paquete->codigo_operacion);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Size del buffer en paquete: %d", paquete->size_buffer);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Deserializado del buffer:");
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Size del stream: %d", paquete->buffer->size);
        kernel_log_generic(args, LOG_LEVEL_TRACE, "Offset del stream: %d", paquete->buffer->offset);

        if (paquete->size_buffer != paquete->buffer->size + (2 * sizeof(uint32_t)))
        {
            kernel_log_generic(args, LOG_LEVEL_WARNING, "Error en el tamaño del buffer. Se esperaba %d y se recibio %ld", paquete->size_buffer, paquete->buffer->size + (2 * sizeof(uint32_t)));
        }
    }
    else
    {
        kernel_log_generic(args, LOG_LEVEL_DEBUG, "Kernel solicito el apagado del modulo.");
    }
}