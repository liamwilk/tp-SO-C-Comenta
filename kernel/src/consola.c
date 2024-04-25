#include "consola.h"
#include <utils/procesos.h>

operaciones obtener_operacion(char *funcion)
{
    char *operacionesStrings[] = {
        "EJECUTAR_SCRIPT",
        "INICIAR_PROCESO",
        "FINALIZAR_PROCESO",
        "DETENER_PLANIFICACION",
        "INICIAR_PLANIFICACION",
        "MULTIPROGRAMACION",
        "PROCESO_ESTADO",
        "FINALIZAR"};
    for (int i = 0; i < NUM_FUNCIONES; i++)
    {
        if (strcmp(operacionesStrings[i], funcion) == 0)
        {
            return i;
        }
    }
    return NUM_FUNCIONES;
}

void procesar_consola(t_log *logger, int *pid, t_queue *new, int *flag, t_kernel *kernel, t_sockets_kernel *sockets)
{
    char *linea;
    t_paquete *finalizar = crear_paquete(TERMINAR);
    while (*flag)
    {
        linea = readline("\n> ");
        if (!linea)
        {
            *flag = 0;
            break;
        }
        add_history(linea);
        char **separar_linea = string_split(linea, " ");
        switch (obtener_operacion(separar_linea[0]))
        {
        case EJECUTAR_SCRIPT:
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            break;
        case INICIAR_PROCESO:
            log_info(logger, "Se ejecuto SCRIPT %s con argumento %s", separar_linea[0], separar_linea[1]);
            char *pathInstrucciones = separar_linea[1];
            pcb *nuevaPcb = nuevo_proceso(kernel, pid, sockets, pathInstrucciones, logger);
            push_new(new, nuevaPcb);
            // Planificar procesos
            break;
        case FINALIZAR_PROCESO:
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            break;
        case DETENER_PLANIFICACION:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            break;
        case INICIAR_PLANIFICACION:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            break;
        case MULTIPROGRAMACION:
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            break;
        case PROCESO_ESTADO:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            break;
        case FINALIZAR:
            *flag = 0;
            enviar_paquete(finalizar, sockets->cpu_dispatch);
            enviar_paquete(finalizar, sockets->memoria);
            liberar_conexion(sockets->cpu_dispatch);
            liberar_conexion(sockets->cpu_interrupt);
            liberar_conexion(sockets->memoria);
            liberar_conexion(sockets->server);
            break;
        default:
            log_info(logger, "Comando no reconocido");
            break;
        }
        free(separar_linea);
        free(linea);
    };
};
