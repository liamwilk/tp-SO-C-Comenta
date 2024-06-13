#include <utils/kernel/main.h>

void ejecutar_script(char *path_instrucciones, hilos_args *hiloArgs)
{
    if (path_instrucciones == NULL)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Path de instrucciones nulo. No se puede leer el archivo");
        return;
    }
    char *current_dir = getcwd(NULL, 0);

    char ruta_completa[PATH_MAX];
    sprintf(ruta_completa, "%s/%s", current_dir, path_instrucciones);

    // Abro el archivo de instrucciones
    FILE *file = fopen(ruta_completa, "r");

    if (file == NULL)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "No se pudo abrir el archivo de instrucciones <%s>", ruta_completa);
        return;
    }

    // Inicializa a NULL para que getline asigne memoria automáticamente
    char *line = NULL;

    // Inicializa a 0 para que getline asigne memoria automáticamente
    size_t len = 0;

    // ssize_t es un tipo de dato que se usa para representar el tamaño de bloques de memoria
    ssize_t read;

    kernel_log_generic(hiloArgs, LOG_LEVEL_DEBUG, "Leyendo archivo de scripts...");

    // Getline lee la linea entera, hasta el \n inclusive
    while ((read = getline(&line, &len, file)) != -1)
    {
        // Separo la línea en palabras
        char **separar_linea = string_split(line, " ");

        if (separar_linea[0] == NULL)
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Comando vacio. Cierro el archivo y libero memoria");
            fclose(file);
            free(line);
            return;
        }

        // Cuento la cantidad de elementos en separar_linea
        uint32_t cantidad_elementos = 0;

        while (separar_linea[cantidad_elementos] != NULL)
        {
            cantidad_elementos++;
        }

        // Saco el salto de linea del ultimo elemento
        remover_salto_linea(separar_linea[cantidad_elementos - 1]);

        switch (obtener_operacion(separar_linea[0]))
        {
        case INICIAR_PROCESO:
            iniciar_proceso(separar_linea, hiloArgs);
            break;
        case FINALIZAR_PROCESO:
            finalizar_proceso(separar_linea, hiloArgs);
            break;
        case INICIAR_PLANIFICACION:
            iniciar_planificacion(separar_linea, hiloArgs);
            break;
        case DETENER_PLANIFICACION:
            detener_planificacion(separar_linea, hiloArgs);
            break;
        case MULTIPROGRAMACION:
            multiprogramacion(separar_linea, hiloArgs);
            break;
        case PROCESO_ESTADO:
            procesos_estados(hiloArgs);
            break;
        case FINALIZAR_CONSOLA:
            finalizar_consola(separar_linea, hiloArgs);
            break;
        default:
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Comando no reconocido. Vuelva a intentar");
            break;
        }

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
};
