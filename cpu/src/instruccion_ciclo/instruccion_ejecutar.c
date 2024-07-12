#include <utils/cpu.h>

int instruccion_ejecutar(t_cpu *args)
{
    instruccion_log(args);

    switch (args->tipo_instruccion)
    {
    case SET:
    {
        // SET (Registro, Valor): Asigna al registro el valor pasado como parámetro.
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion[0], args->instruccion[1], args->instruccion[2]);

        char *registro_destino = strdup(args->instruccion[1]);
        char *valor = strdup(args->instruccion[2]);

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
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion[0], args->instruccion[1], args->instruccion[2]);

        char *argumento_destino = strdup(args->instruccion[1]);
        char *argumento_origen = strdup(args->instruccion[2]);

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
        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion[0], args->instruccion[1], args->instruccion[2]);

        char *argumento_destino = strdup(args->instruccion[1]);
        char *argumento_origen = strdup(args->instruccion[2]);

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
        char *registro = strdup(args->instruccion[1]);
        char *instruccion = strdup(args->instruccion[2]);

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

        char *interfaz = strdup(args->instruccion[1]);
        uint32_t tiempo = atoi(args->instruccion[2]);

        log_debug(args->logger, "Instruccion: '%s' '%s' '%s' reconocida", args->instruccion[0], args->instruccion[1], args->instruccion[2]);

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
        /*
        MOV_IN        EDX             ECX
        MOV_IN (Registro Datos, Registro Dirección)

        Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.
        */

        // Creo copias
        char *registro_datos = strdup(args->instruccion[1]);
        char *registro_direccion = strdup(args->instruccion[2]);

        // Determino que tipo de registros debo operar
        t_registro registro_datos_tipo = obtener_tipo_registro(registro_datos);
        t_registro registro_direccion_tipo = obtener_tipo_registro(registro_direccion);

        // Si alguno de los registros es invalido, no se puede continuar
        if (registro_direccion_tipo == INVALIDO || registro_datos_tipo == INVALIDO)
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }

        // Casos posibles:
        // 1. Registro datos (32) y registro direccion (32). Leo 4 bytes
        // 2. Registro datos (32) y registro direccion (8). Leo 4 bytes
        // 3. Registro datos (8) y registro direccion (32). Leo 1 byte
        // 4. Registro datos (8) y registro direccion (8). Leo 1 byte

        // t_paquete *paquete = crear_paquete(CPU_MEMORIA_MOV_IN);
        t_mov_in *proceso = malloc(sizeof(t_mov_in));

        // Inicializo
        proceso->pid = args->proceso.pid;
        proceso->numero_marco = 0;
        proceso->resultado = 0;
        proceso->direccion_fisica = 0;
        proceso->dato_32 = 0;
        proceso->dato_8 = 0;

        // Paso el valor de los registros a la estructura que le voy a mandar a memoria
        if (registro_datos_tipo == REGISTRO_32) // El registro datos es de 32 bits
        {
            proceso->tamanio_registro_datos = 4;

            if (registro_direccion_tipo == REGISTRO_32) // && El registro direccion es de 32 bits
            {
                uint32_t *registro_direccion_ptr = determinar_tipo_registro_uint32_t(registro_direccion, &args->proceso);
                uint32_t *registro_datos_ptr = determinar_tipo_registro_uint32_t(registro_datos, &args->proceso);

                proceso->registro_direccion = *registro_direccion_ptr;
                proceso->registro_datos = *registro_datos_ptr;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
            else // && El registro direccion es de 8 bits
            {
                uint32_t *registro_datos_tipo = determinar_tipo_registro_uint32_t(registro_datos, &args->proceso);
                uint8_t *registro_direccion_ptr = determinar_tipo_registro_uint8_t(registro_direccion, &args->proceso);

                uint32_t registro_direccion_casteado = casteo_uint32_t(*registro_direccion_ptr);

                proceso->registro_direccion = registro_direccion_casteado;
                proceso->registro_datos = *registro_datos_tipo;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
        }
        else // El registro datos es de 8 bits
        {
            proceso->tamanio_registro_datos = 1;

            if (registro_direccion_tipo == REGISTRO_32) // && El registro direccion es de 32 bits
            {
                uint8_t *registro_datos_ptr = determinar_tipo_registro_uint8_t(registro_datos, &args->proceso);
                uint32_t *registro_direccion_ptr = determinar_tipo_registro_uint32_t(registro_direccion, &args->proceso);

                uint32_t registro_datos_casteado = casteo_uint32_t(*registro_datos_ptr);

                proceso->registro_direccion = *registro_direccion_ptr;
                proceso->registro_datos = registro_datos_casteado;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
            else // && El registro direccion es de 8 bits
            {
                uint8_t *registro_direccion_ptr = determinar_tipo_registro_uint8_t(registro_direccion, &args->proceso);
                uint8_t *registro_datos_ptr = determinar_tipo_registro_uint8_t(registro_datos, &args->proceso);

                uint32_t registro_direccion_casteado = casteo_uint32_t(*registro_direccion_ptr);
                uint32_t registro_datos_casteado = casteo_uint32_t(*registro_datos_ptr);

                proceso->registro_direccion = registro_direccion_casteado;
                proceso->registro_datos = registro_datos_casteado;
                proceso->numero_pagina = calcular_numero_pagina(args, registro_direccion_casteado);
            }
        }

        mmu_iniciar(args, MOV_IN, proceso->registro_direccion, (void *)proceso);

        free(registro_datos);
        free(registro_direccion);
        return 1;
    }
    case MOV_OUT:
    {
        /*

        MOV_OUT         EDX                ECX
        MOV_OUT (Registro Dirección, Registro Datos)

        Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.

        */

        // Dirección logica + desplazamiento del tamaño del registro
        char *registro_direccion = strdup(args->instruccion[1]);

        // Datos a leer
        char *registro_datos = strdup(args->instruccion[2]);

        // Determino que tipo de registros debo operar
        t_registro registro_datos_tipo = obtener_tipo_registro(registro_datos);
        t_registro registro_direccion_tipo = obtener_tipo_registro(registro_direccion);

        // Si alguno de los registros es invalido, no se puede continuar
        if (registro_direccion_tipo == INVALIDO || registro_datos_tipo == INVALIDO)
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }

        // Casos posibles:
        // 1. Registro datos (32) y registro direccion (32). Escribo 4 bytes
        // 2. Registro datos (32) y registro direccion (8). Escribo 4 bytes
        // 3. Registro datos (8) y registro direccion (32). Escribo 1 byte
        // 4. Registro datos (8) y registro direccion (8). Escribo 1 byte

        t_mov_out *proceso = malloc(sizeof(t_mov_out));

        // Inicializo
        proceso->pid = args->proceso.pid;
        proceso->numero_marco = 0;
        proceso->resultado = 0;
        proceso->direccion_fisica = 0;
        proceso->dato_8 = 0;
        proceso->dato_32 = 0;

        // Paso el valor de los registros a la estructura que le voy a mandar a memoria
        if (registro_datos_tipo == REGISTRO_32) // El registro datos es de 32 bits
        {
            proceso->tamanio_registro_datos = 4;

            if (registro_direccion_tipo == REGISTRO_32) // && El registro direccion es de 32 bits
            {
                uint32_t *registro_direccion_ptr = determinar_tipo_registro_uint32_t(registro_direccion, &args->proceso);
                uint32_t *registro_datos_ptr = determinar_tipo_registro_uint32_t(registro_datos, &args->proceso);

                proceso->registro_direccion = *registro_direccion_ptr;
                proceso->registro_datos = *registro_datos_ptr;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
            else // && El registro direccion es de 8 bits
            {
                uint32_t *registro_datos_ptr = determinar_tipo_registro_uint32_t(registro_datos, &args->proceso);
                uint8_t *registro_direccion_ptr = determinar_tipo_registro_uint8_t(registro_direccion, &args->proceso);

                uint32_t registro_direccion_casteado = casteo_uint32_t(*registro_direccion_ptr);

                proceso->registro_direccion = registro_direccion_casteado;
                proceso->registro_datos = *registro_datos_ptr;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
        }
        else // El registro datos es de 8 bits
        {
            proceso->tamanio_registro_datos = 1;

            if (registro_direccion_tipo == REGISTRO_32) // && El registro direccion es de 32 bits
            {
                uint8_t *registro_datos_ptr = determinar_tipo_registro_uint8_t(registro_datos, &args->proceso);
                uint32_t *registro_direccion_ptr = determinar_tipo_registro_uint32_t(registro_direccion, &args->proceso);

                uint32_t registro_datos_casteado = casteo_uint32_t(*registro_datos_ptr);

                proceso->registro_direccion = *registro_direccion_ptr;
                proceso->registro_datos = registro_datos_casteado;
                proceso->numero_pagina = calcular_numero_pagina(args, *registro_direccion_ptr);
            }
            else // && El registro direccion es de 8 bits
            {
                uint8_t *registro_direccion_ptr = determinar_tipo_registro_uint8_t(registro_direccion, &args->proceso);
                uint8_t *registro_datos_ptr = determinar_tipo_registro_uint8_t(registro_datos, &args->proceso);

                uint32_t registro_direccion_casteado = casteo_uint32_t(*registro_direccion_ptr);
                uint32_t registro_datos_casteado = casteo_uint32_t(*registro_datos_ptr);

                proceso->registro_direccion = registro_direccion_casteado;
                proceso->registro_datos = registro_datos_casteado;
                proceso->numero_pagina = calcular_numero_pagina(args, registro_direccion_casteado);
            }
        }

        mmu_iniciar(args, MOV_OUT, proceso->registro_direccion, (void *)proceso);

        // Serializo la estructura y la envio a memoria
        // serializar_t_mov_out(&paquete, proceso);
        // enviar_paquete(paquete, args->config_leida.socket_memoria);
        // eliminar_paquete(paquete);

        free(registro_datos);
        free(registro_direccion);
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
        resize->bytes = atoi(args->instruccion[1]);

        serializar_t_cpu_memoria_resize(&paquete, resize);
        enviar_paquete(paquete, args->config_leida.socket_memoria);

        log_debug(args->logger, "Se solicita un resize de <%d> bytes para el proceso <%d>", resize->bytes, resize->pid);

        free(resize);
        eliminar_paquete(paquete);
        return 1;
    }
    case COPY_STRING:
    {
        /*
        COPY_STRING    38
        COPY_STRING (Tamaño)

        Toma el string apuntado por la direccion logica del registro SI y copia la cantidad de bytes indicadas en el parámetro tamaño a la direccion logica apuntada por el registro DI.

        Pasos:

        1- Obtengo los marcos correspondientes a los numeros de paginas asociados a las direcciones logicas
        2- Con los marcos, calculo las direcciones fisicas a partir de las cuales leer y escribir los datos
        3- Escribo en la direccion fisica de DI los datos leidos de SI
        4- Notifico a todos los procesos que esten esperando por el resultado de la operacion
        */

        // t_paquete *paquete = crear_paquete(CPU_MEMORIA_COPY_STRING);
        t_copy_string *proceso = malloc(sizeof(t_copy_string));

        // Cargo las cosas
        proceso->pid = args->proceso.pid;
        proceso->resultado = 0;
        proceso->cant_bytes = atoi(strdup(args->instruccion[1]));
        proceso->direccion_si = args->proceso.registros.si;
        proceso->direccion_di = args->proceso.registros.di;

        proceso->num_pagina_si = 0;

        proceso->num_pagina_di = 0;

        log_warning(args->logger, "Se calculan las paginas <%d> para SI y <%d> para DI", proceso->num_pagina_si, proceso->num_pagina_di);

        proceso->direccion_fisica_si = 0;
        proceso->direccion_fisica_di = 0;
        proceso->marco_si = 0;
        proceso->marco_di = 0;
        proceso->frase = strdup("");
        proceso->size_frase = strlen(proceso->frase) + 1;

        mmu_iniciar(args, COPY_STRING, proceso->direccion_si, (void *)proceso);

        return 1;
    }
    case IO_STDIN_READ:
    {
        /*                             129
        IO_STDIN_READ    Int2           BX                 EAX
        IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)

        Esta instrucción solicita al Kernel que mediante la interfaz ingresada se lea desde el STDIN (Teclado) un valor cuyo tamaño está delimitado por el valor del Registro Tamaño

        y el mismo se guarde a partir de la Dirección Lógica almacenada en el Registro Dirección. */

        // Creo copias
        char *interfaz = strdup(args->instruccion[1]);
        char *registro_direccion = strdup(args->instruccion[2]);
        char *registro_tamanio = strdup(args->instruccion[3]);

        // Determino que tipo de registros debo operar
        t_registro registro_direccion_tipo = obtener_tipo_registro(registro_direccion);
        t_registro registro_tamanio_tipo = obtener_tipo_registro(registro_tamanio);

        // Si alguno de los registros es invalido, no se puede continuar
        if (registro_direccion_tipo == INVALIDO || registro_tamanio_tipo == INVALIDO)
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }

        // Primero pido el marco a memoria asociado a la dirección lógica, para poder traducir con la MMU

        t_paquete *paquete = crear_paquete(CPU_MEMORIA_IO_STDIN_READ);
        t_io_stdin_read *proceso = malloc(sizeof(t_io_stdin_read));

        // Cargo el PID
        proceso->pid = args->proceso.pid;

        // Cargo la Interfaz
        proceso->interfaz = strdup(interfaz);

        // El tamaño del identificador de la interfaz
        proceso->size_interfaz = strlen(interfaz) + 1;

        // Estos son los datos que me va a devolver memoria
        proceso->marco_inicial = 0;
        proceso->marco_final = 0;

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

        // Calculo desplazamiento
        proceso->desplazamiento = proceso->registro_direccion - proceso->numero_pagina * args->tam_pagina;

        // Me guardo el contexto del proceso
        proceso->registros = args->proceso.registros;

        // Serializo la estructura y se la envio a memoria
        serializar_t_io_stdin_read(&paquete, proceso);
        enviar_paquete(paquete, args->config_leida.socket_memoria);
        eliminar_paquete(paquete);

        // Libero las cosas
        free(proceso->interfaz);
        free(proceso);

        free(interfaz);
        free(registro_direccion);
        free(registro_tamanio);

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
        char *interfaz = strdup(args->instruccion[1]);
        char *registro_direccion = strdup(args->instruccion[2]);
        char *registro_tamanio = strdup(args->instruccion[3]);

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

        t_io_stdout_write *proceso = malloc(sizeof(t_io_stdout_write));

        // Cargo el PID
        proceso->pid = args->proceso.pid;

        // Cargo la Interfaz
        proceso->interfaz = strdup(interfaz);

        // El tamaño del identificador de la interfaz
        proceso->size_interfaz = strlen(interfaz) + 1;

        proceso->marco = 0;
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
        proceso->desplazamiento = proceso->registro_direccion - proceso->numero_pagina * args->tam_pagina;

        // Por ultimo, me guardo el contexto del proceso
        proceso->registros = args->proceso.registros;

        mmu_iniciar(args, IO_STDOUT_WRITE, proceso->registro_direccion, (void *)proceso);

        // Serializo la estructura y la envio a memoria
        // serializar_t_io_stdout_write(&paquete, proceso);
        // enviar_paquete(paquete, args->config_leida.socket_memoria);
        // eliminar_paquete(paquete);

        free(interfaz);
        free(registro_direccion);
        free(registro_tamanio);

        return 1;
    }
    case IO_FS_CREATE:
    {
        /*
        IO_FS_CREATE Int3 notas.txt
        IO_FS_CREATE (Interfaz, Nombre Archivo): Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se cree un archivo en el FS montado en dicha interfaz.*/

        char *interfaz = strdup(args->instruccion[1]);
        char *nombre_archivo = strdup(args->instruccion[2]);

        t_entrada_salida_fs_create *proceso = malloc(sizeof(t_entrada_salida_fs_create));
        proceso->pid = args->proceso.pid;
        proceso->interfaz = strdup(interfaz);
        proceso->size_interfaz = strlen(interfaz) + 1;
        proceso->nombre_archivo = strdup(nombre_archivo);
        proceso->size_nombre_archivo = strlen(nombre_archivo) + 1;
        proceso->resultado = 0; // Después lo modifica FS
        proceso->registros = args->proceso.registros;

        t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_FS_CREATE);
        serializar_t_entrada_salida_fs_create(&paquete, proceso);

        enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

        eliminar_paquete(paquete);
        free(interfaz);
        free(nombre_archivo);
        free(proceso->interfaz);
        free(proceso->nombre_archivo);
        free(proceso);
        return 1;
    }
    case IO_FS_DELETE:
    {
        char *interfaz = strdup(args->instruccion[1]);
        char *nombre_archivo = strdup(args->instruccion[2]);

        // Se utiliza la serializacion de la estructura t_entrada_salida_fs_delete para no repetir logica
        t_entrada_salida_fs_create *proceso = malloc(sizeof(t_entrada_salida_fs_create));
        proceso->pid = args->proceso.pid;
        proceso->interfaz = strdup(interfaz);
        proceso->size_interfaz = strlen(interfaz) + 1;
        proceso->nombre_archivo = strdup(nombre_archivo);
        proceso->size_nombre_archivo = strlen(nombre_archivo) + 1;
        proceso->resultado = 0; // Después lo modifica FS
        proceso->registros = args->proceso.registros;

        t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_FS_DELETE);
        serializar_t_entrada_salida_fs_create(&paquete, proceso);

        enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

        eliminar_paquete(paquete);
        free(interfaz);
        free(nombre_archivo);
        free(proceso->interfaz);
        free(proceso->nombre_archivo);
        free(proceso);
        return 1;
    }
    case IO_FS_TRUNCATE:
    {
        char *interfaz = strdup(args->instruccion[1]);
        char *nombre_archivo = strdup(args->instruccion[2]);
        char *registro_destino = strdup(args->instruccion[3]);
        t_registro registro_tipo = obtener_tipo_registro(registro_destino);
        if (registro_tipo == INVALIDO)
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }
        uint32_t *registro_tamanio_truncar;
        if (registro_tipo == REGISTRO_32)
        {
            registro_tamanio_truncar = determinar_tipo_registro_uint32_t(registro_destino, &args->proceso);
        }
        if (registro_tipo == REGISTRO_8)
        {
            uint8_t *registro_tamanio_ptr = determinar_tipo_registro_uint8_t(registro_destino, &args->proceso);
            uint32_t registro_tamanio_casteado = casteo_uint32_t(*registro_tamanio_ptr);
            registro_tamanio_truncar = &registro_tamanio_casteado;
        }
        t_cpu_kernel_fs_truncate *proceso = malloc(sizeof(t_cpu_kernel_fs_truncate));

        proceso->pid = args->proceso.pid;
        proceso->interfaz = strdup(interfaz);
        proceso->size_interfaz = strlen(interfaz) + 1;
        proceso->nombre_archivo = strdup(nombre_archivo);
        proceso->size_nombre_archivo = strlen(nombre_archivo) + 1;
        proceso->tamanio_a_truncar = *registro_tamanio_truncar;
        proceso->registros = args->proceso.registros;
        proceso->resultado = 0;

        log_debug(args->logger, "Se solicita un truncate de <%d> bytes para el archivo <%s> en la interfaz <%s>", proceso->tamanio_a_truncar, proceso->nombre_archivo, proceso->interfaz);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_FS_TRUNCATE);
        serializar_t_cpu_kernel_fs_truncate(&paquete, proceso);

        enviar_paquete(paquete, args->config_leida.socket_kernel_dispatch);

        eliminar_paquete(paquete);
        free(interfaz);
        free(nombre_archivo);
        free(proceso->interfaz);
        free(proceso->nombre_archivo);
        free(proceso);
        return 1;
    }
    case IO_FS_WRITE:
    {
        char *interfaz = strdup(args->instruccion[1]);
        char *nombre_archivo = strdup(args->instruccion[2]);
        char *registro_direccion = strdup(args->instruccion[3]);
        char *registro_tamanio = strdup(args->instruccion[4]);
        char *registro_puntero_archivo = strdup(args->instruccion[5]);

        t_cpu_memoria_fs_write *proceso = malloc(sizeof(t_cpu_memoria_fs_write));

        t_registro registro_tipo_puntero_archivo = obtener_tipo_registro(registro_puntero_archivo);
        if (registro_tipo_puntero_archivo == INVALIDO)
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }
        if (registro_tipo_puntero_archivo == REGISTRO_32) // El registro direccion es de 32 bits
        {
            uint32_t *registro_tipo_puntero_archivo_ptr = determinar_tipo_registro_uint32_t(registro_puntero_archivo, &args->proceso);
            proceso->puntero_archivo = *registro_tipo_puntero_archivo_ptr;
            proceso->numero_pagina = calcular_numero_pagina(args, *registro_tipo_puntero_archivo_ptr);
        }
        else // El registro direccion es de 8 bits
        {
            uint8_t *registro_tipo_puntero_archivo_ptr = determinar_tipo_registro_uint8_t(registro_puntero_archivo, &args->proceso);

            uint32_t registro_puntero_casteado = casteo_uint32_t(*registro_tipo_puntero_archivo_ptr);

            proceso->puntero_archivo = registro_puntero_casteado;
            proceso->numero_pagina = calcular_numero_pagina(args, registro_puntero_casteado);
        }

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
        proceso->desplazamiento = proceso->registro_direccion - proceso->numero_pagina * args->tam_pagina;

        // Por ultimo, me guardo el contexto del proceso
        proceso->registros = args->proceso.registros;

        proceso->pid = args->proceso.pid;
        proceso->interfaz = strdup(interfaz);
        proceso->size_interfaz = strlen(interfaz) + 1;
        proceso->nombre_archivo = strdup(nombre_archivo);
        proceso->size_nombre_archivo = strlen(nombre_archivo) + 1;
        proceso->resultado = 0;
        proceso->direccion_fisica = 0;

        mmu_iniciar(args, IO_FS_WRITE, proceso->registro_direccion, (void *)proceso);

        free(interfaz);
        free(nombre_archivo);
        free(registro_direccion);
        free(registro_tamanio);
        free(registro_puntero_archivo);

        return 1;
    }
    case IO_FS_READ:
    {
        char *interfaz = strdup(args->instruccion[1]);
        char *nombre_archivo = strdup(args->instruccion[2]);
        char *registro_direccion = strdup(args->instruccion[3]);
        char *registro_tamanio = strdup(args->instruccion[4]);
        char *registro_puntero_archivo = strdup(args->instruccion[5]);

        t_cpu_kernel_fs_read *proceso = malloc(sizeof(t_cpu_kernel_fs_read));

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

        t_registro registro_tipo_puntero_archivo = obtener_tipo_registro(registro_puntero_archivo);
        if (registro_tipo_puntero_archivo == INVALIDO)
        {
            log_error(args->logger, "Registro invalido");
            return -1;
        }
        if (registro_tipo_puntero_archivo == REGISTRO_32) // El registro direccion es de 32 bits
        {
            uint32_t *registro_tipo_puntero_archivo_ptr = determinar_tipo_registro_uint32_t(registro_puntero_archivo, &args->proceso);
            proceso->puntero_archivo = *registro_tipo_puntero_archivo_ptr;
            proceso->numero_pagina = calcular_numero_pagina(args, *registro_tipo_puntero_archivo_ptr);
        }
        else // El registro direccion es de 8 bits
        {
            uint8_t *registro_tipo_puntero_archivo_ptr = determinar_tipo_registro_uint8_t(registro_puntero_archivo, &args->proceso);

            uint32_t registro_puntero_casteado = casteo_uint32_t(*registro_tipo_puntero_archivo_ptr);

            proceso->puntero_archivo = registro_puntero_casteado;
            proceso->numero_pagina = calcular_numero_pagina(args, registro_puntero_casteado);
        }

        // Me guardo el desplazamiento para poder calcular la dirección física
        proceso->desplazamiento = proceso->registro_direccion - proceso->numero_pagina * args->tam_pagina;

        // Por ultimo, me guardo el contexto del proceso
        proceso->registros = args->proceso.registros;

        proceso->pid = args->proceso.pid;
        proceso->interfaz = strdup(interfaz);
        proceso->size_interfaz = strlen(interfaz) + 1;
        proceso->nombre_archivo = strdup(nombre_archivo);
        proceso->size_nombre_archivo = strlen(nombre_archivo) + 1;
        proceso->resultado = 0;
        proceso->direccion_fisica = 0;

        log_warning(args->logger, "String registro puntero: <%s>", registro_puntero_archivo);
        log_warning(args->logger, "Registro puntero: <%d>", proceso->puntero_archivo);

        mmu_iniciar(args, IO_FS_READ, proceso->registro_direccion, (void *)proceso);

        free(interfaz);
        free(nombre_archivo);
        free(registro_direccion);
        free(registro_tamanio);
        free(registro_puntero_archivo);

        return 1;
    }
    case WAIT:
    {
        char *nombre_recurso = strdup(args->instruccion[1]);
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
        char *nombre_recurso = strdup(args->instruccion[1]);
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

    return 0;
}
