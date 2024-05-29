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

t_consola_operacion obtener_operacion(char *funcion);
void imprimir_comandos(hilos_args *args);
void imprimir_logo(hilos_args *args);
void imprimir_header(hilos_args *args);
t_pcb *buscar_proceso(t_diagrama_estados *estados, uint32_t pid);

void hilo_ejecutar_kernel(int socket, hilos_args *args, char *modulo, t_funcion_kernel_ptr switch_case_atencion);

/**
 * Agrega un socket al kernel
 *
 * @param args El puntero al hilos_args.
 * @param type El tipo de socket agregar
 * @param socket El socket a agregar
 * @return El struct de sockets actualizado
 */
t_kernel_sockets kernel_sockets_agregar(hilos_args *args, KERNEL_SOCKETS type, int socket);

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

void kernel_enviar_pcb_cpu(t_kernel *kernel, t_pcb *pcb, t_kernel_sockets cpu);

/** FUNCIONES DE CONSOLA**/

void revisar_paquete_kernel(hilos_args *args, t_paquete *paquete, char *modulo);

t_kernel_entrada_salida *entrada_salida_agregar_interfaz(hilos_args *args, KERNEL_SOCKETS tipo, int socket);
void entrada_salida_remover_interfaz(hilos_args *args, char *interfaz);
t_kernel_entrada_salida *entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz);
t_kernel_entrada_salida *kernel_sockets_agregar_entrada_salida(hilos_args *args, KERNEL_SOCKETS type, int socket);
void entrada_salida_agregar_identificador(hilos_io_args *args, char *identificador);

#endif /* CONSOLA_H_ */
