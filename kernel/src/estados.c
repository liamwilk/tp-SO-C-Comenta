#include "estados.h"

void iniciar_estados(t_queue *new, t_queue *ready)
{
    new = queue_create();
    ready = queue_create();
}

void push_new(t_queue *new, pcb *pcb)
{
    queue_push(new, pcb);
}

void pop_new(t_queue *new)
{
    queue_pop(new);
}

void push_ready(t_queue *ready, pcb *pcb)
{
    queue_push(ready, pcb);
}

void pop_ready(t_queue *ready)
{
    queue_pop(ready);
}