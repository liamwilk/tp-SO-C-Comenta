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

void proceso_push_exec(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->exec->cola, pcb);
    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);
    char *pos_char = string_itoa(list_size(estados->exec->cola) - 1);
    dictionary_put(estados->exec->diccionario, pid_char, pos_char);

    // Actualizo el diccionario de procesos
    char *estado = "EXEC";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_block(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->block->cola, pcb);
    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);
    char *pos_char = string_itoa(list_size(estados->block->cola) - 1);
    dictionary_put(estados->block->diccionario, pid_char, pos_char);

    // Actualizo el diccionario de procesos
    char *estado = "BLOCK";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_exit(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->exit->cola, pcb);
    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);
    char *pos_char = string_itoa(list_size(estados->exit->cola) - 1);
    dictionary_put(estados->exit->diccionario, pid_char, pos_char);

    // Actualizo el diccionario de procesos
    char *estado = "EXIT";
    dictionary_put(estados->procesos, pid_char, estado);
};

t_pcb *proceso_pop_new(diagrama_estados *estados)
{
    // Check if the list is empty
    if (list_is_empty(estados->new->cola))
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->new->cola, 0);
    list_remove(estados->new->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(estados->new->diccionario, pid_char);
    free(pid_char);
    return elem;
};

t_pcb *proceso_pop_ready(diagrama_estados *estados)
{
    // Check if the list is empty
    if (list_size(estados->ready->cola) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->ready->cola, 0);
    list_remove(estados->ready->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(estados->ready->diccionario, pid_char);
    free(pid_char);
    return elem;
};

t_pcb *proceso_pop_exec(diagrama_estados *estados)
{
    if (list_size(estados->exec->cola) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->exec->cola, 0);
    list_remove(estados->exec->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(estados->exec->diccionario, pid_char);
    free(pid_char);
    return elem;
};

t_pcb *proceso_pop_exit(diagrama_estados *estados)
{
    if (list_size(estados->exit->cola) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->exit->cola, 0);
    list_remove(estados->exit->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(estados->exit->diccionario, pid_char);
    free(pid_char);
    return elem;
};

t_pcb *proceso_pop_block(diagrama_estados *estados)
{
    if (list_size(estados->block->cola) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->block->cola, 0);
    list_remove(estados->block->cola, 0);
    char *pid_char = string_itoa(elem->pid);
    dictionary_remove(estados->block->diccionario, pid_char);
    free(pid_char);
    return elem;
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

int proceso_revertir(diagrama_estados *estados, uint32_t processPID)
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
    dictionary_remove(estados->new->diccionario, pid_str);
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

char *proceso_estado(diagrama_estados *estados, int pid)
{
    // Busca en estados->procesos
    char *pid_str = string_itoa(pid);
    char *estado = dictionary_get(estados->procesos, pid_str);
    if (estado == NULL)
    {
        return NULL;
    }
    return estado;
}

bool proceso_matar(diagrama_estados *estados, char *pid)
{
    // Obtener en que estado esta el proceso
    int pidNumber = atoi(pid);
    char *estado = proceso_estado(estados, pidNumber);
    if (estado == NULL)
    {
        return false;
    }
    if (strcmp(estado, "NEW") == 0)
    {
        char *pos_str = dictionary_get(estados->new->diccionario, pid);
        int pos = atoi(pos_str);
        list_remove_and_destroy_element(estados->new->cola, pos, free);
        dictionary_remove(estados->new->diccionario, pid);
        dictionary_remove(estados->procesos, pid);
    }
    else if (strcmp(estado, "READY") == 0)
    {
        char *pos_str = dictionary_get(estados->ready->diccionario, pid);
        int pos = atoi(pos_str);

        list_remove_and_destroy_element(estados->ready->cola, pos, free);
        dictionary_remove(estados->ready->diccionario, pid);
        dictionary_remove(estados->procesos, pid);
    }
    else if (strcmp(estado, "EXEC") == 0)
    {
        char *pos_str = dictionary_get(estados->exec->diccionario, pid);
        int pos = atoi(pos_str);
        list_remove_and_destroy_element(estados->exec->cola, pos, free);
        dictionary_remove_and_destroy(estados->exec->diccionario, pid, free);
        dictionary_remove_and_destroy(estados->procesos, pid, free);
    }
    else if (strcmp(estado, "BLOCK") == 0)
    {
        char *pos_str = dictionary_get(estados->block->diccionario, pid);
        int pos = atoi(pos_str);
        list_remove_and_destroy_element(estados->block->cola, pos, free);
        dictionary_remove(estados->block->diccionario, pid);
        dictionary_remove(estados->procesos, pid);
    }
    else if (strcmp(estado, "EXIT") == 0)
    {
        char *pos_str = dictionary_get(estados->exit->diccionario, pid);
        int pos = atoi(pos_str);
        list_remove_and_destroy_element(estados->exit->cola, pos, free);
        dictionary_remove(estados->exit->diccionario, pid);
        dictionary_remove(estados->procesos, pid);
    }
    else
    {
        return false;
    };
    return true;
}

t_pcb *proceso_transicion_ready_exec(diagrama_estados *estados)
{
    t_pcb *proceso = proceso_pop_ready(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_exec(estados, proceso);
    return proceso;
};

t_pcb *proceso_transicion_exec_block(diagrama_estados *estados)
{
    t_pcb *proceso = proceso_pop_exec(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_block(estados, proceso);
    return proceso;
};

t_pcb *proceso_transicion_block_ready(diagrama_estados *estados)
{
    t_pcb *proceso = proceso_pop_block(estados);
    if (proceso == NULL)
    {
        return NULL;
    }
    proceso_push_ready(estados, proceso);
    return proceso;
};