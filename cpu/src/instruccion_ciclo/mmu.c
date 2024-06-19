#include <utils/cpu.h>

void *hilo_mmu(void *args_void)
{
    t_cpu *args = (t_cpu *)args_void;

    while (1)
    {
        // Espero a que me avisen que hay una direccion logica para traducir
        sem_wait(&args->mmu_ejecucion);

        pthread_testcancel();

        args->pagina = calcular_numero_pagina(args, args->direccion_logica);

        int desplazamiento = args->direccion_logica - (args->pagina * args->tam_pagina);

        // Busco en la TLB si ya fue traducida esa direccion logica
        args->marco = buscar_en_tlb(args->proceso.pid, args->pagina, args->config_leida.cantidadEntradasTlb, args->tlb);

        // Si la direccion logica ya fue traducida

        if (args->marco != -1)
        {
            log_info(args->logger, "PID: <%d> - TLB HIT - Pagina: <%d>", args->proceso.pid, args->pagina);

            args->direccion_fisica = (args->marco * args->tam_pagina) + desplazamiento;

            args->resultado = 1;
        }
        else
        {
            // Si la direccion logica no fue traducida, pido el frame asociado a ese numero de pagina a Memoria
            log_info(args->logger, "PID: <%d> - TLB MISS - Pagina: <%d>", args->proceso.pid, args->pagina);

            t_paquete *paquete = crear_paquete(CPU_MEMORIA_NUMERO_FRAME);

            t_cpu_memoria_numero_marco *numero_frame = malloc(sizeof(t_cpu_memoria_numero_marco));

            numero_frame->pid = args->proceso.pid;
            numero_frame->numero_pagina = args->pagina;

            serializar_t_cpu_memoria_numero_marco(&paquete, numero_frame);

            enviar_paquete(paquete, args->config_leida.socket_memoria);

            eliminar_paquete(paquete);

            // Espero a que memoria me devuelva el numero de frame
            sem_wait(&args->mmu_ejecucion);

            if (args->resultado)
            {
                // En este punto, ya tengo el numero de frame cargado en TLB, asi que lo busco y lo guardo en la estructura
                args->marco = buscar_en_tlb(args->proceso.pid, args->pagina, args->config_leida.cantidadEntradasTlb, args->tlb);

                // Calculo la direccion fisica
                args->direccion_fisica = (args->marco * args->tam_pagina) + desplazamiento;
            }
            else
            {
                log_error(args->logger, "Error al buscar el frame en memoria");
            }
        }

        if (args->resultado)
        {
            switch (args->codigo)
            {
            case MOV_IN:
            {
                t_paquete *paquete = crear_paquete(CPU_MEMORIA_MOV_IN);
                t_mov_in *proceso_enviar = (t_mov_in *)args->paquete;

                proceso_enviar->numero_marco = args->marco;
                proceso_enviar->direccion_fisica = args->direccion_fisica;

                serializar_t_mov_in(&paquete, proceso_enviar);
                enviar_paquete(paquete, args->config_leida.socket_memoria);
                
                log_debug(args->logger, "Se envio la solicitud de la instruccion <MOV_IN> del proceso PID <%d> a Memoria", proceso_enviar->pid);

                eliminar_paquete(paquete);
                free(proceso_enviar);
                break;
            }
            case MOV_OUT:
            {
                t_paquete *paquete = crear_paquete(CPU_MEMORIA_MOV_OUT);
                t_mov_out *proceso_enviar = (t_mov_out *)args->paquete;

                proceso_enviar->numero_marco = args->marco;
                proceso_enviar->direccion_fisica = args->direccion_fisica;

                serializar_t_mov_out(&paquete, proceso_enviar);
                enviar_paquete(paquete, args->config_leida.socket_memoria);
                
                log_debug(args->logger, "Se envio la solicitud de la instruccion <MOV_OUT> del proceso PID <%d> a Memoria", proceso_enviar->pid);

                eliminar_paquete(paquete);
                free(proceso_enviar);
                break;
            }

            default:
            {
                break;
            }
            }
        }
    }

    log_debug(args->logger, "Hilo MMU finalizado");

    pthread_exit(0);
}