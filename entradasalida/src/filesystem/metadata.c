#include <utils/entradasalida.h>
#include <dirent.h>

typedef struct t_fcb
{
    uint32_t total_size;
    uint32_t inicio;
    uint32_t fin_bloque;
    t_config *metadata; // Esto es para ir guardando los cambios en el archivo de metadata
} t_fcb;

void metadata_inicializar(t_io *args)
{
    log_info(args->logger, "Inicializando metadata");
    // Creando archivo de diccionarios
    args->dial_fs.archivos = dictionary_create();

    char *full_path = string_new();
    string_append(&full_path, args->dial_fs.pathBaseDialFs);
    string_append(&full_path, "/");
    string_append(&full_path, args->identificador);
    // No es necesario comprobar si existe el directorio, ya que previamente lo crearon bloques.dat o bitmap.dat
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(full_path)) == NULL)
    {
        perror("opendir() error");
    }
    else
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, "bloques.dat") != 0 && strcmp(entry->d_name, "bitmap.dat") != 0 && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                char *file_path = string_new();
                string_append(&file_path, full_path);
                string_append(&file_path, "/");
                string_append(&file_path, entry->d_name);
                t_config *nuevo_config = config_create(file_path);

                int tamanio_en_bytes = config_get_int_value(nuevo_config, "TAMANIO_ARCHIVO");
                int bloque_inicial = config_get_int_value(nuevo_config, "BLOQUE_INICIAL");

                t_fcb *nuevo_fcb = malloc(sizeof(t_fcb));
                nuevo_fcb->total_size = tamanio_en_bytes;
                nuevo_fcb->inicio = bloque_inicial;
                nuevo_fcb->fin_bloque = bloque_inicial + (tamanio_en_bytes / args->dial_fs.blockSize);
                nuevo_fcb->metadata = nuevo_config;

                // Voy a verificar si el inicio y el fin de bloque no se encuentra entre otros bloques
                t_list *keys = dictionary_keys(args->dial_fs.archivos);
                for (int i = 0; i < list_size(keys); i++)
                {
                    char *key = list_get(keys, i);
                    t_fcb *fcb = dictionary_get(args->dial_fs.archivos, key);
                    if (strcmp(entry->d_name, key) != 0)
                    {
                        if (bloque_inicial > fcb->inicio && bloque_inicial < fcb->fin_bloque)
                        {
                            log_error(args->logger, "El archivo %s se encuentra entre los bloques de %s", entry->d_name, key);
                            log_error(args->logger, "[DIALFS] Error en el filesystem se estan sobreescribiendo bloques de distintos archivos");
                            free(file_path);
                            free(nuevo_fcb);
                            exit(1);
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
                dictionary_put(args->dial_fs.archivos, entry->d_name, nuevo_fcb);
                log_info(args->logger, "Archivo: %s, Tamaño: %d, Bloque inicial: %d Bloque final: %d", entry->d_name, tamanio_en_bytes, bloque_inicial, nuevo_fcb->fin_bloque);
                free(file_path);
            }
        }
        closedir(dir);
    }
    // Cargo un valor en el archivo de bloques
    // memset(args->dial_fs.archivo_bloques, 'A', args->dial_fs.tamanio_archivo);
    free(full_path);
}

void metadata_modificar_bloque_inicial(t_io *args, char *archivo, int nuevo_bloque_inicial)
{
    t_fcb *fcb = dictionary_get(args->dial_fs.archivos, archivo);
    config_set_value(fcb->metadata, "BLOQUE_INICIAL", string_itoa(nuevo_bloque_inicial));
    config_save(fcb->metadata); // Guardar los cambios en el archivo de metadata
    fcb->inicio = nuevo_bloque_inicial;
}

void metadata_modificar_tamanio_archivo(t_io *args, char *archivo, int nuevo_tamanio)
{
    t_fcb *fcb = dictionary_get(args->dial_fs.archivos, archivo);
    config_set_value(fcb->metadata, "TAMANIO_ARCHIVO", string_itoa(nuevo_tamanio));
    config_save(fcb->metadata); // Guardar los cambios en el archivo de metadata
    fcb->total_size = nuevo_tamanio;
    fcb->fin_bloque = fcb->inicio + (nuevo_tamanio / args->dial_fs.blockSize);
}