#include <utils/entradasalida.h>
#include <dirent.h>
// El filesystem tomara todos los archivos salvo BLOQUES y BITMAP como metadata
#define BLOQUES = "bloques.dat"
#define BITMAP = "bitmap.dat"

typedef struct t_fcb
{
    uint32_t total_size;
    uint32_t inicio;
    uint32_t fin_bloque;
    t_list *metadata;
    void *archivo; // Archivo cargado en memoria (No se si es buena idea)
} t_fcb;

void metadata_inicializar(t_io *args, t_dictionary *archivos)
{
    log_info(args->logger, "Inicializando metadata");
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
                dictionary_put(archivos, entry->d_name, nuevo_fcb);
                log_info(args->logger, "Archivo: %s, Tamaño: %d, Bloque inicial: %d", entry->d_name, tamanio_en_bytes, bloque_inicial);
                free(file_path);
            }
        }
        closedir(dir);
    }
    free(full_path);
}