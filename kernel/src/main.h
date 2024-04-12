#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <utils/hilos.h>
#include <utils/modulos.h>
#include <utils/configs.h>
#include <utils/conexiones.h>

// Manejo de envios
typedef enum
{
	MENSAJE,
	PAQUETE
} op_code;

typedef struct
{
	int size;
	void *stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer *buffer;
} t_paquete;

// Estructuras de configuracion y log
t_log *logger;
t_config *config;

// Config en memoria
int puertoEscucha, puertoMemoria, puertoCpuDispatch, puertoCpuInterrupt, quantum, gradoMultiprogramacion;
char *ipMemoria;
char *ipCpu;
char *algoritmoPlanificador;
char *recursos;
char *instanciasRecursos;

// Threads
pthread_t cpuInterruptThread;
pthread_t cpuDispatchThread;
pthread_t memoriatThread;

// Sockets
int socketMemoria;
int socketCpuDispatch;
int socketCpuInterrupt;
int serverKernel;

// Funciones
t_log *iniciar_logger(char *);
t_config *iniciar_config(t_log *logger);
void terminar_programa(int, t_log *, t_config *);
void *serializar_paquete(t_paquete *, int);
void enviar_mensaje(char *, int);
void crear_buffer(t_paquete *);
void agregar_a_paquete(t_paquete *, void *, int);
void enviar_paquete(t_paquete *, int);
void eliminar_paquete(t_paquete *);
void liberar_conexion(int);
int recibir_operacion(int);
void recibir_mensaje(t_log *, int);
void *recibir_buffer(int *, int);
uint32_t handshake(t_log *, int, uint32_t, char *);

#endif /* MAIN_H_ */