#ifndef CONSOLA_H_
#define CONSOLA_H_
#include "string.h"
#include "readline/readline.h"
#include "readline/history.h"
#include "commons/log.h"
#include "utils/modulos.h"
#include "utils/serial.h"
#include "utils/conexiones.h"
#include "commons/string.h"
#include "utils/procesos.h"
#include "estados.h"

typedef enum
{
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    MULTIPROGRAMACION,
    PROCESO_ESTADO,
    FINALIZAR,
    NUM_FUNCIONES // siempre mantener este al final para saber el tamaño del enum
} operaciones;

operaciones obtener_operacion(char *funcion);

void procesar_consola(t_log *logger, int *pid, t_queue *new, int *flag, t_kernel *kernel, t_sockets_kernel *sockets);

#endif /* CONSOLA_H_ */
