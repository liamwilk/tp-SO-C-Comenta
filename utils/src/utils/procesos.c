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
    *registros_cpu = (t_registros_cpu){.pc = 0, .eax = 0, .ebx = 0, .ecx = 0, .edx = 0, .si = 0, .di = 0, .ax = 0, .bx = 0, .cx = 0, .dx = 0};
    t_pcb *nuevo_pcb = malloc(sizeof(t_pcb));
    *nuevo_pcb = (t_pcb){.pid = new_pid(), .quantum = quantum, .registros_cpu = registros_cpu, .memoria_aceptado = false};
    return nuevo_pcb;
};

void proceso_push_new(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->new->cola, pcb);
    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);
    char *pos_char = string_itoa(list_size(estados->new->cola) - 1);
    // No hace falta castearlo a void*, ese tipo acepta cualquier tipo de puntero (para el pos_char)
    dictionary_put(estados->new->diccionario, pid_char, pos_char);

    // Actualizo el diccionario de procesos
    char *estado = "NEW";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_ready(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->ready->cola, pcb);
    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);
    char *pos_char = string_itoa(list_size(estados->ready->cola) - 1);
    dictionary_put(estados->ready->diccionario, pid_char, pos_char);

    // Actualizo el diccionario de procesos
    char *estado = "READY";
    dictionary_put(estados->procesos, pid_char, estado);
};

t_pcb *proceso_pop_new(diagrama_estados *estados)
{
    t_pcb *elem = list_get(estados->new->cola, 0);
    list_remove(estados->new->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(estados->new->diccionario, pid_char);
    free(pid_char);
    return elem;
};

t_pcb *proceso_pop_ready(diagrama_estados *estados)
{
    t_pcb *elem = list_get(estados->ready->cola, 0);
    list_remove(estados->ready->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(estados->ready->diccionario, pid_char);
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
            t_pcb *proceso = proceso_pop_new(estados);
            if (proceso->memoria_aceptado == false)
            {
                log_warning(logger, "No se puede mover el proceso PID: <%d> a ready, ya que no fue aceptado por memoria", proceso->pid);
                continue;
            }
            proceso_push_ready(estados, proceso);
            cant_en_ready++;
            cant_en_new--;
            log_debug(logger, "Proceso PID: <%d> movido a ready", proceso->pid);
        }
    }
    if (cant_en_new == 0)
    {
        log_warning(logger, "No se pueden mover mas procesos a ready, ya que no  hay más procesos en new");
    }
    else
    {
        log_warning(logger, "No se pueden mover mas procesos a ready, ya que se alcanzo el grado de multiprogramacion");
    }
};

t_pcb *proceso_buscar_new(diagrama_estados *estados, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(estados->new->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(estados->new->cola, pos);
}

t_pcb *proceso_buscar_ready(diagrama_estados *estados, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(estados->ready->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(estados->ready->cola, pos);
}

t_pcb *proceso_buscar_exec(diagrama_estados *estados, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(estados->exec->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(estados->exec->cola, pos);
}

t_pcb *proceso_buscar_block(diagrama_estados *estados, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(estados->block->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(estados->block->cola, pos);
}

int proceso_eliminar_new(diagrama_estados *estados, uint32_t processPID)
{
    t_pcb *pcb = proceso_buscar_new(estados, processPID);
    if (pcb == NULL)
    {
        return 0;
    }
    char *pid_str = string_itoa(processPID);
    char *pos_str = dictionary_get(estados->new->diccionario, pid_str);
    int pos = atoi(pos_str);
    void *proceso = list_remove(estados->new->cola, pos);
    free(proceso);
    dictionary_remove_and_destroy(estados->new->diccionario, pid_str);
    pid -= 1;
    return 1;
}

t_pcb *proceso_buscar_exit(diagrama_estados *estados, int pid)
{
    // Se utiliza el diccionario para obtener la posición del proceso en la lista
    char *pid_str = string_itoa(pid);
    char *pos_str = dictionary_get(estados->exit->diccionario, pid_str);
    if (pos_str == NULL)
    {
        return NULL;
    }
    int pos = atoi(pos_str);
    return list_get(estados->exit->cola, pos);
}