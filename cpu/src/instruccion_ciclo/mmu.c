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

            free(numero_frame);

            eliminar_paquete(paquete);

            // Espero a que memoria me devuelva el numero de frame
            sem_wait(&args->mmu_ejecucion);

            if (args->resultado)
            {
                // En este punto, ya tengo el numero de frame cargado en TLB, asi que lo busco y lo guardo en la estructura

                if (args->config_leida.cantidadEntradasTlb > 0)
                {
                    args->marco = buscar_en_tlb(args->proceso.pid, args->pagina, args->config_leida.cantidadEntradasTlb, args->tlb);
                }

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
            case IO_STDOUT_WRITE:
            {
                // Inicio la peticion contra Kernel para que retransmita a la interfaz
                t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_STDOUT_WRITE);
                t_io_stdout_write *proceso_completo = (t_io_stdout_write *)args->paquete;

                proceso_completo->direccion_fisica = args->direccion_fisica;
                proceso_completo->marco = args->marco;

                serializar_t_io_stdout_write(&paquete, proceso_completo);
                enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
                eliminar_paquete(paquete);

                log_debug(args->logger, "Se envio la solicitud de la instruccion IO_STDOUT_WRITE del proceso PID <%d> a Kernel", proceso_completo->pid);

                free(proceso_completo->interfaz);
                free(proceso_completo);
                break;
            }
            case COPY_STRING:
            {
                // Ahora, pediría el marco de DI
                t_copy_string *proceso_completo = (t_copy_string *)args->paquete;

                log_debug(args->logger, "Marco SI: <%d>", args->marco);
                log_debug(args->logger, "Direccion fisica SI: <%d>", args->direccion_fisica);

                proceso_completo->direccion_fisica_si = args->direccion_fisica;
                proceso_completo->marco_si = args->marco;
                mmu_iniciar(args, COPY_STRING_2, proceso_completo->direccion_di, (void *)proceso_completo);
                break;
            }
            case COPY_STRING_2:
            {
                t_paquete *paquete = crear_paquete(CPU_MEMORIA_COPY_STRING_2);
                t_copy_string *proceso_completo = (t_copy_string *)args->paquete;

                log_debug(args->logger, "Marco DI: <%d>", args->marco);
                log_debug(args->logger, "Direccion fisica SI: <%d>", args->direccion_fisica);

                proceso_completo->direccion_fisica_di = args->direccion_fisica;
                proceso_completo->marco_di = args->marco;

                serializar_t_copy_string(&paquete, proceso_completo);
                enviar_paquete(paquete, args->config_leida.socket_memoria);
                eliminar_paquete(paquete);

                free(proceso_completo);
                break;
            }
            case IO_STDIN_READ:
            {
                t_io_stdin_read *proceso_recibido = (t_io_stdin_read *)args->paquete;

                log_debug(args->logger, "Se obtuvo el marco inicial <%d> de la pagina <%d> asociado a la instruccion IO_STDIN_READ del proceso PID <%d>", proceso_recibido->marco_inicial, proceso_recibido->numero_pagina, proceso_recibido->pid);

                // Inicio la peticion contra Kernel para que retransmita a la interfaz
                t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_STDIN_READ);
                t_io_stdin_read *proceso_completo = malloc(sizeof(t_io_stdin_read));

                proceso_completo->pid = proceso_recibido->pid;
                proceso_completo->resultado = proceso_recibido->resultado;
                proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
                proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
                proceso_completo->marco_inicial = proceso_recibido->marco_inicial;
                proceso_completo->marco_final = proceso_recibido->marco_final;
                proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
                proceso_completo->direccion_fisica = args->direccion_fisica;
                proceso_completo->desplazamiento = proceso_recibido->desplazamiento;
                proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
                proceso_completo->size_interfaz = strlen(proceso_completo->interfaz) + 1;
                proceso_completo->registros = proceso_recibido->registros;

                serializar_t_io_stdin_read(&paquete, proceso_completo);
                enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
                eliminar_paquete(paquete);

                log_debug(args->logger, "Se envio la solicitud de la instruccion IO_STDIN_READ del proceso PID <%d> a Kernel", proceso_recibido->pid);

                free(proceso_completo->interfaz);
                free(proceso_completo);
                free(proceso_recibido);
                break;
            }
            case IO_FS_WRITE:
            {
                t_cpu_memoria_fs_write *proceso_recibido = (t_cpu_memoria_fs_write *)args->paquete;

                t_cpu_memoria_fs_write *proceso_completo = malloc(sizeof(t_cpu_memoria_fs_write));
                t_paquete *paquete = crear_paquete(CPU_MEMORIA_IO_FS_WRITE);

                log_debug(args->logger, "Se obtuvo el marco inicial <%d> de la pagina <%d> asociado a la instruccion IO_FS_WRITE del proceso PID <%d> con puntero archivo <%d>", args->marco, proceso_recibido->numero_pagina, proceso_recibido->pid, proceso_recibido->puntero_archivo);

                proceso_completo->pid = proceso_recibido->pid;
                proceso_completo->resultado = proceso_recibido->resultado;
                proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
                proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
                proceso_completo->size_interfaz = strlen(proceso_completo->interfaz) + 1;
                proceso_completo->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
                proceso_completo->size_nombre_archivo = strlen(proceso_completo->nombre_archivo) + 1;
                proceso_completo->registros = proceso_recibido->registros;
                proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
                proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
                proceso_completo->marco = args->marco;
                proceso_completo->direccion_fisica = args->direccion_fisica;
                proceso_completo->desplazamiento = proceso_recibido->desplazamiento;
                proceso_completo->puntero_archivo = proceso_recibido->puntero_archivo;

                serializar_t_cpu_memoria_fs_write(&paquete, proceso_completo);
                enviar_paquete(paquete, args->config_leida.socket_memoria);
                eliminar_paquete(paquete);

                log_debug(args->logger, "Se envio la solicitud de la instruccion IO_FS_WRITE del proceso PID <%d> a Memoria", proceso_recibido->pid);
                free(proceso_completo->interfaz);
                free(proceso_completo->nombre_archivo);
                free(proceso_completo);
                free(proceso_recibido->interfaz);
                free(proceso_recibido->nombre_archivo);
                free(proceso_recibido);
                break;
            }
            case IO_FS_READ:
            {
                t_cpu_kernel_fs_read *proceso_recibido = (t_cpu_kernel_fs_read *)args->paquete;

                t_cpu_kernel_fs_read *proceso_completo = malloc(sizeof(t_cpu_kernel_fs_read));
                t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_FS_READ);

                log_debug(args->logger, "Se obtuvo el marco inicial <%d> de la pagina <%d> asociado a la instruccion IO_FS_READ del proceso PID <%d> con puntero archivo <%d>", args->marco, proceso_recibido->numero_pagina, proceso_recibido->pid, proceso_recibido->puntero_archivo);
                proceso_completo->pid = proceso_recibido->pid;
                proceso_completo->resultado = proceso_recibido->resultado;
                proceso_completo->numero_pagina = proceso_recibido->numero_pagina;
                proceso_completo->interfaz = strdup(proceso_recibido->interfaz);
                proceso_completo->size_interfaz = strlen(proceso_completo->interfaz) + 1;
                proceso_completo->nombre_archivo = strdup(proceso_recibido->nombre_archivo);
                proceso_completo->size_nombre_archivo = strlen(proceso_completo->nombre_archivo) + 1;
                proceso_completo->registros = proceso_recibido->registros;
                proceso_completo->registro_direccion = proceso_recibido->registro_direccion;
                proceso_completo->registro_tamanio = proceso_recibido->registro_tamanio;
                proceso_completo->marco = args->marco;
                proceso_completo->direccion_fisica = args->direccion_fisica;
                proceso_completo->desplazamiento = proceso_recibido->desplazamiento;
                proceso_completo->puntero_archivo = proceso_recibido->puntero_archivo;

                serializar_t_cpu_kernel_fs_read(&paquete, proceso_completo);
                enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);
                eliminar_paquete(paquete);

                log_debug(args->logger, "Se envio la solicitud de la instruccion IO_FS_READ del proceso PID <%d> a Kernel", proceso_recibido->pid);
                free(proceso_completo->interfaz);
                free(proceso_completo->nombre_archivo);
                free(proceso_completo);
                free(proceso_recibido->interfaz);
                free(proceso_recibido->nombre_archivo);
                free(proceso_recibido);
                break;
            }
            default:
            {
                log_warning(args->logger, "Instruccion no reconocida");
                break;
            }
            }
        }
    }
    pthread_exit(0);
}