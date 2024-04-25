#include "procesos.h"

int new_pid(int *pid)
{
    *pid = *pid + 1;
    return *pid;
};

pcb *crear_pcb(t_log *logger, int *pid, int quantum)
{
    log_info(logger, "Creando nueva pcb...");
    pcb *nuevo_pcb = malloc(sizeof(pcb));
    nuevo_pcb->pid = new_pid(pid);
    nuevo_pcb->program_counter = 0;
    nuevo_pcb->quantum = quantum;
    t_registros_cpu *registros_cpu = malloc(sizeof(t_registros_cpu));
    registros_cpu->pc = 0;
    registros_cpu->eax = 0;
    registros_cpu->ebx = 0;
    registros_cpu->ecx = 0;
    registros_cpu->ax = 0;
    registros_cpu->bx = 0;
    registros_cpu->cx = 0;
    registros_cpu->dx = 0;
    nuevo_pcb->registros_cpu = registros_cpu;
    return nuevo_pcb;
};

pcb *nuevo_proceso(t_kernel *kernel, int *pid, t_sockets_kernel *sockets, char *instrucciones, t_log *logger)
{
    pcb *nuevaPcb = crear_pcb(logger, pid, kernel->quantum);
    t_kernel_memoria *kernel_memoria = malloc(sizeof(t_kernel_memoria));
    kernel_memoria->pathInstrucciones = instrucciones;
    kernel_memoria->program_counter = nuevaPcb->program_counter;
    t_paquete *paquete = crear_paquete(PAQUETE);
    agregar_a_paquete(paquete, &kernel_memoria->pathInstrucciones, sizeof(char *));
    agregar_a_paquete(paquete, &kernel_memoria->program_counter, sizeof(int));
    enviar_paquete(paquete, sockets->memoria);
    log_info(logger, "Se crea el proceso  <%d> en NEW", nuevaPcb->pid);
    return nuevaPcb;
};
