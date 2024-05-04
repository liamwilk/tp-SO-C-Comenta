#ifndef HILOS_H_
#define HILOS_H_
#include "consola.h"
#include "commons/log.h"
#include "pthread.h"
#include "utils/handshake.h"
#include <utils/serial.h>
#include <utils/handshake.h>
#include <utils/conexiones.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <limits.h>

typedef struct hilos_args
{
    t_log *logger;
    t_kernel *kernel;
    diagrama_estados *estados;
    int *kernel_orden_apagado;
} hilos_args;

typedef struct hilos_io_args
{
    hilos_args *args;
    int socket;
} hilos_io_args;

/*----ATENDER----*/

void *hilos_atender_consola(void *args);
void *hilos_atender_memoria(void *args);
void *hilos_atender_cpu_dispatch(void *args);
void *hilos_atender_cpu_interrupt(void *args);

void *hilos_atender_entrada_salida_generic(void *args);
void *hilos_atender_entrada_salida_stdin(void *args);
void *hilos_atender_entrada_salida_stdout(void *args);
void *hilos_atender_entrada_salida_dialfs(void *args);

/*----CONECTAR----*/
void *hilos_conectar_memoria(void *args);
void *hilos_conectar_cpu_dispatch(void *args);
void *hilos_conectar_cpu_interrupt(void *args);
void *hilos_esperar_entrada_salida(void *args);

void hilos_memoria_inicializar(hilos_args *args, pthread_t thread_conectar_memoria, pthread_t thread_atender_memoria);
void hilos_cpu_inicializar(hilos_args *args, pthread_t thread_conectar_cpu_dispatch, pthread_t thread_atender_cpu_dispatch, pthread_t thread_conectar_cpu_interrupt, pthread_t thread_atender_cpu_interrupt);
void hilos_io_inicializar(hilos_args *args, pthread_t thread_esperar_entrada_salida);
void hilos_consola_inicializar(hilos_args *args, pthread_t thread_atender_consola);

#endif /* HILOS_H_ */