#include <utils/cpu.h>

int instruccion_ejecutar(t_cpu *args)
{
    remover_salto_linea(args->instruccion.array[args->instruccion.cantidad_elementos - 1]);
    log_instruccion(args);

    switch (args->tipo_instruccion)
    {
    case SET:
    {
        // SET (Registro, Valor): Asigna al registro el valor pasado como parámetro.
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        char *registro_destino = strdup(args->instruccion.array[1]);
        char *valor = strdup(args->instruccion.array[2]);

        t_registro registro_tipo = obtener_tipo_registro(registro_destino);

        if (registro_tipo == REGISTRO_32)
        {
            uint32_t *registro = determinar_tipo_registro_uint32_t(registro_destino, &args->proceso);
            if (registro == 0)
            {
                log_error(args->logger, "Registro %s a insertar invalido", registro_destino);
                return -1;
            }
            uint32_t valor_a_enviar = strtoul(valor, NULL, 10);
            if (valor_a_enviar > UINT32_MAX)
            {
                log_error(args->logger, "El numero es mas grande que la capacidad de uint32");
                return -1;
            }
            *registro = valor_a_enviar;
        }
        else if (registro_tipo == REGISTRO_8) // Misma lógica que para registro de 4 bytes;
        {
            uint8_t *registro = determinar_tipo_registro_uint8_t(registro_destino, &args->proceso);
            if (registro == 0)
            {
                log_error(args->logger, "Registro %s a insertar invalido", registro_destino);
                return -1;
            }
            unsigned long num = strtoul(valor, NULL, 10);
            if (num > UINT8_MAX)
            {
                log_error(args->logger, "El numero es mas grande que la capacidad de uint8");
                return -1;
            }
            uint8_t valor_a_enviar = (uint8_t)num;
            *registro = valor_a_enviar;
            log_debug(args->logger, "Valor final del registro %s: %d", registro_destino, *registro);
        }
        else
        {
            log_error(args->logger, "No existe registro con ese tipo de dato");
        }
        imprimir_registros(args);

        free(registro_destino);
        free(valor);
        return 0;
    }
    case SUM:
    {
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        char *argumento_destino = strdup(args->instruccion.array[1]);
        char *argumento_origen = strdup(args->instruccion.array[2]);

        // Determino si es un registro de 32 o 8 bits
        t_registro argumento_destino_tipo = obtener_tipo_registro(argumento_destino);
        t_registro argumento_origen_tipo = obtener_tipo_registro(argumento_origen);

        log_debug(args->logger, "Argumento destino tipo: %d", argumento_destino_tipo);
        log_debug(args->logger, "Argumento origen tipo: %d", argumento_origen_tipo);

        if (argumento_destino_tipo != INVALIDO && argumento_origen_tipo != INVALIDO)
        {
            if (argumento_destino_tipo == argumento_origen_tipo)
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_destino_anterior = *registro_destino;
                    *registro_destino = *registro_destino + *registro_origen;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino %s no encontrado", argumento_destino);
                        return -1;
                    }

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen %s no encontrado", argumento_origen);
                        return -1;
                    }
                    uint32_t registro_destino_anterior = *registro_destino;
                    *registro_destino += *registro_origen;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);
                }
            }
            else
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {

                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_destino_anterior = *registro_destino;

                    uint32_t registro_origen_casteado = casteo_uint32_t(*registro_origen);

                    *registro_destino = *registro_destino + registro_origen_casteado;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);

                    log_debug(args->logger, "Argumento destino tipo: %d", argumento_destino_tipo);
                }
                if (argumento_destino_tipo == REGISTRO_8)
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint8_t registro_destino_anterior = *registro_destino;
                    uint8_t registro_origen_casteado;

                    if (*registro_origen > 0)
                    {
                        registro_origen_casteado = casteo_uint8_t(*registro_origen);

                        if (registro_origen_casteado == 0)
                        {
                            log_error(args->logger, "No se puede realizar la suma, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                            return -1;
                        }
                    }
                    else
                    { // Dado que son unsigned, son siempre positivos. Entonces, el unico caso que queda es que sea 0.
                        registro_origen_casteado = 0;
                    }

                    if (*registro_destino + registro_origen_casteado > UINT8_MAX)
                    {
                        log_error(args->logger, "No se puede realizar la suma, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                        return -1;
                    }

                    *registro_destino = *registro_destino + registro_origen_casteado;

                    log_debug(args->logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);

                    log_debug(args->logger, "Argumento destino tipo: %d", argumento_destino_tipo);
                }
                break;
            }
        }
        else
        {
            log_error(args->logger, "Se reconocieron los registros %s y %s. No se pueden sumar porque al menos uno no es válido.", argumento_destino, argumento_origen);
            return -1;
        }

        free(argumento_destino);
        free(argumento_origen);
        return 0;
    }
    case SUB:
    {
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        char *argumento_destino = strdup(args->instruccion.array[1]);
        char *argumento_origen = strdup(args->instruccion.array[2]);

        // Determino si es un registro de 32 o 8 bits
        t_registro argumento_destino_tipo = obtener_tipo_registro(argumento_destino);
        t_registro argumento_origen_tipo = obtener_tipo_registro(argumento_origen);

        if (argumento_destino_tipo != INVALIDO && argumento_origen_tipo != INVALIDO)
        {
            if (argumento_destino_tipo == argumento_origen_tipo)
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    if (*registro_destino < *registro_origen)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= *registro_origen;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino %s no encontrado", argumento_destino);
                        return -1;
                    }

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen %s no encontrado", argumento_origen);
                        return -1;
                    }

                    if (*registro_destino < *registro_origen)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino = *registro_destino - *registro_origen;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);
                }
            }
            else
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_origen_casteado = casteo_uint32_t(*registro_origen);

                    if (*registro_destino < registro_origen_casteado)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= registro_origen_casteado;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, &args->proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(args->logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, &args->proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(args->logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(args->logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint8_t registro_origen_casteado;

                    if (*registro_origen > 0)
                    {
                        registro_origen_casteado = casteo_uint8_t(*registro_origen);

                        if (registro_origen_casteado == 0)
                        {
                            log_error(args->logger, "No se puede realizar la resta, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                            return -1;
                        }
                    }
                    else
                    { // Dado que son unsigned, son siempre positivos. Entonces, el unico caso que queda es que sea 0.
                        registro_origen_casteado = 0;
                    }

                    if (*registro_destino < registro_origen_casteado)
                    {
                        log_error(args->logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= registro_origen_casteado;

                    log_debug(args->logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(args->logger, "Registros del proceso %d", args->proceso.pid);

                    imprimir_registros(args);
                }
            }
        }
        else
        {
            log_error(args->logger, "Se reconocieron los registros %s y %s. No se pueden sumar porque al menos uno no es válido.", argumento_destino, argumento_origen);
            return -1;
        }

        free(argumento_destino);
        free(argumento_origen);
        return 0;
    }
    case JNZ:
    {
        // JNZ (Registro, Instrucción): Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro.
        char *registro = strdup(args->instruccion.array[1]);
        char *instruccion = strdup(args->instruccion.array[2]);

        t_registro registro_tipo_size = obtener_tipo_registro(registro);

        if (registro_tipo_size == REGISTRO_8)
        {
            uint8_t *registro_destino = determinar_tipo_registro_uint8_t(registro, &args->proceso);
            if (*registro_destino != 0)
            {
                uint32_t valor_pc = strtoul(instruccion, NULL, 10);
                args->proceso.registros.pc = valor_pc;
            }
        }
        else if (registro_tipo_size == REGISTRO_32)
        {
            uint32_t *registro_destino = determinar_tipo_registro_uint32_t(registro, &args->proceso);
            if (*registro_destino != 0)
            {
                uint32_t valor_pc = strtoul(instruccion, NULL, 10);
                args->proceso.registros.pc = valor_pc;
            }
        }
        else
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }
        return 0;
    }
    case IO_GEN_SLEEP:
    {
        // IO_GEN_SLEEP (Interfaz, Unidades de trabajo): Esta instrucción solicita al Kernel que se envíe a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo.

        char *interfaz = strdup(args->instruccion.array[1]);
        uint32_t tiempo = atoi(args->instruccion.array[2]);

        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion.array[0], args->instruccion.array[1], args->instruccion.array[2]);

        log_debug(args->logger, "Interfaz: %s", interfaz);
        log_debug(args->logger, "Tiempo: %d", tiempo);

        log_debug(args->logger, "Se envia un sleep de %d a la Interfaz IO Generic ID %s", tiempo, interfaz);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_GEN_SLEEP);

        t_cpu_kernel_io_gen_sleep *unidad = malloc(sizeof(t_cpu_kernel_io_gen_sleep));
        unidad->tiempo = tiempo;
        unidad->size_interfaz = strlen(interfaz) + 1;
        unidad->interfaz = strdup(interfaz);
        unidad->pid = args->proceso.pid;
        unidad->registros = args->proceso.registros;

        serializar_t_cpu_kernel_io_gen_sleep(&paquete, unidad);

        enviar_paquete(paquete, args->config_leida.socket_kernel_interrupt);

        imprimir_registros(args);

        free(unidad->interfaz);
        free(unidad);
        eliminar_paquete(paquete);
        return 1;
    }
    case MOV_IN:
    {
        log_debug(args->logger, "reconoci un MOV_IN");

        // TODO: Implementar

        // Ahora lo que hago es testear si funciona la MMU correctamente

        // t_cpu_memoria_numero_frame *proceso = malloc(sizeof(t_cpu_memoria_numero_frame));
        // proceso->pid = cpu_proceso->pid;

        // proceso->numero_pagina = calcular_numero_pagina(cpu_proceso);

        // t_paquete *paquete = crear_paquete(CPU_MEMORIA_NUMERO_FRAME);
        // serializar_t_cpu_memoria_numero_frame(&paquete, proceso);

        // enviar_paquete(paquete, cpu_paquete.socket_memoria);
        // free(paquete);

        // uint32_t numero_marco = deserializar_t_memoria_cpu_numero_marco(recibir_paquete(cpu_paquete.socket_memoria));

        // log_info(args->logger, "Numero de marco: %d", numero_marco);

        return 1;
    }
    case MOV_OUT:
    {
        log_debug(args->logger, "reconoci un MOV_OUT");
        return 1;
    }
    case RESIZE:
    {
        /*
        RESIZE (Tamaño): Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
        */

        t_paquete *paquete = crear_paquete(CPU_MEMORIA_RESIZE);
        t_cpu_memoria_resize *resize = malloc(sizeof(t_cpu_memoria_resize));
        resize->pid = args->proceso.pid;
        resize->bytes = atoi(args->instruccion.array[1]);

        serializar_t_cpu_memoria_resize(&paquete, resize);
        enviar_paquete(paquete, args->config_leida.socket_memoria);

        log_debug(args->logger, "Se solicita un resize de <%d> bytes para el proceso <%d>", resize->bytes, resize->pid);

        free(resize);
        eliminar_paquete(paquete);
        return 1;
    }
    case COPY_STRING:
    {
        log_debug(args->logger, "reconoci un COPY_STRING");
        return 1;
    }
    case IO_STDIN_READ:
    {
        log_debug(args->logger, "reconoci un IO_STDIN_READ");
        return 1;
    }
    case IO_STDOUT_WRITE:
    {
        /*
        IO_STDOUT_WRITE    Int3           BX                 EAX
        IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño)

        Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde la posición de memoria indicada por la Dirección Lógica almacenada en el Registro Dirección, un tamaño indicado por el Registro Tamaño y se imprima por pantalla.
        */

        // Pido a Memoria el marco asociado a la direccion logica que me llega para poder traducir con MMU

        // Le adjunto en el paquete los datos que necesita, más los que yo necesito que me devuelva para seguir con la ejecución de la instruccion.

        // Creo copias
        char *interfaz = strdup(args->instruccion.array[1]);
        char *registro_direccion = strdup(args->instruccion.array[2]);
        char *registro_tamanio = strdup(args->instruccion.array[3]);

        // Determino que tipo de registros debo operar
        t_registro registro_direccion_tipo = obtener_tipo_registro(registro_direccion);
        t_registro registro_tamanio_tipo = obtener_tipo_registro(registro_tamanio);

        // Si alguno de los registros es invalido, no se puede continuar
        if (registro_direccion_tipo == INVALIDO || registro_tamanio_tipo == INVALIDO)
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }

        // Casos posibles:
        // 1. Registro tamaño de 32 bits y registro direccion de 32 bits
        // 2. Registro tamaño de 32 bits y registro direccion de 8 bits: Casteo a 32 bits
        // 3. Registro tamaño de 8 bits y registro direccion de 32 bits: Casteo a 32 bits
        // 4. Registro tamaño de 8 bits y registro direccion de 8 bits: Casteo a 32 bits

        t_paquete *paquete = crear_paquete(CPU_MEMORIA_IO_STDOUT_WRITE);
        t_io_stdout_write *proceso = malloc(sizeof(t_io_stdout_write));

        // Cargo el PID
        proceso->pid = args->proceso.pid;
        
        // Cargo la Interfaz
        proceso->interfaz = strdup(interfaz);
        
        // El tamaño del identificador de la interfaz
        proceso->size_interfaz = strlen(interfaz) + 1;
        
        // Este es el dato que me va a devolver memoria
        proceso->marco = 0;

        // Esto tambien me lo devuelve memoria
        proceso->resultado = 0;

        // Paso el valor de los registros a la estructura que le voy a mandar a memoria
        if (registro_direccion_tipo == REGISTRO_32) // El registro direccion es de 32 bits
        {
            if (registro_tamanio_tipo == REGISTRO_32) // && El registro tamaño es de 32 bits
            {
                uint32_t *registro_direccion_ptr = determinar_tipo_registro_uint32_t(registro_direccion, &args->proceso);
                uint32_t *registro_tamanio_ptr = determinar_tipo_registro_uint32_t(registro_tamanio, &args->proceso);

                proceso->registro_direccion = *registro_direccion_ptr;
                proceso->registro_tamanio = *registro_tamanio_ptr;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
            else // && El registro tamaño es de 8 bits
            {
                uint32_t *registro_direccion_ptr = determinar_tipo_registro_uint32_t(registro_direccion, &args->proceso);
                uint8_t *registro_tamanio_ptr = determinar_tipo_registro_uint8_t(registro_tamanio, &args->proceso);

                uint32_t registro_tamanio_casteado = casteo_uint32_t(*registro_tamanio_ptr);

                proceso->registro_direccion = *registro_direccion_ptr;
                proceso->registro_tamanio = registro_tamanio_casteado;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
        }
        else // El registro direccion es de 8 bits
        {
            if (registro_tamanio_tipo == REGISTRO_32) // && El registro tamaño es de 32 bits
            {
                uint8_t *registro_direccion_ptr = determinar_tipo_registro_uint8_t(registro_direccion, &args->proceso);
                uint32_t *registro_tamanio_ptr = determinar_tipo_registro_uint32_t(registro_tamanio, &args->proceso);

                uint32_t registro_direccion_casteado = casteo_uint32_t(*registro_direccion_ptr);

                proceso->registro_direccion = registro_direccion_casteado;
                proceso->registro_tamanio = *registro_tamanio_ptr;
                proceso->numero_pagina = calcular_numero_pagina(args, registro_direccion_casteado);
            }
            else // && El registro tamaño es de 8 bits
            {
                uint8_t *registro_direccion_ptr = determinar_tipo_registro_uint8_t(registro_direccion, &args->proceso);
                uint8_t *registro_tamanio_ptr = determinar_tipo_registro_uint8_t(registro_tamanio, &args->proceso);

                uint32_t registro_direccion_casteado = casteo_uint32_t(*registro_direccion_ptr);
                uint32_t registro_tamanio_casteado = casteo_uint32_t(*registro_tamanio_ptr);

                proceso->registro_direccion = registro_direccion_casteado;
                proceso->registro_tamanio = registro_tamanio_casteado;
                proceso->numero_pagina = calcular_numero_pagina(args, registro_direccion_casteado);
            }
        }

        // Me guardo el desplazamiento para poder calcular la dirección física

        // desplazamiento = dirección_lógica - número_página * tamaño_página
        proceso->desplazamiento = proceso->registro_direccion - proceso->numero_pagina * args->tam_pagina;
        
        // Por ultimo, me guardo el contexto del proceso
        proceso->registros = args->proceso.registros;

        // Serializo la estructura y la envio a memoria
        serializar_t_io_stdout_write(&paquete, proceso);
        enviar_paquete(paquete, args->config_leida.socket_memoria);
        eliminar_paquete(paquete);

        free(proceso->interfaz);
        free(proceso);

        return 1;
    }
    case IO_FS_CREATE:
    {
        log_debug(args->logger, "reconoci un IO_FS_CREATE");
        return 1;
    }
    case IO_FS_DELETE:
    {
        log_debug(args->logger, "reconoci un IO_FS_DELETE");
        return 1;
    }
    case IO_FS_TRUNCATE:
    {
        log_debug(args->logger, "reconoci un IO_FS_TRUNCATE");
        return 1;
    }
    case IO_FS_WRITE:
    {
        log_debug(args->logger, "reconoci un IO_FS_WRITE");
        return 1;
    }
    case IO_FS_READ:
    {
        log_debug(args->logger, "reconoci un IO_FS_READ");
        return 1;
    }
    case WAIT:
    {
        char *nombre_recurso = strdup(args->instruccion.array[1]);
        log_debug(args->logger, "Recurso a esperar: %s", nombre_recurso);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_WAIT);

        t_cpu_kernel_solicitud_recurso *contexto = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        contexto->pid = args->proceso.pid;
        contexto->registros = &(args->proceso.registros);
        contexto->size_nombre_recurso = strlen(nombre_recurso) + 1;
        contexto->nombre_recurso = strdup(nombre_recurso);

        serializar_t_cpu_kernel_solicitud_recurso(&paquete, contexto);

        enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

        free(nombre_recurso);
        free(contexto->nombre_recurso);
        free(contexto);

        eliminar_paquete(paquete);

        return 1;
    }
    case SIGNAL:
    {
        char *nombre_recurso = strdup(args->instruccion.array[1]);
        log_debug(args->logger, "Recurso a liberar: %s", nombre_recurso);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_SIGNAL);

        t_cpu_kernel_solicitud_recurso *contexto = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        contexto->pid = args->proceso.pid;
        contexto->registros = &(args->proceso.registros);
        contexto->size_nombre_recurso = strlen(nombre_recurso) + 1;
        contexto->nombre_recurso = strdup(nombre_recurso);

        serializar_t_cpu_kernel_solicitud_recurso(&paquete, contexto);

        enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

        free(nombre_recurso);
        free(contexto->nombre_recurso);
        free(contexto);
        eliminar_paquete(paquete);

        return 1;
    }
    case EXIT:
    {
        args->proceso.ejecutado = 1;
        instruccion_finalizar(args);
        return 1;
    }
    }

    log_error(args->logger, "Instruccion invalida");
    return 0;
};
