#include <utils/entradasalida.h>

void bloques_desmapear(t_io *args)
{
    if (args->dial_fs.archivo_bloques != MAP_FAILED && args->dial_fs.archivo_bloques != NULL)
    {
        if (munmap(args->dial_fs.archivo_bloques, args->dial_fs.tamanio_archivo) == -1)
        {
            log_error(args->logger, "Error al desmapear el archivo de 'bloques.dat': %s", strerror(errno));
        }
        else
        {
            log_debug(args->logger, "Archivo de 'bloques.dat' desmapeado correctamente de la memoria");
        }

        args->dial_fs.archivo_bloques = NULL;
    }
    else
    {
        log_warning(args->logger, "Se intento de desmapear un archivo de bloques que no estaba mapeado");
    }
}

void bloques_mapear(t_io *args)
{
    struct stat st_bloques = {0};

    // Construyo el path de los bloques añadiendo bloques.dat al final
    args->dial_fs.path_bloques = string_new();
    string_append(&args->dial_fs.path_bloques, args->dial_fs.pathBaseDialFs);
    string_append(&args->dial_fs.path_bloques, "/");
    string_append(&args->dial_fs.path_bloques, args->identificador);

    if (stat(args->dial_fs.path_bloques, &st_bloques) == -1)
    {
        log_warning(args->logger, "No existe el directorio de bloques, se creará: %s", args->dial_fs.path_bloques);
        mkdir(args->dial_fs.path_bloques, 0777);
    }

    string_append(&args->dial_fs.path_bloques, "/bloques.dat");

    log_debug(args->logger, "Path de 'bloques.dat': %s", args->dial_fs.path_bloques);

    // Cargo el tamaño del archivo
    args->dial_fs.tamanio_archivo = args->dial_fs.blockSize * args->dial_fs.blockCount;

    // Abro el archivo de bloques.dat en modo lectura y escritura, lo creo si no existe
    int fd = open(args->dial_fs.path_bloques, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

    if (fd == -1)
    {
        if (errno == EEXIST)
        {
            // Archivo ya existe, abrirlo sin O_EXCL
            fd = open(args->dial_fs.path_bloques, O_RDWR, S_IRUSR | S_IWUSR);
            if (fd == -1)
            {
                log_error(args->logger, "No se pudo abrir el archivo de 'bloques.dat': %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            log_error(args->logger, "No se pudo crear el archivo de 'bloques.dat': %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Seteo el tamaño del archivo y verifico que se haya podido truncar
        if (ftruncate(fd, args->dial_fs.tamanio_archivo) == -1)
        {
            log_error(args->logger, "No se pudo truncar el tamaño del archivo de 'bloques.dat': %s", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Inicializo los bloques con '\n'
        void *temp_map = mmap(0, args->dial_fs.tamanio_archivo, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        if (temp_map == MAP_FAILED)
        {
            log_error(args->logger, "Error al mapear el archivo de 'bloques.dat' para inicialización: %s", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Inicializo el archivo con '\n'
        memset(temp_map, '\n', args->dial_fs.tamanio_archivo);

        if (munmap(temp_map, args->dial_fs.tamanio_archivo) == -1)
        {
            log_error(args->logger, "Error al desmapear el archivo de 'bloques.dat' después de la inicialización: %s", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }

        log_debug(args->logger, "Nuevo archivo de 'bloques.dat' inicializado correctamente");
    }

    // Mapeo el archivo en memoria
    args->dial_fs.archivo_bloques = mmap(0, args->dial_fs.tamanio_archivo, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (args->dial_fs.archivo_bloques == MAP_FAILED)
    {
        log_error(args->logger, "Error al mapear el archivo de 'bloques.dat': %s", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Cierro el descriptor del archivo
    close(fd);

    log_debug(args->logger, "Archivo de 'bloques.dat' mapeado correctamente a Memoria");
}