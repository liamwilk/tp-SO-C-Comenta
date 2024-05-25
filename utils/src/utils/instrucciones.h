#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <utils/serial.h>
#include <string.h>

/*  Funciones correspondientes a cada instrucción   */

int set(void *registro, uint32_t valor, int size);

int sum(void *registro_origen, void *registro_destino, int tamanio_destino);

int sub(void *registro_origen, void *registro_destino, int tamanio_destino);

int jnz(void *pc, void *registro, uint32_t instruccion);

void io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo, int socket_kernel_dispatch, t_log *logger);

void serializar_io_gen_sleep(t_paquete **paquete, char *nombre_interfaz, uint32_t unidades_trabajo);

#endif /* INSTRUCCIONES_H */