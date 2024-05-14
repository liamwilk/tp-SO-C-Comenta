#ifndef CONSOLA_H_
#define CONSOLA_H_
#include "string.h"
#include "readline/readline.h"
#include "readline/history.h"
#include "commons/log.h"
#include "utils/modulos.h"
#include "utils/serial.h"
#include "utils/conexiones.h"
#include "commons/string.h"
#include "utils/procesos.h"
#include <utils/kernel.h>

typedef void (*t_funcion_kernel_ptr)(t_log *, t_op_code, hilos_args *, t_buffer *);

typedef enum
{
    PROCESO_ESTADO,
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    MULTIPROGRAMACION,
    FINALIZAR_PROCESO,
    FINALIZAR_CONSOLA,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    TOPE_ENUM_CONSOLA // siempre mantener este al final para saber el tamaño del enum
} t_consola_operacion;

t_consola_operacion obtener_operacion(char *funcion);
void imprimir_comandos(hilos_args *args);
void imprimir_logo(hilos_args *args);
void imprimir_header(hilos_args *args);
t_pcb *buscar_proceso(t_diagrama_estados *estados, uint32_t pid);

void log_generic_rl(hilos_args *args, t_log_level nivel, const char *mensaje);
void log_generic(hilos_args *args, t_log_level nivel, const char *mensaje, ...) __attribute__((format(printf, 3, 4)));

void hilo_ejecutar_kernel(int socket, hilos_args *args, char *modulo, t_funcion_kernel_ptr switch_case_atencion);

/**
 * Agrega un socket al kernel
 *
 * @param kernel El puntero al kernel.
 * @param type El tipo de socket agregar
 * @param socket El socket a agregar
 * @return El struct de sockets actualizado
 */
t_kernel_sockets kernel_sockets_agregar(t_kernel *kernel, KERNEL_SOCKETS type, int socket);

/**
 * @fn    kernel_inicializar
 * @brief Inicializa el kernel junto con todas sus configuraciones
 * @param config Instancia de module.config
 * @return Kernel: Struct de kernel
 */
t_kernel kernel_inicializar(t_config *config);

/**
 * @fn    log_kernel
 * @brief Log obligatorios de kernel junto con su configuracion
 * @param kernel Instancia de kernel (kernel_inicializar)
 * @param logger Instancia de t_log
 */
void kernel_log(hilos_args *args);

/**----PROCESOS Y ESTADOS----**/

/**
 * @brief Crea un nuevo proceso en el kernel.
 *
 * Esta función se encarga de crear un nuevo proceso en el kernel. Envia el struct t_kernel_memoria_proceso al modulo memoria
 *
 * @param kernel Un puntero a la estructura del kernel.
 * @param new  Un puntero a la cola de new.
 * @param logger Un puntero al logger.
 * @param instrucciones Una cadena de caracteres que contiene las instrucciones para el nuevo proceso.
 */
t_pcb *kernel_nuevo_proceso(hilos_args *args, t_diagrama_estados *estados, t_log *logger, char *instrucciones);

void kernel_enviar_pcb_cpu(t_kernel *kernel, t_pcb *pcb, KERNEL_SOCKETS cpu);

/** FUNCIONES DE CONSOLA**/

void kernel_finalizar(hilos_args *args);

t_pcb *kernel_transicion_ready_exec(t_diagrama_estados *estados, t_kernel *kernel);

t_pcb *kernel_transicion_exec_block(t_diagrama_estados *estados);

t_pcb *kernel_transicion_block_ready(t_diagrama_estados *estados, t_log *logger);

t_pcb *kernel_transicion_exec_ready(t_diagrama_estados *estados, t_log *logger, t_kernel *kernel);

void kernel_desalojar_proceso(t_diagrama_estados *estados, t_kernel *kernel, t_log *logger, t_pcb *proceso);
void revisar_paquete_kernel(hilos_args *args, t_paquete *paquete, char *modulo);
#endif /* CONSOLA_H_ */
