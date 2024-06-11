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
    *nuevo_pcb = (t_pcb){.pid = new_pid(), .quantum = quantum, .registros_cpu = registros_cpu, .memoria_aceptado = false, .sleeping_thread = PTHREAD_CREATE_JOINABLE};
    return nuevo_pcb;
};

void proceso_push_new(t_diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->new, pcb);
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "NEW";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_ready(t_diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->ready, pcb);
    char *pid_char = string_itoa(pcb->pid);

    char *estado = "READY";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_exec(t_diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->exec, pcb);
    char *pid_char = string_itoa(pcb->pid);

    char *estado = "EXEC";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_block(t_diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->block, pcb);
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "BLOCK";
    dictionary_put(estados->procesos, pid_char, estado);
};

void proceso_push_exit(t_diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->exit, pcb);

    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "EXIT";
    dictionary_put(estados->procesos, pid_char, estado);
};

t_pcb *proceso_pop_new(t_diagrama_estados *estados)
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

t_pcb *proceso_pop_ready(t_diagrama_estados *estados)
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

t_pcb *proceso_pop_exec(t_diagrama_estados *estados)
{
    if (list_size(estados->exec) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->exec, 0);
    list_remove(estados->exec, 0);
    return elem;
};

t_pcb *proceso_pop_exit(t_diagrama_estados *estados)
{
    if (list_size(estados->exit) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->exit, 0);
    list_remove(estados->exit, 0);
    return elem;
};

t_pcb *proceso_remover_block(t_diagrama_estados *estados, uint32_t pid)
{
    if (list_size(estados->block) == 0)
    {
        return NULL;
    }
    for (int i = 0; i < list_size(estados->block); i++)
    {
        t_pcb *proceso = list_get(estados->block, i);
        if (proceso->pid == pid)
        {
            list_remove(estados->block, i);
            return proceso;
        }
    }
    return NULL;
};

t_pcb *proceso_buscar_new(t_diagrama_estados *estados, int pid)
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

t_pcb *proceso_buscar_ready(t_diagrama_estados *estados, int pid)
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
    // Buscamos en la cola de ready mayor prioridad
    for (int i = 0; i < list_size(estados->ready_mayor_prioridad); i++)
    {
        t_pcb *proceso = list_get(estados->ready_mayor_prioridad, i);
        if (proceso->pid == pid)
        {
            return proceso;
        }
    }
    return NULL;
}

t_pcb *proceso_buscar_exec(t_diagrama_estados *estados, int pid)
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

t_pcb *proceso_buscar_block(t_diagrama_estados *estados, int pid)
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

int proceso_revertir(t_diagrama_estados *estados, uint32_t processPID)
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

t_pcb *proceso_buscar_exit(t_diagrama_estados *estados, int pid)
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

char *proceso_estado(t_diagrama_estados *estados, int pid)
{
    char *pid_str = string_itoa(pid);
    char *estado = dictionary_get(estados->procesos, pid_str);
    if (estado == NULL)
    {
        return NULL;
    }
    return estado;
}

void proceso_matar(t_diagrama_estados *estados, char *pid)
{
    /**LIBERA LA MEMORIA DE ESE PROCESO EN KERNEL**/

    char *estado = proceso_estado(estados, atoi(pid));
    t_list *cola = proceso_obtener_estado(estados, estado);
    int pidNumber = atoi(pid);
    for (int i = 0; i < list_size(cola); i++)
    {
        t_pcb *proceso = list_get(cola, i);
        if (proceso->pid == pidNumber)
        {
            list_remove_and_destroy_element(cola, i, free);

            /**SE AGREGA REFERENCIA A ESTADO EXIT DEL PROCESO EN DICCIONARIO**/
            dictionary_put(estados->procesos, pid, "EXIT");
            return;
        }
    }
}

t_list *proceso_obtener_estado(t_diagrama_estados *estados, char *estado)
{
    if (strcmp(estado, "NEW") == 0)
    {
        return estados->new;
    }
    else if (strcmp(estado, "READY") == 0)
    {
        return estados->ready;
    }
    else if (strcmp(estado, "EXEC") == 0)
    {
        return estados->exec;
    }
    else if (strcmp(estado, "BLOCK") == 0)
    {
        return estados->block;
    }
    else if (strcmp(estado, "EXIT") == 0)
    {
        return estados->exit;
    }
    else
    {
        return NULL;
    }
};

void proceso_push_cola_prioritaria(t_diagrama_estados *estados, t_pcb *pcb)
{
    list_add(estados->ready_mayor_prioridad, pcb);

    // Añado el proceso al diccionario de procesos, mapeando el PID a la posición en la lista de procesos
    char *pid_char = string_itoa(pcb->pid);

    // Actualizo el diccionario de procesos
    char *estado = "READY_PRIORIDAD";
    dictionary_put(estados->procesos, pid_char, estado);
}

t_pcb *proceso_pop_cola_prioritaria(t_diagrama_estados *estados)
{
    if (list_size(estados->ready_mayor_prioridad) == 0)
    {
        return NULL;
    }
    t_pcb *elem = list_get(estados->ready_mayor_prioridad, 0);
    list_remove(estados->ready_mayor_prioridad, 0);
    return elem;
}

void proceso_actualizar_registros(t_pcb *pcb, t_registros_cpu registros_cpu)
{
    pcb->registros_cpu->pc = registros_cpu.pc;
    pcb->registros_cpu->eax = registros_cpu.eax;
    pcb->registros_cpu->ebx = registros_cpu.ebx;
    pcb->registros_cpu->ecx = registros_cpu.ecx;
    pcb->registros_cpu->edx = registros_cpu.edx;
    pcb->registros_cpu->si = registros_cpu.si;
    pcb->registros_cpu->di = registros_cpu.di;
    pcb->registros_cpu->ax = registros_cpu.ax;
    pcb->registros_cpu->bx = registros_cpu.bx;
    pcb->registros_cpu->cx = registros_cpu.cx;
    pcb->registros_cpu->dx = registros_cpu.dx;
}

t_algoritmo determinar_algoritmo(char *algoritmoPlanificador)
{
    if (strcmp(algoritmoPlanificador, "FIFO") == 0)
    {
        return FIFO;
    }
    else if (strcmp(algoritmoPlanificador, "RR") == 0)
    {
        return RR;
    }
    else if (strcmp(algoritmoPlanificador, "VRR") == 0)
    {
        return VRR;
    }
    return -1;
}

bool proceso_tiene_prioridad(char *algoritmoPlanificador, int quantum_restante)
{
    t_algoritmo ALGORITMO = determinar_algoritmo(algoritmoPlanificador);
    // !! Se establece un offset de 100 milisegundos o más para considerar que un proceso tiene prioridad
    if (ALGORITMO == VRR && quantum_restante > 100)
    {
        return true;
    }
    else
    {
        return false;
    }
}

t_pcb *proceso_buscar(t_diagrama_estados *estados, uint32_t pid)
{
    t_pcb *proceso = proceso_buscar_new(estados, pid);
    if (proceso != NULL)
    {
        return proceso;
    }
    proceso = proceso_buscar_ready(estados, pid);
    if (proceso != NULL)
    {
        return proceso;
    }
    proceso = proceso_buscar_exec(estados, pid);
    if (proceso != NULL)
    {
        return proceso;
    }
    proceso = proceso_buscar_block(estados, pid);
    if (proceso != NULL)
    {
        return proceso;
    }
    proceso = proceso_buscar_exit(estados, pid);
    if (proceso != NULL)
    {
        return proceso;
    }
    return NULL;
}
