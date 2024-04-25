#ifndef PROCESOS_H_
#define PROCESOS_H_
#include <commons/log.h>
#include <stdlib.h>
#include <stdint.h>
#include <utils/modulos.h>
#include <utils/serial.h>

typedef struct t_registros_cpu
{
    uint32_t pc, eax, ebx, ecx;
    uint8_t ax, bx, cx, dx;
} t_registros_cpu;

typedef struct pcb
{
    int pid;
    int program_counter;
    int quantum;
    t_registros_cpu *registros_cpu;
} pcb;

typedef struct t_kernel_memoria
{
    char *pathInstrucciones;
    int program_counter;
    // Agregar a demanda el struct
} t_kernel_memoria;

pcb *crear_pcb(t_log *logger, int *pid, int quantum);

int new_pid(int *pid);
pcb *nuevo_proceso(t_kernel *kernel, int *pid, t_sockets_kernel *sockets, char *instrucciones, t_log *logger);
#endif /* PROCESOS_H_ */
