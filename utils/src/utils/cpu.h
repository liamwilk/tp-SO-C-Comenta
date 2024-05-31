#ifndef CPU_H_
#define CPU_H_

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/serial.h>
#include <utils/instrucciones.h>
#include <math.h>

/*Estructura basica de la CPU*/

typedef struct t_cpu
{
    int puertoEscuchaDispatch;
    int puertoEscuchaInterrupt;
    char *ipMemoria;
    int puertoMemoria;
    int cantidadEntradasTlb;
    char *algoritmoTlb;
    int socket_kernel_dispatch;
    int socket_kernel_interrupt;
    int socket_memoria;
    int socket_server_dispatch;
    int socket_server_interrupt;
    uint32_t tam_pagina;
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

/**
 * Ejecuta una instrucción en la CPU.
 *
 * @param paquete El paquete de datos de la CPU.
 * @param datos_instruccion Los datos de la instrucción a ejecutar.
 * @param INSTRUCCION La instrucción a ejecutar.
 * @param cpu_proceso El proceso de la CPU.
 * @param logger El logger para registrar eventos.
 * @return El resultado de la ejecución de la instrucción.
 */
int cpu_ejecutar_instruccion(t_cpu paquete, t_memoria_cpu_instruccion *datos_instruccion, t_instruccion INSTRUCCION, t_cpu_proceso *cpu_proceso, t_log *logger);

/**
 * Recibe una instrucción de memoria en la CPU.
 *
 * @param buffer El buffer que contiene la instrucción.
 * @param logger El logger para registrar eventos.
 * @param datos_instruccion Los datos de la instrucción recibida.
 * @param INSTRUCCION La instrucción recibida.
 * @param proceso El proceso de la CPU.
 * @return 0 si se recibió la instrucción correctamente, -1 en caso contrario.
 */
int cpu_memoria_recibir_instruccion(t_buffer *buffer, t_log *logger, t_memoria_cpu_instruccion *datos_instruccion, t_instruccion *INSTRUCCION, t_cpu_proceso *proceso);

/**
 * Avisa al kernel que el proceso ha finalizado en la CPU.
 *
 * @param proceso El proceso de la CPU.
 * @param socket_kernel_dispatch El socket de conexión con el kernel.
 */
void cpu_kernel_avisar_finalizacion(t_cpu_proceso proceso, int socket_kernel_dispatch);

/**
 * Recibe un proceso del kernel en la CPU.
 *
 * @param buffer El buffer que contiene el proceso.
 * @param logger El logger para registrar eventos.
 * @return El proceso recibido del kernel.
 */
t_cpu_proceso cpu_kernel_recibir_proceso(t_buffer *buffer, t_log *logger);

/**
 * Determina el tipo de registro como uint32_t en base a una instrucción.
 *
 * @param instruccion La instrucción a analizar.
 * @param proceso El proceso de la CPU.
 * @return Un puntero al tipo de registro uint32_t.
 */
uint32_t *determinar_tipo_registro_uint32_t(char *instruccion, t_cpu_proceso *proceso);

/**
 * Determina el tipo de registro como uint8_t en base a una instrucción.
 *
 * @param instruccion La instrucción a analizar.
 * @param proceso El proceso de la CPU.
 * @return Un puntero al tipo de registro uint8_t.
 */
uint8_t *determinar_tipo_registro_uint8_t(char *instruccion, t_cpu_proceso *proceso);

/**
 * Determina el código de una instrucción en base a su nombre.
 *
 * @param instruccion El nombre de la instrucción.
 * @return El código de la instrucción.
 */
t_instruccion determinar_codigo_instruccion(char *instruccion);

/**
 * Obtiene el tipo de registro en base a su nombre.
 *
 * @param nombre_registro El nombre del registro.
 * @return El tipo de registro.
 */
t_registro obtener_tipo_registro(char *nombre_registro);

/**
 * Imprime los registros del proceso de la CPU.
 *
 * @param logger El logger para registrar eventos.
 * @param cpu_proceso El proceso de la CPU.
 */
void imprimir_registros(t_log *logger, t_cpu_proceso *cpu_proceso);

/**
 * Verifica si un valor uint32_t es válido para ser convertido a uint8_t.
 *
 * @param valor El valor a verificar.
 * @return true si el valor es válido, false en caso contrario.
 */
bool casteo_verificar_uint_8t_valido(uint32_t valor);

/**
 * Realiza el casteo de un valor uint32_t a uint8_t.
 *
 * @param valor El valor a castear.
 * @return El valor castado.
 */
uint8_t casteo_uint8_t(uint32_t valor);

/**
 * Realiza el casteo de un valor uint8_t a uint32_t.
 *
 * @param valor El valor a castear.
 * @return El valor castado.
 */
uint32_t casteo_uint32_t(uint8_t valor);

/**
 * Recibe una interrupción en la CPU.
 *
 * @param logger El logger para registrar eventos.
 * @param buffer El buffer que contiene la interrupción.
 * @param proceso El proceso de la CPU.
 * @return 0 si se recibió la interrupción correctamente, -1 en caso contrario.
 */
int cpu_recibir_interrupcion(t_log *logger, t_buffer *buffer, t_cpu_proceso proceso);

/**
 * Procesa una interrupción en la CPU.
 *
 * @param logger El logger para registrar eventos.
 * @param cpu La CPU que recibió la interrupción.
 * @param proceso El proceso de la CPU.
 */
void cpu_procesar_interrupt(t_log *logger, t_cpu cpu, t_cpu_proceso proceso);

/**
 * Registra una instrucción en el logger.
 *
 * @param logger El logger para registrar eventos.
 * @param cpu_proceso El proceso de la CPU.
 * @param datos_instruccion Los datos de la instrucción.
 */
void log_instruccion(t_log *logger, t_cpu_proceso *cpu_proceso, t_memoria_cpu_instruccion *datos_instruccion);

/**
 * Envía un aviso a la memoria sobre el tamaño de página de la CPU.
 *
 * @param cpu La CPU que envía el aviso.
 */
void cpu_enviar_aviso_memoria_tam_pagina(t_cpu *cpu);

/**
 * Realiza la traducción de una dirección lógica a una dirección física utilizando la MMU.
 *
 * @param cpu La CPU que realiza la traducción.
 * @param direccion_logica La dirección lógica a traducir.
 * @param numero_marco El número de marco correspondiente a la dirección lógica.
 * @return La dirección física resultante.
 */
int mmu(t_cpu *cpu, uint32_t direccion_logica, uint32_t numero_marco);

/**
 * Calcula el número de página correspondiente a una dirección lógica.
 *
 * @param cpu Puntero a la estructura de la CPU.
 * @param direccion_logica Dirección lógica a calcular.
 * @return El número de página correspondiente a la dirección lógica.
 */
int calcular_numero_pagina(t_cpu *cpu, uint32_t direccion_logica);

#endif /* CPU_H_ */