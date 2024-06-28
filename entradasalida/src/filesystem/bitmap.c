#include <utils/entradasalida.h>

void bitmap_desmapear(t_io *args)
{
    if (args->dial_fs.archivo_bitmap != MAP_FAILED && args->dial_fs.archivo_bitmap != NULL)
    {
        if (munmap(args->dial_fs.archivo_bitmap, args->dial_fs.tamanio_bitmap) == -1)
        {
            log_error(args->logger, "Error al desmapear el archivo de 'bitmap.dat': %s", strerror(errno));
        }
        else
        {
            log_debug(args->logger, "Archivo de 'bitmap.dat' desmapeado correctamente de la memoria");
        }

        args->dial_fs.archivo_bitmap = NULL;
    }
    else
    {
        log_warning(args->logger, "Se intento de desmapear un archivo de bitmap que no estaba mapeado");
    }
}

void bitmap_mapear(t_io *args)
{
    struct stat st_bitmap = {0};

    // Construyo el path del bitmap añadiendo bitmap.dat al final
    args->dial_fs.path_bitmap = string_new();
    string_append(&args->dial_fs.path_bitmap, args->dial_fs.pathBaseDialFs);
    string_append(&args->dial_fs.path_bitmap, "/");
    string_append(&args->dial_fs.path_bitmap, args->identificador);

    if (stat(args->dial_fs.path_bloques, &st_bitmap) == -1)
    {
        log_warning(args->logger, "No existe el directorio de bloques, se creará: %s", args->dial_fs.path_bitmap);
        mkdir(args->dial_fs.path_bitmap, 0777);
    }

    string_append(&args->dial_fs.path_bitmap, "/bitmap.dat");

    log_debug(args->logger, "Path de 'bitmap.dat': %s", args->dial_fs.path_bitmap);

    // Calculo el tamaño del archivo bitmap: cada bit representa un bloque, redondeo hacia arriba
    args->dial_fs.tamanio_bitmap = (args->dial_fs.blockCount + 7) / 8;

    // Abro el archivo de bitmap.dat en modo lectura y escritura, lo creo si no existe
    int fd = open(args->dial_fs.path_bitmap, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1)
    {
        log_error(args->logger, "No se pudo abrir o crear el archivo de 'bitmap.dat': %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Verifico si el archivo es nuevo (tamaño 0) para inicializarlo
    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        log_error(args->logger, "No se pudo obtener la información del archivo de 'bitmap.dat': %s", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (st.st_size == 0)
    {
        // Seteo el tamaño del archivo y verifico que se haya podido truncar
        if (ftruncate(fd, args->dial_fs.tamanio_bitmap) == -1)
        {
            log_error(args->logger, "No se pudo truncar el tamaño del archivo de 'bitmap.dat': %s", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Inicializo el bitmap con 0s (todos los bloques libres)
        void *temp_map = mmap(0, args->dial_fs.tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (temp_map == MAP_FAILED)
        {
            log_error(args->logger, "Error al mapear el archivo de 'bitmap.dat' para inicialización: %s", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Inicializo el archivo con 0s
        memset(temp_map, 0, args->dial_fs.tamanio_bitmap);

        if (munmap(temp_map, args->dial_fs.tamanio_bitmap) == -1)
        {
            log_error(args->logger, "Error al desmapear el archivo de 'bitmap.dat' después de la inicialización: %s", strerror(errno));
            close(fd);
            exit(EXIT_FAILURE);
        }

        log_debug(args->logger, "Nuevo archivo de 'bitmap.dat' inicializado correctamente");
    }

    // Mapeo el archivo en memoria
    args->dial_fs.archivo_bitmap = (char *)mmap(0, args->dial_fs.tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    args->dial_fs.bitarray = bitarray_create_with_mode(args->dial_fs.archivo_bitmap, args->dial_fs.tamanio_bitmap, LSB_FIRST);
    if (args->dial_fs.archivo_bitmap == MAP_FAILED)
    {
        log_error(args->logger, "Error al mapear el archivo de 'bitmap.dat': %s", strerror(errno));
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Cierro el descriptor del archivo
    close(fd);

    log_debug(args->logger, "Archivo de 'bitmap.dat' mapeado correctamente a memoria");
}

int bitmap_inicializar(t_io *args)
{
    /* TODO:
    Leer el primer bit del bitmap, si está en 0 -> no está creado el bitmap
    Si está en 1 -> está creado el bitmap -> Leer a partir del próximo bit (n+1)
    */
    return 0;
}
