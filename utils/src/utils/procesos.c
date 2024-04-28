#include "procesos.h"

int pid = 0;
int new_pid()
{
    pid += 1;
    return pid;
};

t_pcb *pcb_crear(t_log *logger, int quantum)
{
    t_registros_cpu *registros_cpu = malloc(sizeof(t_registros_cpu));
    *registros_cpu = (t_registros_cpu){.pc = 0, .eax = 0, .ebx = 0, .ecx = 0, .ax = 0, .bx = 0, .cx = 0, .dx = 0};
    t_pcb *nuevo_pcb = malloc(sizeof(t_pcb));
    *nuevo_pcb = (t_pcb){.pid = new_pid(), .program_counter = 0, .quantum = quantum, .registros_cpu = registros_cpu};
    return nuevo_pcb;
}

void proceso_agregar_new(t_queue *new, t_pcb *pcb) { queue_push(new, pcb); };
void proceso_agregar_ready(t_queue *ready, t_pcb *pcb) { queue_push(ready, pcb); };
void proceso_quitar_new(t_queue *new) { queue_pop(new); };
void proceso_quitar_ready(t_queue *ready) { queue_pop(ready); };

void proceso_mover_ready(int gradoMultiprogramacion, t_log *logger, diagrama_estados *estados)
{

    int cant_en_new = queue_size(estados->new);
    int cant_en_ready = queue_size(estados->ready);

    if (cant_en_ready < gradoMultiprogramacion)
    {
        while (cant_en_ready < gradoMultiprogramacion && cant_en_new > 0)
        {
            t_pcb *proceso = queue_pop(estados->new);
            proceso_agregar_ready(estados->ready, proceso);
            cant_en_ready++;
            cant_en_new--;
            log_info(logger, "Proceso %d movido a ready", proceso->pid);
        }
    }
    if (cant_en_new == 0)
    {
        log_warning(logger, "No se pueden mover mas procesos a ready, ya que se no hay mas procesos en new");
    }
    else
    {
        log_warning(logger, "No se pueden mover mas procesos a ready, ya que se alcanzo el grado de multiprogramacion");
    }
};
