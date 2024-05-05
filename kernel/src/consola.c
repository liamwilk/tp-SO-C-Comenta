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

t_pcb *buscar_proceso(diagrama_estados *estados, uint32_t pid)
{
    t_pcb *pcb = proceso_buscar_new(estados->new, pid);
    if (pcb == NULL)
    {
        pcb = proceso_buscar_ready(estados->ready, pid);
        if (pcb == NULL)
        {
            pcb = proceso_buscar_exec(estados->exec, pid);
            if (pcb == NULL)
            {
                pcb = proceso_buscar_block(estados->block, pid);
            }
        }
    }
    return pcb;
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
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            if (!separar_linea[1])
            {
                log_error(logger, "No se ingreso un path de instrucciones");
                break;
            }
            char *pathInstrucciones = separar_linea[1];
            kernel_nuevo_proceso(kernel, estados->new, logger, pathInstrucciones);
            break;

        case FINALIZAR_PROCESO:
        {
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            uint32_t pid = (uint32_t)atoi(separar_linea[1]);

            t_pcb *busqueda = buscar_proceso(estados, pid);
            if (busqueda == NULL)
            {
                log_error(logger, "El PID <%d> no existe", pid);
                break;
            }

            t_pcb *proceso_buscado = buscar_proceso(estados, pid);
            if (proceso_buscado == NULL)
            {
                log_error(logger, "El PID <%d> no existe", pid);
                break;
            }

            t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
            t_kernel_memoria_finalizar_proceso *proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
            proceso->pid = pid;
            serializar_t_kernel_memoria_finalizar_proceso(&paquete, proceso);
            enviar_paquete(paquete, kernel->sockets.memoria);

            eliminar_paquete(paquete);
            free(proceso);
            break;
        }
        case DETENER_PLANIFICACION:
        {
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);

            // Hardcodeo una prueba para IO Generic acá, luego hay que ubicarla donde vaya.

            if (kernel->sockets.entrada_salida_generic == 0)
            {
                log_error(logger, "No se pudo enviar el paquete a IO Generic porque aun no se conecto.");
                break;
            }

            log_debug(logger, "Se envia un sleep de 10 segundos a IO Generic");
            t_paquete *paquete = crear_paquete(KERNEL_ENTRADA_SALIDA_IO_GEN_SLEEP);

            t_kernel_entrada_salida_unidad_de_trabajo *unidad = malloc(sizeof(t_kernel_entrada_salida_unidad_de_trabajo));
            unidad->unidad_de_trabajo = 10;

            serializar_t_kernel_entrada_salida_unidad_de_trabajo(&paquete, unidad);

            log_debug(logger, "Socket IO Generic: %d\n", kernel->sockets.entrada_salida_generic);

            enviar_paquete(paquete, kernel->sockets.entrada_salida_generic);
            log_debug(logger, "Se envio el paquete a IO Generic");

            eliminar_paquete(paquete);
            free(unidad);

            break;
        }
        case INICIAR_PLANIFICACION:
        {
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            proceso_mover_ready(kernel->gradoMultiprogramacion, logger, estados);
            log_debug(logger, "Se movieron los procesos a READY");
            // TODO: Planificador a corto plazo (FIFO y RR)
            break;
        }
        case MULTIPROGRAMACION:
        {
            log_info(logger, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
            int grado_multiprogramacion = atoi(separar_linea[1]);
            kernel->gradoMultiprogramacion = grado_multiprogramacion;
            log_info(logger, "GRADO_MULTIPROGRAMACION: %d", kernel->gradoMultiprogramacion);
            break;
        }
        case PROCESO_ESTADO:
        {
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            break;
        }
        case FINALIZAR:
        {
            log_info(logger, "Se ejecuto script %s", separar_linea[0]);
            *flag = 0;
            kernel_finalizar(kernel);
            break;
        }
        default:
        {
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
        }
        free(separar_linea);
        free(linea);
    };
};
