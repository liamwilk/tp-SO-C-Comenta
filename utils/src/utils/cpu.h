#ifndef CPU_H_
#define CPU_H_

#include <math.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/serial.h>
#include <utils/procesos.h>
#include <utils/template.h>
#include <utils/instrucciones.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <utils/handshake.h>
#include <commons/temporal.h>

typedef enum
{
    FIFO_TLB,
    LRU
} t_algoritmo_tlb;

typedef struct
{
    pthread_t thread_atender_kernel_dispatch, thread_atender_kernel_interrupt, thread_conectar_memoria, thread_atender_memoria, thread_esperar_kernel_dispatch, thread_esperar_kernel_interrupt, thread_mmu;
} t_threads;

typedef struct
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
} t_config_leida;

typedef struct t_cpu_proceso
{
    uint32_t pid;
    uint32_t ejecutado;
    t_registros_cpu registros;
} t_cpu_proceso;

typedef struct t_cpu
{
    t_config_leida config_leida;
    uint32_t tam_pagina;
    uint32_t pid;
    t_registros_cpu registros;
    t_log *logger;
    t_config *config;
    t_instruccion tipo_instruccion;
    char **instruccion;
    uint32_t cantidad_elementos;
    t_threads threads;
    t_cpu_proceso proceso;
    int flag_interrupt;
    /////////////////////////
    t_tlb *tlb;
    uint32_t proximo_indice_reemplazo;
    uint32_t direccion_fisica;
    uint32_t direccion_logica;
    sem_t mmu_ejecucion;
    uint32_t resultado;
    uint32_t marco;
    uint32_t pagina;
    t_instruccion codigo;
    void *paquete;
    sem_t log_sem;
} t_cpu;

typedef void (*t_funcion_cpu_ptr)(t_cpu *, t_op_code, t_buffer *);

/**
 * @fn    cpu_leer_config
 * @brief Inicializa el cpu junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return config
 */
void cpu_leer_config(t_cpu *args);

/**
 * @fn    cpu_imprimir_log
 * @brief Logs necesarios para cpu
 * @param config Instancia de module.config
 * @return t_cpu
 */
void cpu_imprimir_log(t_cpu *cpu);

void instruccion_solicitar(t_cpu *args);

int instruccion_ejecutar(t_cpu *args);

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
int instruccion_recibir(t_cpu *args, t_buffer *buffer);

/**
 * Avisa al kernel que el proceso ha finalizado en la CPU.
 *
 * @param proceso El proceso de la CPU.
 * @param socket_kernel_dispatch El socket de conexión con el kernel.
 */
void instruccion_finalizar(t_cpu *args);

void proceso_recibir(t_cpu *args, t_buffer *buffer);

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

void imprimir_registros(t_cpu *args);

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
void recibir_interrupcion(t_cpu *logger, t_buffer *buffer);

/**
 * Procesa una interrupción en la CPU.
 *
 * @param logger El logger para registrar eventos.
 * @param cpu La CPU que recibió la interrupción.
 * @param proceso El proceso de la CPU.
 */
void instruccion_interrupt(t_cpu *cpu);

/**
 * Registra una instrucción en el logger.
 *
 * @param logger El logger para registrar eventos.
 * @param cpu_proceso El proceso de la CPU.
 * @param datos_instruccion Los datos de la instrucción.
 */
void instruccion_log(t_cpu *args);

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
uint32_t calcular_numero_pagina(t_cpu *cpu, uint32_t direccion_logica);

/**
 * @brief Función para atender el kernel dispatch.
 *
 * @return void* Puntero a la función atender_kernel_dispatch.
 */
void *atender_kernel_dispatch(void *args);

/**
 * @brief Función para atender la interrupción del kernel.
 *
 * @return void* Puntero a la función atender_kernel_interrupt.
 */
void *atender_kernel_interrupt(void *args);

/**
 * @brief Función para atender la memoria.
 *
 * @return void* Puntero a la función atender_memoria.
 */
void *atender_memoria(void *args);

/**
 * @brief Función para esperar el kernel dispatch.
 *
 * @return void* Puntero a la función esperar_kernel_dispatch.
 */
void *esperar_kernel_dispatch(void *args);

/**
 * @brief Función para esperar la interrupción del kernel.
 *
 * @return void* Puntero a la función esperar_kernel_interrupt.
 */
void *esperar_kernel_interrupt(void *args);

/**
 * @brief Función para conectar con la memoria.
 *
 * @return void* Puntero a la función conectar_memoria.
 */
void *conectar_memoria(void *args);

/**
 * @brief Deserializa un buffer en una estructura t_memoria_cpu_instruccion.
 *
 * Esta función toma un buffer y lo deserializa en una estructura t_memoria_cpu_instruccion.
 *
 * @param buffer El buffer a deserializar.
 * @return Un puntero a la estructura t_memoria_cpu_instruccion deserializada.
 */
t_memoria_cpu_instruccion *deserializar_t_memoria_cpu_instruccion(t_buffer *buffer);

/**
 * @brief Deserializa un buffer en una estructura t_kernel_cpu_proceso.
 *
 * Esta función toma un buffer y lo deserializa en una estructura t_kernel_cpu_proceso.
 *
 * @param buffer El buffer a deserializar.
 * @return Un puntero a la estructura t_kernel_cpu_proceso deserializada.
 */
t_kernel_cpu_proceso *deserializar_t_kernel_cpu_proceso(t_buffer *buffer);

/**
 * @brief Función que realiza un switch case para determinar la acción a realizar en la memoria.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param codigo_operacion Código de operación que indica la acción a realizar.
 * @param buffer Puntero al buffer que contiene los datos necesarios para la operación.
 */
void switch_case_memoria(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función que realiza un switch case para determinar la acción a realizar en el kernel durante un dispatch.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param codigo_operacion Código de operación que indica la acción a realizar.
 * @param buffer Puntero al buffer que contiene los datos necesarios para la operación.
 */
void switch_case_kernel_dispatch(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función que realiza un switch case para determinar la acción a realizar en el kernel durante una interrupción.
 *
 * @param logger Puntero al logger utilizado para registrar eventos.
 * @param codigo_operacion Código de operación que indica la acción a realizar.
 * @param buffer Puntero al buffer que contiene los datos necesarios para la operación.
 */
void switch_case_kernel_interrupt(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer);

/**
 * @brief Función que determina el código de instrucción a partir de una cadena de caracteres.
 *
 * @param instruccion Cadena de caracteres que representa una instrucción.
 * @return El código de instrucción correspondiente.
 */
t_instruccion determinar_codigo_instruccion(char *instruccion);

typedef struct
{
    t_paquete *paquete;
} cleanup_args_t;

void hilo_ejecutar_cpu(t_cpu *args, int socket, char *modulo, t_funcion_cpu_ptr switch_case_atencion);

void inicializar_hilos_cpu(t_cpu *argumentos);

void inicializar_argumentos_cpu(t_cpu *args, t_log_level nivel, int argc, char *argv[]);

void inicializar_servidores_cpu(t_cpu *argumentos);

void liberar_recursos_cpu(t_cpu *argumentos);

void inicializar_modulo_cpu(t_cpu *argumentos, t_log_level nivel, int argc, char *argv[]);

void instruccion_ciclo(t_cpu *args, t_buffer *buffer);

void inicializar_tlb(t_cpu *args);

void *hilo_mmu(void *args_void);

int buscar_entrada_vacia_tlb(uint32_t cantidad_entradas_tlb, t_tlb *tlb);

int buscar_en_tlb(uint32_t pid, uint32_t pagina, uint32_t cantidad_entradas_tlb, t_tlb *tlb);

void agregar_en_tlb(uint32_t pid, uint32_t pagina, uint32_t frame, t_cpu *args);

int reemplazar_en_tlb(char *algoritmo_reemplazo, uint32_t cantidad_entradas_tlb, t_cpu *args);

t_algoritmo_tlb determinar_codigo_algoritmo(char *algoritmo_tlb);

void mmu_iniciar(t_cpu *cpu, t_instruccion codigo, uint32_t direccion_logica, void *paquete);

void cleanup(void *arg);

#endif /* CPU_H_ */