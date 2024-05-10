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
#include <utils/kernel.h>
typedef enum
{
    PROCESO_ESTADO,
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    MULTIPROGRAMACION,
    FINALIZAR_PROCESO,
    FINALIZAR_CONSOLA,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    TOPE_ENUM_CONSOLA // siempre mantener este al final para saber el tamaño del enum
} t_consola_operacion;

t_consola_operacion obtener_operacion(char *funcion);
void imprimir_comandos();
void imprimir_logo();
void imprimir_header();
t_pcb *buscar_proceso(t_diagrama_estados *estados, uint32_t pid);

#endif /* CONSOLA_H_ */
