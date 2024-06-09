#include "entradasalida.h"

void kernel_interrumpir_io(t_list *list_entrada_salida, uint32_t pid, char *motivo)
{
    t_paquete *paquete = crear_paquete(KERNEL_IO_INTERRUPCION);
    t_kernel_io_interrupcion interrupcion = {.pid = pid, .motivo = motivo, .len_motivo = strlen(motivo) + 1};
    serializar_t_kernel_io_interrupcion(&paquete, &interrupcion);

    t_kernel_entrada_salida *io = kernel_entrada_salida_buscar_interfaz(list_entrada_salida, pid);

    io->ocupado = 0;
    io->pid = 0;

    enviar_paquete(paquete, io->socket);
    eliminar_paquete(paquete);
}

t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(t_list *list_entrada_salida, uint32_t pid)
{
    for (int i = 0; i < list_size(list_entrada_salida); i++)
    {
        t_kernel_entrada_salida *modulo = list_get(list_entrada_salida, i);
        if (modulo->pid == pid)
        {
            return modulo;
        }
    }
    return NULL;
}