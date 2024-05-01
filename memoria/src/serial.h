#ifndef SERIAL_H
#define SERIAL_H

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<pthread.h>
#include<limits.h>
#include <utils/handshake.h>
#include <utils/modulos.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

/**
 * @fn    *deserializar_t_kernel_memoria
 * @brief Deserializa un buffer en un t_kernel_memoria
 * @param buffer El buffer a deserializar
 */
t_kernel_memoria *deserializar_t_kernel_memoria(t_buffer* buffer);

/**
 * @fn    *deserializar_t_cpu_memoria_instruccion
 * @brief Deserializa un buffer en un t_cpu_memoria_instruccion
 * @param buffer El buffer a deserializar
 * @return *t_cpu_memoria_instruccion
 */
t_cpu_memoria_instruccion *deserializar_t_cpu_memoria_instruccion(t_buffer *buffer);

/**
 * @fn    serializar_t_memoria_cpu_instruccion
 * @brief Serializa un t_memoria_cpu_instruccion en un paquete
 * @param paquete_instruccion El paquete a serializar
 * @param instruccion La instruccion a serializar
 * @return *t_cpu_memoria_instruccion
 */
void serializar_t_memoria_cpu_instruccion(t_paquete** paquete_instruccion, t_memoria_cpu_instruccion *instruccion);

#endif // SERIAL_H