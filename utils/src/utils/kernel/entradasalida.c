#include "entradasalida.h"

void kernel_interrumpir_io(hilos_args *args, uint32_t pid, char *motivo)
{
    t_paquete *paquete = crear_paquete(KERNEL_IO_INTERRUPCION);
    t_kernel_io_interrupcion interrupcion = {.pid = pid, .motivo = motivo, .len_motivo = strlen(motivo) + 1};
    serializar_t_kernel_io_interrupcion(&paquete, &interrupcion);

    t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz_pid(args, pid);

    io->ocupado = 0;
    io->pid = 0;

    enviar_paquete(paquete, io->socket);
    eliminar_paquete(paquete);
}

t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz)
{
    // Busco el indice en la lista de entrada/salida
    int *indice = dictionary_get(args->kernel->sockets.dictionary_entrada_salida, interfaz);

    // Si no se encuentra la interfaz en el diccionario, no se puede buscar
    if (indice == NULL)
    {
        return NULL;
    }

    // Obtengo el TAD de la lista de entrada/salida
    t_kernel_entrada_salida *entrada_salida = list_get(args->kernel->sockets.list_entrada_salida, *indice);

    if (entrada_salida == NULL)
    {
        return NULL;
    }

    kernel_log_generic(args, LOG_LEVEL_DEBUG, "Se encontro el modulo de entrada/salida en el socket %d asociado a la interfaz %s", entrada_salida->socket, interfaz);

    return entrada_salida;
}
t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz_pid(hilos_args *args, uint32_t pid)
{
    for (int i = 0; i < list_size(args->kernel->sockets.list_entrada_salida); i++)
    {
        t_kernel_entrada_salida *modulo = list_get(args->kernel->sockets.list_entrada_salida, i);
        if (modulo->pid == pid)
        {
            return modulo;
        }
    }
    return NULL;
}