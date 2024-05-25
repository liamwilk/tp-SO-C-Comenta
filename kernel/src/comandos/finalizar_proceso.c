#include <utils/kernel.h>

void finalizar_proceso(char **separar_linea, hilos_args *hiloArgs)
{
    if (separar_linea[1] == 0)
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "Falta proporcionar un numero de PID para eliminar. Vuelva a intentar.");
        return;
    }
    else
    {
        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se ejecuto script %s con argumento %s", separar_linea[0], separar_linea[1]);
        int pidReceived = atoi(separar_linea[1]);
        bool existe = kernel_finalizar_proceso(hiloArgs, pidReceived, INTERRUPTED_BY_USER);

        if (!existe)
        {
            kernel_log_generic(hiloArgs, LOG_LEVEL_ERROR, "El PID <%d> no existe", pidReceived);
            return;
        }

        t_paquete *paquete = crear_paquete(KERNEL_MEMORIA_FINALIZAR_PROCESO);
        t_kernel_memoria_finalizar_proceso *proceso = malloc(sizeof(t_kernel_memoria_finalizar_proceso));
        proceso->pid = pidReceived;
        serializar_t_kernel_memoria_finalizar_proceso(&paquete, proceso);
        enviar_paquete(paquete, hiloArgs->kernel->sockets.memoria);

        kernel_log_generic(hiloArgs, LOG_LEVEL_INFO, "Se elimino el proceso <%d>", pidReceived);
        eliminar_paquete(paquete);
        free(proceso);
        return;
    }
}