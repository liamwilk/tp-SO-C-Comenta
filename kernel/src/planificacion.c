#include <planificacion.h>

void algoritmo_fifo(t_kernel *kernel, diagrama_estados *estados, int socket_dispatch)
{
    if (list_size(estados->ready->cola) > 0 && list_size(estados->exec->cola) == 0)
    {
        t_pcb *aux = proceso_quitar_ready(estados->ready);
        t_cpu_kernel_proceso *proceso = malloc(sizeof(t_cpu_kernel_proceso));
        proceso->pid = aux->pid;
        proceso->registros = malloc(sizeof(t_registros_cpu));
        proceso->registros->pc = aux->registros_cpu->pc;
        proceso->registros->eax = aux->registros_cpu->eax;
        proceso->registros->ebx = aux->registros_cpu->ebx;
        proceso->registros->ecx = aux->registros_cpu->ecx;
        proceso->registros->edx = aux->registros_cpu->edx;
        proceso->registros->si = aux->registros_cpu->si;
        proceso->registros->di = aux->registros_cpu->di;
        proceso->registros->ax = aux->registros_cpu->ax;
        proceso->registros->bx = aux->registros_cpu->bx;
        proceso->registros->cx = aux->registros_cpu->cx;
        proceso->registros->dx = aux->registros_cpu->dx;

        t_paquete *paquete = crear_paquete(KERNEL_CPU_EJECUTAR_PROCESO);

        serializar_t_registros_cpu(&paquete, proceso->pid, proceso->registros);
        enviar_paquete(paquete, socket_dispatch);
        eliminar_paquete(paquete);
        free(proceso);
        free(aux);
    }
    if (list_size(estados->block->cola) == 0)
    {
        return;
    }
}