#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <utils/serial.h>
#include <string.h>

/*  Funciones correspondientes a cada instrucción   */

/**
 * @brief Establece el valor de un registro.
 *
 * @param registro Puntero al registro.
 * @param valor Valor a establecer en el registro.
 * @param size Tamaño del registro en bytes.
 * @return 0 si se estableció correctamente, -1 en caso de error.
 */
int set(void *registro, uint32_t valor, int size);

/**
 * @brief Suma el contenido de un registro origen al contenido de un registro destino.
 *
 * @param registro_origen Puntero al registro origen.
 * @param registro_destino Puntero al registro destino.
 * @param tamanio_destino Tamaño del registro destino en bytes.
 * @return 0 si se realizó la suma correctamente, -1 en caso de error.
 */
int sum(void *registro_origen, void *registro_destino, int tamanio_destino);

/**
 * @brief Resta el contenido de un registro origen al contenido de un registro destino.
 *
 * @param registro_origen Puntero al registro origen.
 * @param registro_destino Puntero al registro destino.
 * @param tamanio_destino Tamaño del registro destino en bytes.
 * @return 0 si se realizó la resta correctamente, -1 en caso de error.
 */
int sub(void *registro_origen, void *registro_destino, int tamanio_destino);

/**
 * @brief Salta a una instrucción específica si el valor de un registro es distinto de cero.
 *
 * @param pc Puntero al contador de programa.
 * @param registro Puntero al registro a verificar.
 * @param instruccion Instrucción a la que se debe saltar.
 * @return 0 si se realizó el salto correctamente, -1 en caso de error.
 */
int jnz(void *pc, void *registro, uint32_t instruccion);

/**
 * @brief Realiza una pausa en la ejecución de un hilo de IO.
 *
 * @param nombre_interfaz Nombre de la interfaz de IO.
 * @param unidades_trabajo Unidades de trabajo a realizar.
 * @param socket_kernel_dispatch Socket de comunicación con el kernel dispatch.
 * @param logger Puntero al logger.
 */
void io_gen_sleep(char *nombre_interfaz, uint32_t unidades_trabajo, int socket_kernel_dispatch, t_log *logger);

/**
 * @brief Serializa los parámetros de la función io_gen_sleep en un paquete.
 *
 * @param paquete Doble puntero al paquete donde se almacenará la serialización.
 * @param nombre_interfaz Nombre de la interfaz de IO.
 * @param unidades_trabajo Unidades de trabajo a realizar.
 */
void serializar_io_gen_sleep(t_paquete **paquete, char *nombre_interfaz, uint32_t unidades_trabajo);

#endif /* INSTRUCCIONES_H */