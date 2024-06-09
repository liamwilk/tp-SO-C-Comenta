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
#include <utils/kernel/main.h>

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

/**
 * Envía un PCB a una CPU en el kernel.
 *
 * @param kernel El puntero al kernel.
 * @param pcb El puntero al PCB a enviar.
 * @param cpu El socket de la CPU a la que se enviará el PCB.
 */
void kernel_enviar_pcb_cpu(t_kernel *kernel, t_pcb *pcb, t_kernel_sockets cpu);

/** FUNCIONES DE CONSOLA**/

/**
 * @brief Revisa un paquete en el kernel.
 *
 * Esta función se encarga de revisar un paquete en el kernel y realizar las acciones correspondientes.
 *
 * @param args Los argumentos del hilo.
 * @param paquete El paquete a revisar.
 * @param modulo El módulo al que pertenece el paquete.
 */
void revisar_paquete_kernel(hilos_args *args, t_paquete *paquete, char *modulo);

/**
 * @brief Agrega una interfaz de entrada/salida en el kernel.
 *
 * Esta función se encarga de agregar una interfaz de entrada/salida en el kernel.
 *
 * @param args Los argumentos del hilo.
 * @param tipo El tipo de socket de la interfaz.
 * @param socket El socket de la interfaz.
 *
 * @return Un puntero a la estructura de entrada/salida creada.
 */
t_kernel_entrada_salida *entrada_salida_agregar_interfaz(hilos_args *args, KERNEL_SOCKETS tipo, int socket);

/**
 * @brief Remueve una interfaz de entrada/salida del kernel.
 *
 * Esta función se encarga de remover una interfaz de entrada/salida del kernel.
 *
 * @param args Los argumentos del hilo.
 * @param interfaz La interfaz a remover.
 */
void entrada_salida_remover_interfaz(hilos_args *args, char *interfaz);

/**
 * @brief Busca una interfaz de entrada/salida en el kernel.
 *
 * Esta función se encarga de buscar una interfaz de entrada/salida en el kernel.
 *
 * @param args Los argumentos del hilo.
 * @param interfaz La interfaz a buscar.
 *
 * @return Un puntero a la estructura de entrada/salida encontrada, o NULL si no se encontró.
 */
t_kernel_entrada_salida *entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz);
t_kernel_entrada_salida *kernel_sockets_agregar_entrada_salida(hilos_args *args, KERNEL_SOCKETS type, int socket);
void entrada_salida_agregar_identificador(hilos_io_args *args, char *identificador);
void hilos_ejecutar_entrada_salida(hilos_io_args *io_args, char *modulo, t_funcion_kernel_io_prt switch_case_atencion);
void entrada_salida_procesar_rechazado(hilos_io_args *argumentos, char *identificador);
#endif /* CONSOLA_H_ */
