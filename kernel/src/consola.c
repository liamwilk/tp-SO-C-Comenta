#include "consola.h"
#include <utils/procesos.h>

t_consola_operacion obtener_operacion(char *funcion)
{
    char *operacionesStrings[] = {
        "PROCESO_ESTADO",
        "EJECUTAR_SCRIPT",
        "INICIAR_PROCESO",
        "MULTIPROGRAMACION",
        "FINALIZAR_PROCESO",
        "FINALIZAR",
        "DETENER_PLANIFICACION",
        "INICIAR_PLANIFICACION",
    };
    for (int i = 0; i < TOPE_ENUM_CONSOLA; i++)
    {
        if (strcmp(operacionesStrings[i], funcion) == 0)
        {
            return i;
        }
    }
    return TOPE_ENUM_CONSOLA;
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

void imprimir_logo()
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
}

void imprimir_comandos()
{
    printf("└─ EJECUTAR_SCRIPT <path>\n");
    printf("└─ INICIAR_PROCESO <path>\n");
    printf("└─ FINALIZAR_PROCESO <PID>\n");
    printf("└─ DETENER_PLANIFICACION\n");
    printf("└─ INICIAR_PLANIFICACION\n");
    printf("└─ MULTIPROGRAMACION <grado>\n");
    printf("└─ PROCESO_ESTADO <PID>\n");
    printf("└─ FINALIZAR\n");
}

void imprimir_header()
{
    imprimir_logo();
    printf("Comandos disponibles: \n");
    imprimir_comandos();
}