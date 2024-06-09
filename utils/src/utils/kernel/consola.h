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

// TODO: Mover a un archivo de utilidades
void manejador_interrupciones(union sigval arg);
int interrumpir_temporizador(hilos_args *args);
void inicializar_temporizador(hilos_args *args, timer_args_t *temporizador);
void iniciar_temporizador(hilos_args *args, int duracion);