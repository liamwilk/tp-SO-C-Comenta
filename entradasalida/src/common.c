#include "common.h"

void* identificar_modulo(char* nombre, int socket_kernel){
    t_paquete *identificacion = crear_paquete(IO_IDENTIFICADOR);

    actualizar_buffer(identificacion, strlen(nombre)+1+sizeof(uint32_t));

    serializar_uint32_t(strlen(nombre)+1,identificacion);
    serializar_char(nombre, identificacion);

    enviar_paquete(identificacion, socket_kernel);
    eliminar_paquete(identificacion);
    return NULL;
}