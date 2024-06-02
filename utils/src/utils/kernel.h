#ifndef KERNEL_H_
#define KERNEL_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <utils/serial.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <commons/collections/queue.h>
#include "procesos.h"
#include "conexiones.h"
#include <semaphore.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <pwd.h>

/*Estructura basica del kernel*/
typedef enum
{
    FIFO,
    RR,
    VRR
} t_algoritmo;
typedef enum
{
    MEMORIA,
    CPU_DISPATCH,
    CPU_INTERRUPT,
    SERVER,
    ENTRADA_SALIDA_GENERIC,
    ENTRADA_SALIDA_STDIN,
    ENTRADA_SALIDA_STDOUT,
    ENTRADA_SALIDA_DIALFS
} KERNEL_SOCKETS;

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

typedef enum
{
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_INTERFACE,
    OUT_OF_MEMORY,
    INTERRUPTED_BY_USER
} KERNEL_MOTIVO_FINALIZACION;

typedef struct
{
    int pid;
    int ocupado;
    int orden;
    int socket;
    bool identificado;
    bool valido;
    char *interfaz;
    KERNEL_SOCKETS tipo;
} t_kernel_entrada_salida;

typedef struct t_kernel_sockets
{
    int memoria;
    int cpu_dispatch;
    int cpu_interrupt;
    int server;
    int entrada_salida;
    int entrada_salida_generic;
    int entrada_salida_stdin;
    int entrada_salida_stdout;
    int entrada_salida_dialfs;
    int id_entrada_salida;
    t_list *list_entrada_salida;
    t_dictionary *dictionary_entrada_salida;
} t_kernel_sockets;

typedef struct
{
    pthread_t thread_atender_entrada_salida;
    pthread_t thread_atender_cpu_interrupt;
    pthread_t thread_atender_cpu_dispatch;
    pthread_t thread_atender_memoria;
    pthread_t thread_atender_consola;
    pthread_t thread_planificador;
} t_kernel_threads;

typedef struct t_kernel
{
    int puertoEscucha;
    char *ipMemoria;
    int puertoMemoria;
    char *ipCpu;
    int puertoCpuDispatch;
    int puertoCpuInterrupt;
    char *algoritmoPlanificador;
    int quantum;
    char *recursos;
    char *instanciasRecursos;
    int gradoMultiprogramacion;
    sem_t planificador_iniciar;
    sem_t sistema_finalizar;
    sem_t log_lock;
    sem_t memoria_consola_finalizacion_proceso;
    bool detener_planificador;
    bool estado_planificador;
    t_kernel_sockets sockets;
    t_kernel_threads threads;
    pthread_mutex_t lock;
} t_kernel;

typedef struct
{
    t_log *logger;
    t_kernel *kernel;
    t_diagrama_estados *estados;
    int kernel_orden_apagado;
} hilos_args;

typedef struct
{
    hilos_args *args;
    t_kernel_entrada_salida *entrada_salida;
} hilos_io_args;

typedef void (*t_funcion_kernel_ptr)(t_log *, t_op_code, hilos_args *, t_buffer *);
typedef void (*t_funcion_kernel_io_prt)(hilos_io_args *, char *, t_op_code, t_buffer *);

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

void kernel_log_generic_rl(hilos_args *args, t_log_level nivel, const char *mensaje);

/**
 * Inicializa los estados de las colas del kernel.
 *
 * Esta función inicializa los estados de las colas del kernel, que incluyen las colas new, ready, exec, block y exit.
 *
 * @param estados  Diagrama de 5 estados
 */
t_diagrama_estados kernel_inicializar_estados(t_diagrama_estados *estados);

/**
 * @brief Desaloja un proceso del kernel.
 *
 * Esta función se encarga de desalojar un proceso del kernel, actualizando el diagrama de estados,
 * Se utiliza principalmente en ROUND ROBIN o VIRTUAL ROUND ROBIN
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 * @param pid Proceso a desalojar
 */
void kernel_desalojar_proceso(hilos_args *kernel_hilos_args, t_pcb *pcb);

/**
 * Transiciona un PCB del estado EXEC al estado READY en el diagrama de estados del kernel.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_ready(hilos_args *kernel_hilos_args);

/**
 * Transiciona un PCB del estado READY al estado EXEC en el kernel.
 *
 * Esta función toma como parámetros un diagrama de estados y un kernel, y transiciona un Bloque de Control de Proceso (PCB)
 * del estado READY al estado EXEC en el kernel. Retorna un puntero al PCB actualizado.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_ready_exec(hilos_args *kernel_hilos_args);

/**
 * Transiciona un proceso del estado bloqueado al estado listo en el kernel.
 *
 * Esta función toma como parámetros un diagrama de estados y un logger. Extrae un proceso del estado bloqueado,
 * lo coloca en el estado listo y devuelve el proceso. Si no hay procesos en el estado bloqueado, devuelve NULL.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 * @param modulo
 * @param unidad
 */
t_pcb *kernel_transicion_block_ready(hilos_io_args *io_args, char *modulo, t_entrada_salida_kernel_unidad_de_trabajo *unidad);

/**
 * @brief Función de transición para ejecutar un bloque en el kernel.
 *
 * Esta función realiza la transición del estado exec al estado "block" y devuelve el siguiente PCB (Bloque de Control de Proceso)
 * a ser ejecutado según el diagrama de estados y el logger proporcionados.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_block(hilos_args *kernel_hilo_args);

/**
 * @brief Realiza la transición del estado EXEC al estado EXIT en el kernel.
 *
 * Esta función actualiza el diagrama de estados y registra la transición en el logger.
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_exec_exit(hilos_args *kernel_hilo_args);

/**
 * @brief Función de transición para ejecutar un bloque en el kernel.
 *
 * Esta función realiza la transición del estado new al estado "ready" y devuelve el  PCB (Bloque de Control de Proceso)
 *
 * @param kernel_hilos_args Son los argumentos que recibe cada hilo al momento de ejecutarse
 */
t_pcb *kernel_transicion_new_ready(hilos_args *kernel_hilo_args);

/**
 * Finaliza el kernel.
 *
 * Esta función se encarga de finalizar el kernel y realizar cualquier limpieza necesaria.
 *
 * @param args Los argumentos para el kernel.
 */
void kernel_finalizar(hilos_args *args);

/**
 * Busca una interfaz en las estructuras de entrada/salida del kernel.
 *
 * @param args Los argumentos para el hilo.
 * @param interfaz El nombre de la interfaz a buscar.
 * @return Un puntero a la estructura de entrada/salida encontrada, o NULL si no se encuentra.
 */
t_kernel_entrada_salida *entrada_salida_buscar_interfaz(hilos_args *args, char *interfaz);
/**
 * @brief Función para registrar el estado de un hilo listo para ejecutar.
 *
 * @param kernel_hilos_args Estructura de argumentos del kernel.
 */
void log_ready(hilos_args *kernel_hilos_args, bool prioridad);

/**
 * @brief Función para finalizar un proceso en el kernel.
 *
 * @param kernel_hilos_args Estructura de argumentos del kernel.
 * @param pid Identificador del proceso a finalizar.
 * @param MOTIVO Motivo de finalización del proceso.
 * @return true si se pudo finalizar el proceso, false en caso contrario.
 */
bool kernel_finalizar_proceso(hilos_args *kernel_hilos_args, uint32_t pid, KERNEL_MOTIVO_FINALIZACION MOTIVO);

/**
 * @brief Función para revisar un paquete recibido por el kernel.
 *
 * @param paquete Paquete a revisar.
 * @param args Estructura de argumentos del kernel.
 * @param modulo Nombre del módulo que envía el paquete.
 */
void kernel_revisar_paquete(t_paquete *paquete, hilos_args *args, char *modulo);

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
 * @brief Inicia el hilo del planificador.
 *
 * @param args Los argumentos del hilo.
 */
void hilo_planificador_iniciar(hilos_args *args);

/**
 * @brief Cambia el estado del hilo del planificador.
 *
 * @param args Los argumentos del hilo.
 * @param estado El estado del hilo (true para activo, false para inactivo).
 */
void hilo_planificador_estado(hilos_args *args, bool estado);

/**
 * @brief Detiene el hilo del planificador.
 *
 * @param args Los argumentos del hilo.
 */
void hilo_planificador_detener(hilos_args *args);

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
 * @brief Registra el manejador de señales.
 */
void registrar_manejador_senales();

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

void hilos_ejecutar_entrada_salida(hilos_io_args *io_args, char *modulo, t_funcion_kernel_io_prt switch_case_atencion);
void switch_case_kernel_entrada_salida_generic(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_stdin(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_stdout(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void switch_case_kernel_entrada_salida_dialfs(hilos_io_args *io_args, char *modulo, t_op_code codigo_operacion, t_buffer *buffer);
void kernel_interrumpir_cpu(hilos_args *args, uint32_t pid, char *motivo);

void kernel_avisar_memoria_finalizacion_proceso(hilos_args *args, uint32_t pid);

/**
 * @brief Interrumpe las operaciones de E/S para un proceso específico en el kernel.
 *
 * Esta función interrumpe las operaciones de E/S para un proceso específico identificado por su PID.
 * La interrupción se realiza con una razón dada especificada por el parámetro `motivo`.
 *
 * @param args Un puntero a la estructura `hilos_args` que contiene los argumentos necesarios para el kernel.
 * @param pid El PID (Identificador de Proceso) del proceso para interrumpir sus operaciones de E/S.
 * @param motivo Una cadena que especifica la razón para interrumpir las operaciones de E/S.
 */
void kernel_interrumpir_io(hilos_args *args, uint32_t pid, char *motivo);

/**
 * Transiciona un bloque al estado de salida en el kernel.
 *
 * Esta función se encarga de transicionar un bloque al estado de salida en el kernel.
 *
 * @param kernel_hilos_args Los argumentos para los hilos del kernel.
 * @param pid El ID del proceso del bloque a transicionar.
 */
void kernel_transicion_block_exit(hilos_args *kernel_hilos_args, uint32_t pid);

/**
 * Busca una interfaz de entrada/salida del kernel basada en los argumentos del hilo y el ID del proceso.
 *
 * @param args Los argumentos del hilo.
 * @param pid El ID del proceso.
 * @return Un puntero a la interfaz t_kernel_entrada_salida encontrada, o NULL si no se encuentra.
 */
t_kernel_entrada_salida *kernel_entrada_salida_buscar_interfaz(hilos_args *args, uint32_t pid);

t_algoritmo determinar_algoritmo(hilos_args *args);

/**Virtual Round Robin**/
t_pcb *kernel_transicion_block_ready_mayor_prioridad(hilos_io_args *io_args, char *modulo, t_entrada_salida_kernel_unidad_de_trabajo *unidad);
t_pcb *kernel_transicion_exec_ready_mayor_prioridad(hilos_args *kernel_hilos_args);
t_pcb *kernel_transicion_ready_exec_mayor_prioridad(hilos_args *kernel_hilos_args);

#endif /* KERNEL_H */