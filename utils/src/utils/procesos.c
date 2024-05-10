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
    list_add(estados->new, pcb);
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "NEW";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_ready(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->ready, pcb);
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "READY";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_exec(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->exec, pcb);
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "EXEC";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_block(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->block, pcb);
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "BLOCK";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_exit(diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->exit, pcb);

    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "EXIT";
    dictionary_put(estados->procesos, pid_char, estado);
};

t_pcb *proceso_pop_new(diagrama_estados *estados)
{
    // Check if the list is empty
    if (list_is_empty(estados->new))
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->new, 0);
    list_remove(estados->new, 0);
    return elem;
};

t_pcb *proceso_pop_ready(diagrama_estados *estados)
{
    // Check if the list is empty
    if (list_size(estados->ready) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->ready, 0);
    list_remove(estados->ready, 0);
    return elem;
};

t_pcb *proceso_pop_exec(diagrama_estados *estados)
{
    if (list_size(estados->exec) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->exec, 0);
    list_remove(estados->exec, 0);
    return elem;
};

t_pcb *proceso_pop_exit(diagrama_estados *estados)
{
    if (list_size(estados->exit) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->exit, 0);
    list_remove(estados->exit, 0);
    return elem;
};

t_pcb *proceso_pop_block(diagrama_estados *estados)
{
    if (list_size(estados->block) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->block, 0);
    list_remove(estados->block, 0);
    return elem;
};

t_pcb *proceso_buscar_new(diagrama_estados *estados, int pid)
{
    // Recorrer la lista hasta que se encuentre el pid
    for (int i = 0; i < list_size(estados->new); i++)
    {
        t_pcb *proceso = list_get(estados->new, i);
        if (proceso->pid == pid)
        {
            return proceso;
        }
    }
    return NULL;
}

t_pcb *proceso_buscar_ready(diagrama_estados *estados, int pid)
{
    // Recorrer la lista hasta que se encuentre el pid
    for (int i = 0; i < list_size(estados->ready); i++)
    {
        t_pcb *proceso = list_get(estados->ready, i);
        if (proceso->pid == pid)
        {
            return proceso;
        }
    }
    return NULL;
}

t_pcb *proceso_buscar_exec(diagrama_estados *estados, int pid)
{
    // Recorrer la lista hasta que se encuentre el pid
    for (int i = 0; i < list_size(estados->exec); i++)
    {
        t_pcb *proceso = list_get(estados->exec, i);
        if (proceso->pid == pid)
        {
            return proceso;
        }
    }
    return NULL;
}

t_pcb *proceso_buscar_block(diagrama_estados *estados, int pid)
{
    // Recorrer la lista hasta que se encuentre el pid
    for (int i = 0; i < list_size(estados->block); i++)
    {
        t_pcb *proceso = list_get(estados->block, i);
        if (proceso->pid == pid)
        {
            return proceso;
        }
    }
    return NULL;
}

int proceso_revertir(diagrama_estados *estados, uint32_t processPID)
{
    // Buscar posicion del proceso en la lista
    int pos = 0;
    for (int i = 0; i < list_size(estados->new); i++)
    {
        t_pcb *proceso = list_get(estados->new, i);
        if (proceso->pid == processPID)
        {
            pos = i;
            break;
        }
    }
    list_remove_and_destroy_element(estados->new, pos, free);
    dictionary_remove(estados->procesos, string_itoa(processPID));
    pid -= 1;
    return 1;
}

t_pcb *proceso_buscar_exit(diagrama_estados *estados, int pid)
{
    // Recorrer la lista hasta que se encuentre el pid
    for (int i = 0; i < list_size(estados->exit); i++)
    {
        t_pcb *proceso = list_get(estados->exit, i);
        if (proceso->pid == pid)
        {
            return proceso;
        }
    }
    return NULL;
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
        // Buscar en la lista de new
        for (int i = 0; i < list_size(estados->new); i++)
        {
            t_pcb *proceso = list_get(estados->new, i);
            if (proceso->pid == pidNumber)
            {
                list_remove_and_destroy_element(estados->new, i, free);
                dictionary_remove(estados->procesos, pid);
                return true;
            }
        }
    }
    else if (strcmp(estado, "READY") == 0)
    {
        for (int i = 0; i < list_size(estados->ready); i++)
        {
            t_pcb *proceso = list_get(estados->ready, i);
            if (proceso->pid == pidNumber)
            {
                list_remove_and_destroy_element(estados->ready, i, free);
                dictionary_remove(estados->procesos, pid);
                return true;
            }
        }
    }
    else if (strcmp(estado, "EXEC") == 0)
    {
        for (int i = 0; i < list_size(estados->exec); i++)
        {
            t_pcb *proceso = list_get(estados->exec, i);
            if (proceso->pid == pidNumber)
            {
                list_remove_and_destroy_element(estados->exec, i, free);
                dictionary_remove(estados->procesos, pid);
                return true;
            }
        }
    }
    else if (strcmp(estado, "BLOCK") == 0)
    {
        for (int i = 0; i < list_size(estados->block); i++)
        {
            t_pcb *proceso = list_get(estados->block, i);
            if (proceso->pid == pidNumber)
            {
                list_remove_and_destroy_element(estados->block, i, free);
                dictionary_remove(estados->procesos, pid);
                return true;
            }
        }
    }
    else if (strcmp(estado, "EXIT") == 0)
    {
        for (int i = 0; i < list_size(estados->exit); i++)
        {
            t_pcb *proceso = list_get(estados->exit, i);
            if (proceso->pid == pidNumber)
            {
                list_remove_and_destroy_element(estados->exit, i, free);
                dictionary_remove(estados->procesos, pid);
                return true;
            }
        }
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