#ifndef CPU_H_
#define CPU_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <stdio.h>
#include "serial.h"
#include "instrucciones.h"

/*Estructura basica de la CPU*/

typedef struct t_cpu
{
    int puertoEscuchaDispatch;
    int puertoEscuchaInterrupt;
    char *ipMemoria;
    int puertoMemoria;
    int cantidadEntradasTlb;
    char *algoritmoTlb;
} t_cpu;

typedef struct t_cpu_proceso
{
    uint32_t pid;
    t_registros_cpu registros;
} t_cpu_proceso;

/**
 * @fn    cpu_inicializar
 * @brief Inicializa el cpu junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
t_cpu cpu_inicializar(t_config *config);

/**
 * @fn    cpu_log
 * @brief Logs necesarios para cpu
 * @param config Instancia de module.config
 * @return t_cpu
 */
void cpu_log(t_cpu cpu, t_log *logger);

/**
 * Solicita la próxima instrucción de memoria para un proceso dado.
 *
 * @param pid El ID del proceso.
 * @param pc El contador de programa.
 * @param socket_memoria El socket para la comunicación con la memoria.
 */
void cpu_memoria_pedir_proxima_instruccion(t_cpu_proceso *proceso, int socket_memoria);

void cpu_ejecutar_instruccion(t_memoria_cpu_instruccion *datos_instruccion, t_instruccion INSTRUCCION, t_cpu_proceso *cpu_proceso, t_log *logger);

int cpu_memoria_recibir_instruccion(t_buffer *buffer, t_log *logger, t_memoria_cpu_instruccion *datos_instruccion, t_instruccion *INSTRUCCION, t_cpu_proceso *proceso);

void cpu_kernel_avisar_finalizacion(t_cpu_proceso proceso, int socket_kernel_interrupt);

t_cpu_proceso cpu_kernel_recibir_proceso(t_buffer *buffer, t_log *logger);

uint32_t *determinar_tipo_registro_uint32_t(char *instruccion, t_cpu_proceso *proceso);

uint8_t *determinar_tipo_registro_uint8_t(char *instruccion, t_cpu_proceso *proceso);
t_instruccion determinar_codigo_instruccion(char *instruccion);

t_registro obtener_tipo_registro(char *nombre_registro);
void imprimir_registros(t_log *logger, t_cpu_proceso *cpu_proceso);
void remover_salto_linea(char *argumento_origen);

#endif /* CPU_H_ */