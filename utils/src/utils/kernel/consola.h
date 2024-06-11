#ifndef KERNEL_CONSOLA_H_
#define KERNEL_CONSOLA_H_
#include "structs.h"
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include <readline/readline.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <pwd.h>
#include <ctype.h>

/**
 * @brief Formatea y registra un mensaje genérico con un número variable de argumentos.
 *
 * Esta función toma un número variable de argumentos y los formatea en un mensaje utilizando la cadena de formato proporcionada.
 * Luego, registra el mensaje formateado en el nivel de registro especificado.
 *
 * @param args Un puntero a la estructura hilos_args.
 * @param nivel El nivel de registro en el que se registrará el mensaje.
 * @param mensaje La cadena de formato para el mensaje.
 * @param ... Los argumentos variables que se formatearán en el mensaje.
 */
void kernel_log_generic(hilos_args *args, t_log_level nivel, const char *mensaje, ...) __attribute__((format(printf, 3, 4)));

/**
 * Registra un mensaje genérico en el log del kernel con el nivel de log especificado.
 *
 * @param args Los argumentos para el hilo.
 * @param nivel El nivel de log del mensaje.
 * @param mensaje El mensaje a registrar.
 */
void kernel_log_generic_rl(hilos_args *args, t_log_level nivel, const char *mensaje);

/**
 * Log oficial de la catedra que permite mostrar por pantalla la cola de rady
 *
 * @param kernel_hilos_args Los argumentos para los hilos del kernel.
 * @param prioridad         Establece si loguear la cola de prioridad o no (VRR)
 */
void kernel_log_ready(hilos_args *kernel_hilos_args, bool prioridad);

/**
 * @brief Realiza el autocompletado de una instrucción.
 *
 * @param input_text El texto de entrada.
 * @param state El estado del autocompletado.
 * @return La instrucción autocompletada.
 */
char *autocompletado_instruccion(const char *input_text, int state);

/**
 * @brief Realiza el autocompletado general.
 *
 * @param text El texto de entrada.
 * @param start El índice de inicio del autocompletado.
 * @param end El índice de fin del autocompletado.
 * @return Un arreglo de cadenas con las opciones de autocompletado.
 */
char **autocompletado(const char *text, int start, int end);

/**
 * @brief Realiza el autocompletado de un argumento.
 *
 * @param input_text El texto de entrada.
 * @param state El estado del autocompletado.
 * @return El argumento autocompletado.
 */
char *autocompletado_argumento(const char *input_text, int state);

/**
 * @brief Obtiene el ancho del terminal.
 *
 * @return El ancho del terminal.
 */
int obtener_ancho_terminal();

/**
 * @brief Actualiza el prompt cuando se recibe una señal.
 *
 * @param signal La señal recibida.
 */
void actualizar_prompt(int signal);

/**
 * @brief Obtiene la operación correspondiente a una función.
 *
 * @param funcion El nombre de la función.
 * @return La operación correspondiente a la función.
 */
t_consola_operacion obtener_operacion(char *funcion);

/**
 * @brief Ejecuta un script de instrucciones.
 *
 * @param path_instrucciones La ruta del archivo de instrucciones.
 * @param hiloArgs Los argumentos del hilo.
 */
void ejecutar_script(char *path_instrucciones, hilos_args *hiloArgs);

/**
 * @brief Genera el prompt.
 *
 * @return El prompt generado.
 */
char *generar_prompt();

/**
 * @brief Reinicia el prompt.
 *
 * @param hiloArgs Los argumentos del hilo.
 */
void reiniciar_prompt(hilos_args *hiloArgs);

/**SCRIPT DE CONSOLA**/

/**
 * @brief Función para iniciar un proceso en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void iniciar_proceso(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para finalizar un proceso en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void finalizar_proceso(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para iniciar la planificación de procesos en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void iniciar_planificacion(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para detener la planificación de procesos en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void detener_planificacion(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para establecer el grado de multiprogramación en el kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void multiprogramacion(char **separar_linea, hilos_args *hiloArgs);

/**
 * @brief Función para mostrar el estado de los procesos en el kernel.
 *
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void procesos_estados(hilos_args *hiloArgs);

/**
 * @brief Función para finalizar la consola del kernel.
 *
 * @param separar_linea Array de strings con los argumentos de la línea de comando.
 * @param hiloArgs Estructura de argumentos del hilo.
 */
void finalizar_consola(char **separar_linea, hilos_args *hiloArgs);

/**
 * Imprime los comandos disponibles.
 *
 * @param args Los argumentos para el hilo.
 */
void imprimir_comandos(hilos_args *args);

/**
 * Imprime el logo.
 *
 * Esta función imprime el logo utilizando los argumentos proporcionados.
 *
 * @param args Los argumentos para imprimir el logo.
 */
void imprimir_logo(hilos_args *args);

/**
 * Imprime el encabezado para la consola.
 *
 * @param args Los argumentos para el hilo.
 */
void imprimir_header(hilos_args *args);

/**
 * @fn    log_kernel
 * @brief Log obligatorios de kernel junto con su configuracion
 * @param kernel Instancia de kernel (kernel_inicializar)
 * @param logger Instancia de t_log
 */
void kernel_log(hilos_args *args);

#endif /* KERNEL_CONSOLA_H_ */
