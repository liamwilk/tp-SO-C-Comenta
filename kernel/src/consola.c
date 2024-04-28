#include "consola.h"
#include <utils/procesos.h>

operacion obtener_operacion(char *funcion)
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

void consola_iniciar(t_log *logger, t_kernel *kernel, diagrama_estados *estados, int *flag)
{
    char *linea;
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
            log_warning(logger, "Se ejecuto SCRIPT %s con argumento %s", separar_linea[0], separar_linea[1]);
            if (!separar_linea[1])
            {
                log_error(logger, "No se ingreso un path de instrucciones");
                break;
            }
            char *pathInstrucciones = separar_linea[1];
            kernel_nuevo_proceso(kernel, estados->new, logger, pathInstrucciones);
            break;
        case FINALIZAR_PROCESO:
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            break;
        case DETENER_PLANIFICACION:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            break;
        case INICIAR_PLANIFICACION:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            proceso_mover_ready(kernel->gradoMultiprogramacion, logger, estados);
            break;
        case MULTIPROGRAMACION:
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            int grado_multiprogramacion = atoi(separar_linea[1]);
            kernel->gradoMultiprogramacion = grado_multiprogramacion;
            log_info(logger, "GRADO_MULTIPROGRAMACION: %d", kernel->gradoMultiprogramacion);
            break;
        case PROCESO_ESTADO:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            break;
        case FINALIZAR:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            kernel_finalizar(kernel, flag);
            break;
        default:
            log_info(logger, "Comando no reconocido");
            break;
        }
        free(separar_linea);
        free(linea);
    };
};
