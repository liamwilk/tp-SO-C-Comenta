#ifndef HILOS_H_
#define HILOS_H_
#include <consola.h>
#include <commons/log.h>
#include <pthread.h>
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
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <utils/modulos.h>
#include <utils/configs.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "planificacion.h"
#include <utils/template.h>
#include "modulos/cpu_dispatch.h"
#include "modulos/cpu_interrupt.h"
#include "modulos/memoria.h"

typedef enum
{
    FIFO,
    RR,
    VRR
} t_algoritmo;

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
void *hilo_planificador(void *args);
t_algoritmo determinar_algoritmo(hilos_args *args);
void hilo_planificador_detener(hilos_args *hiloArgs);
void hilo_planificador_iniciar(hilos_args *hiloArgs);

void hilos_memoria_inicializar(hilos_args *args, pthread_t thread_conectar_memoria, pthread_t thread_atender_memoria);
void hilos_cpu_inicializar(hilos_args *args, pthread_t thread_conectar_cpu_dispatch, pthread_t thread_atender_cpu_dispatch, pthread_t thread_conectar_cpu_interrupt, pthread_t thread_atender_cpu_interrupt);
void hilos_io_inicializar(hilos_args *args, pthread_t thread_esperar_entrada_salida);
void hilos_consola_inicializar(hilos_args *args, pthread_t thread_atender_consola);
void hilos_planificador_inicializar(hilos_args *args, pthread_t thread_planificador);

int obtener_key_finalizacion_hilo(hilos_args *args);
int obtener_key_detencion_algoritmo(hilos_args *args);
#endif /* HILOS_H_ */