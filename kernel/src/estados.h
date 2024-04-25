#ifndef ESTADOS_H_
#define ESTADOS_H_
#include "string.h"
#include "readline/readline.h"
#include "readline/history.h"
#include "commons/log.h"
#include "utils/modulos.h"
#include "utils/serial.h"
#include "utils/conexiones.h"
#include "commons/string.h"
#include "utils/procesos.h"
#include "commons/collections/queue.h"

void iniciar_estados(t_queue *new, t_queue *ready);

// Va agregar a new la pcb
void push_new(t_queue *new, pcb *pcb);
void pop_new(t_queue *new);

// Manejo de cola de ready
void push_ready(t_queue *ready, pcb *pcb);
void pop_ready(t_queue *ready);

#endif /* ESTADOS_H_ */
