#include "procesos.h"

uint32_t pid = 0;
uint32_t new_pid()
{
    pid += 1;
    return pid;
};

t_pcb *pcb_crear(t_log *logger, int quantum)
{
    t_registros_cpu *registros_cpu = malloc(sizeof(t_registros_cpu));
    *registros_cpu = (t_registros_cpu){.pc = 0, .eax = 0, .ebx = 0, .ecx = 0, .ax = 0, .bx = 0, .cx = 0, .dx = 0};
    t_pcb *nuevo_pcb = malloc(sizeof(t_pcb));
    *nuevo_pcb = (t_pcb){.pid = new_pid(), .quantum = quantum, .registros_cpu = registros_cpu, .memoria_aceptado = false};
    return nuevo_pcb;
}

void proceso_agregar_new(t_new *new, t_pcb *pcb)
{
    list_add(new->cola, pcb);
    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);
    char *pos_char = string_itoa(list_size(new->cola) - 1);
    // No hace falta castearlo a void*, ese tipo acepta cualquier tipo de puntero (para el pos_char)
    dictionary_put(new->diccionario, pid_char, pos_char);
    free(pid_char);
    free(pos_char);
};

void proceso_agregar_ready(t_ready *ready, t_pcb *pcb)
{
    list_add(ready->cola, pcb);
    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);
    char *pos_char = string_itoa(list_size(ready->cola) - 1);
    dictionary_put(ready->diccionario, pid_char, pos_char);
    free(pid_char);
    free(pos_char);
};
t_pcb *proceso_quitar_new(t_new *new)
{
    t_pcb *elem = list_get(new->cola, 0);
    list_remove(new->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(new->diccionario, pid_char);
    free(pid_char);
    return elem;
};

t_pcb *proceso_quitar_ready(t_ready *ready)
{
    t_pcb *elem = list_get(ready->cola, 0);
    list_remove(ready->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(ready->diccionario, pid_char);
    free(pid_char);
    return elem;
};

void proceso_mover_ready(int gradoMultiprogramacion, t_log *logger, diagrama_estados *estados)
{

    int cant_en_new = list_size(estados->new->cola);
    int cant_en_ready = list_size(estados->ready->cola);

    if (cant_en_ready < gradoMultiprogramacion)
    {
        while (cant_en_ready < gradoMultiprogramacion && cant_en_new > 0)
        {
            t_pcb *proceso = proceso_quitar_new(estados->new);
            if (proceso->memoria_aceptado == false)
            {
                log_warning(logger, "No se puede mover el proceso PID: <%d> a ready, ya que no fue aceptado por memoria", proceso->pid);
                continue;
            }
            proceso_agregar_ready(estados->ready, proceso);
            cant_en_ready++;
            cant_en_new--;
            log_debug(logger, "Proceso PID: <%d> movido a ready", proceso->pid);
        }
    }
    if (cant_en_new == 0)
    {
        log_warning(logger, "No se pueden mover mas procesos a ready, ya que se  hay mas procesos en new");
    }
    else
    {
        log_warning(logger, "No se pueden mover mas procesos a ready, ya que se alcanzo el grado de multiprogramacion");
    }
};

t_pcb *proceso_buscar_new(t_new *new, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(new->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(new->cola, pos);
}

t_pcb *proceso_buscar_ready(t_ready *ready, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(ready->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(ready->cola, pos);
}

t_pcb *proceso_buscar_exec(t_exec *exec, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(exec->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(exec->cola, pos);
}

t_pcb *proceso_buscar_block(t_block *block, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(block->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(block->cola, pos);
}

t_pcb *proceso_buscar_exit(t_exit *exit, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(exit->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(exit->cola, pos);
}