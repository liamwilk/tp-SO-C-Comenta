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
    printf("              _                 _ _ _   _____ _____\n");
    printf("             | |               | (_) | |  _  /  ___|\n");
    printf(" _ __ ___  __| | ___  _ __   __| |_| |_| | | \\ `--. \n");
    printf("| '__/ _ \\/ _` |/ _ \\| '_ \\ / _` | | __| | | |`--. \\\n");
    printf("| | |  __/ (_| | (_) | | | | (_| | | |_\\ \\_/ /\\__/ /\n");
    printf("|_|  \\___|\\__,_|\\___/|_| |_|\\__,_|_|\\__|\\___/\\____/ \n");
    printf("\n");
    printf("---------------------------------------------------------------\n");
    printf("Implementación de C-Comenta - www.faq.utnso.com.ar/tp-c-comenta\n");
    printf("Sistemas Operativos - 1C 2024 - UTN FRBA\n");
    printf("---------------------------------------------------------------\n");
    printf("\n");
    printf("Comandos disponibles: \n");
    printf("└─ EJECUTAR_SCRIPT <path>\n");
    printf("└─ INICIAR_PROCESO <path>\n");
    printf("└─ FINALIZAR_PROCESO <PID>\n");
    printf("└─ DETENER_PLANIFICACION\n");
    printf("└─ INICIAR_PLANIFICACION\n");
    printf("└─ MULTIPROGRAMACION <grado>\n");
    printf("└─ PROCESO_ESTADO <PID>\n");
    printf("└─ FINALIZAR\n");
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
            else
            {
                char *pathInstrucciones = separar_linea[1];
                kernel_nuevo_proceso(kernel, estados->new, logger, pathInstrucciones);
                break;
            }

        case FINALIZAR_PROCESO:
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            break;
        case DETENER_PLANIFICACION:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            break;
        case INICIAR_PLANIFICACION:
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            proceso_mover_ready(kernel->gradoMultiprogramacion, logger, estados);
            log_debug(logger, "Se movieron los procesos a READY");
            // Planificador a corto plazo (fifo y round robin)
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
            *flag = 0;
            t_paquete *finalizar = crear_paquete(TERMINAR);
            enviar_paquete(finalizar, kernel->sockets.cpu_dispatch);
            enviar_paquete(finalizar, kernel->sockets.memoria);
            liberar_conexion(kernel->sockets.cpu_dispatch);
            liberar_conexion(kernel->sockets.cpu_interrupt);
            liberar_conexion(kernel->sockets.memoria);
            liberar_conexion(kernel->sockets.server);
            eliminar_paquete(finalizar);
            break;
        default:
            printf("\nEl comando ingresado no es válido, por favor, intente nuevamente: \n");
            printf("└─ EJECUTAR_SCRIPT <path>\n");
            printf("└─ INICIAR_PROCESO <path>\n");
            printf("└─ FINALIZAR_PROCESO <PID>\n");
            printf("└─ DETENER_PLANIFICACION\n");
            printf("└─ INICIAR_PLANIFICACION\n");
            printf("└─ MULTIPROGRAMACION <grado>\n");
            printf("└─ PROCESO_ESTADO <PID>\n");
            printf("└─ FINALIZAR\n");
            break;
        }
        free(separar_linea);
        free(linea);
    };
};
