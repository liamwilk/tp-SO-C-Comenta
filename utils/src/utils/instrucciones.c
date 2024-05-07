#include "instrucciones.h"

int set(void *registro, uint32_t valor, int size)
{
    switch (size)
    {
    case sizeof(uint32_t):
        *(uint32_t *)registro = valor;
        return 0;
        break;

    case sizeof(uint8_t):
        *(uint8_t *)registro = (uint8_t)valor;
        return 0;
        break;
    default:
        return 1;
        break;
    }
}

int sum(void *registro_origen, void *registro_destino, int tamanio_destino)
{
    switch (tamanio_destino)
    {
    case sizeof(uint32_t):
        *(uint32_t *)registro_destino += *(uint32_t *)registro_origen;
        return 0;
        break;
    case sizeof(uint8_t):
        *(uint8_t *)registro_destino += *(uint8_t *)registro_origen;
        return 0;
        break;

    default:
        return 1;
        break;
    }
}

int sub(void *registro_origen, void *registro_destino, int tamanio_destino)
{
    switch (tamanio_destino)
    {
    case sizeof(uint32_t):
        *(uint32_t *)registro_destino -= *(uint32_t *)registro_origen;
        return 0;
    case sizeof(uint8_t):
        *(uint8_t *)registro_destino -= *(uint8_t *)registro_origen;
        return 0;
    default:
        return 1;
    }
}

int jnz(void *pc, void *registro, uint32_t instruccion)
{
    if (*(int *)registro == 0)
    { // todos los datos de tipo numérico representan al 0 de la misma forma, por lo que no hay riesgo con este casting explícito
        *(uint32_t *)pc = instruccion;
        return 0;
    }
    else
    {
        return 1;
    }
}

void io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo, int socket_kernel_dispatch, t_log *logger)
{

    t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_GEN_SLEEP);

    log_debug(logger, "Nombre de la Interfaz: %s", nombre_interfaz);
    log_debug(logger, "Cantidad de unidades de Trabajo: %d", unidades_trabajo);

    serializar_io_gen_sleep(&paquete, nombre_interfaz, unidades_trabajo);

    // Envio la instruccion a kernel
    enviar_paquete(paquete, socket_kernel_dispatch);
    eliminar_paquete(paquete);
}

void serializar_io_gen_sleep(t_paquete **paquete, char *nombre_interfaz, uint32_t unidades_trabajo)
{
    uint32_t size = strlen(nombre_interfaz) + 1;
    // Actualizo el buffer del paquete
    actualizar_buffer(*paquete, sizeof(uint32_t) + size + sizeof(uint32_t));

    // Serializo los datos
    serializar_uint32_t(size, *paquete);
    serializar_char(nombre_interfaz, *paquete);
    serializar_uint32_t(unidades_trabajo, *paquete);
}