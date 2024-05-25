#include "cpu.h"

t_cpu cpu_inicializar(t_config *config)
{
    t_cpu cpu = {
        .puertoEscuchaDispatch = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH"),
        .puertoEscuchaInterrupt = config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT"),
        .ipMemoria = config_get_string_value(config, "IP_MEMORIA"),
        .puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA"),
        .cantidadEntradasTlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB"),
        .algoritmoTlb = config_get_string_value(config, "ALGORITMO_TLB"),
    };
    return cpu;
};

void cpu_log(t_cpu cpu, t_log *logger)
{
    log_info(logger, "PUERTO_ESCUCHA_DISPATCH: %d", cpu.puertoEscuchaDispatch);
    log_info(logger, "PUERTO_ESCUCHA_INTERRUPT: %d", cpu.puertoEscuchaInterrupt);
    log_info(logger, "IP_MEMORIA: %d", cpu.puertoMemoria);
    log_info(logger, "PUERTO_MEMORIA: %d", cpu.puertoEscuchaInterrupt);
    log_info(logger, "CANTIDAD_ENTRADAS_TLB: %d", cpu.cantidadEntradasTlb);
    log_info(logger, "ALGORITMO_TLB: %s", cpu.algoritmoTlb);
    printf("\n");
};

void cpu_memoria_pedir_proxima_instruccion(t_cpu_proceso *proceso, int socket_memoria)
{
    t_paquete *paquete = crear_paquete(CPU_MEMORIA_PROXIMA_INSTRUCCION);
    actualizar_buffer(paquete, sizeof(uint32_t) + sizeof(uint32_t));
    serializar_uint32_t(proceso->registros.pc, paquete);
    serializar_uint32_t(proceso->pid, paquete);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
}

int cpu_ejecutar_instruccion(t_cpu cpu_paquete, t_memoria_cpu_instruccion *datos_instruccion, t_instruccion instruccion, t_cpu_proceso *cpu_proceso, t_log *logger)
{

    // Todas instrucciones vienen con un salto de linea en el ultimo argumento, lo remuevo para que funcionen las comparaciones
    remover_salto_linea(datos_instruccion->array[datos_instruccion->cantidad_elementos - 1]);

    /* TODO: Refactorear este log para que cumpla la misma funcion de una forma mas limpia. Daba segmentation fault el log original porque
    cuando llegaba una instruccion de un solo elemento y queria forzar a imprimir array[2] estaba accediendo a una posicion
    de memoria que no era del array.
    */

    if (datos_instruccion->cantidad_elementos == 1)
    {
        log_info(logger, "PID: <%d> - Ejecutando: <%s>", cpu_proceso->pid, datos_instruccion->array[0]);
    }

    if (datos_instruccion->cantidad_elementos == 2)
    {
        log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s>", cpu_proceso->pid, datos_instruccion->array[0], datos_instruccion->array[1]);
    }

    if (datos_instruccion->cantidad_elementos == 3)
    {
        log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s>", cpu_proceso->pid, datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2]);
    }

    if (datos_instruccion->cantidad_elementos == 4)
    {
        log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s>", cpu_proceso->pid, datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2], datos_instruccion->array[3]);
    }

    if (datos_instruccion->cantidad_elementos == 5)
    {
        log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s> - <%s>", cpu_proceso->pid, datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2], datos_instruccion->array[3], datos_instruccion->array[4]);
    }

    if (datos_instruccion->cantidad_elementos == 6)
    {
        log_info(logger, "PID: <%d> - Ejecutando: <%s> - <%s> - <%s> - <%s> - <%s> - <%s>", cpu_proceso->pid, datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2], datos_instruccion->array[3], datos_instruccion->array[4], datos_instruccion->array[5]);
    }

    switch (instruccion)
    {
    case SET:
    {
        // SET (Registro, Valor): Asigna al registro el valor pasado como parámetro.
        log_debug(logger, "Instruccion: '%s' '%s' '%s' reconocida", datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2]);

        char *registro_destino = strdup(datos_instruccion->array[1]);
        char *valor = strdup(datos_instruccion->array[2]);

        t_registro registro_tipo = obtener_tipo_registro(registro_destino);

        if (registro_tipo == REGISTRO_32)
        {
            uint32_t *registro = determinar_tipo_registro_uint32_t(registro_destino, cpu_proceso);
            if (registro == 0)
            {
                log_error(logger, "Registro %s a insertar invalido", registro_destino);
                return -1;
            }
            uint32_t valor_a_enviar = strtoul(valor, NULL, 10); // Se usa strtoul ya que es la única forma viable de hacer el pasaje de char* a uint y sus variantes
            if (valor_a_enviar > UINT32_MAX)                    // En base a lo anterior, se hace el chequeo del tamaño para evitar el overflow
            {
                log_error(logger, "El numero es mas grande que la capacidad de uint32");
                return -1;
            }
            *registro = valor_a_enviar; // Se asigna el valor obtenido;
        }
        else if (registro_tipo == REGISTRO_8) // Misma lógica que para registro de 4 bytes;
        {
            uint8_t *registro = determinar_tipo_registro_uint8_t(registro_destino, cpu_proceso);
            if (registro == 0)
            {
                log_error(logger, "Registro %s a insertar invalido", registro_destino);
                return -1;
            }
            unsigned long num = strtoul(valor, NULL, 10);
            if (num > UINT8_MAX)
            {
                log_error(logger, "El numero es mas grande que la capacidad de uint8");
                return -1;
            }
            uint8_t valor_a_enviar = (uint8_t)num;
            *registro = valor_a_enviar;
            log_debug(logger, "Valor final del registro %s: %d", registro_destino, *registro);
        }
        else
        {
            log_error(logger, "No existe registro con ese tipo de dato");
        }
        imprimir_registros(logger, cpu_proceso);

        free(registro_destino);
        free(valor);
        return 0;
    }
    case SUM:
    {
        log_debug(logger, "Instruccion: '%s' '%s' '%s' reconocida", datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2]);

        char *argumento_destino = strdup(datos_instruccion->array[1]);
        char *argumento_origen = strdup(datos_instruccion->array[2]);

        // Determino si es un registro de 32 o 8 bits
        t_registro argumento_destino_tipo = obtener_tipo_registro(argumento_destino);
        t_registro argumento_origen_tipo = obtener_tipo_registro(argumento_origen);

        log_debug(logger, "Argumento destino tipo: %d", argumento_destino_tipo);
        log_debug(logger, "Argumento origen tipo: %d", argumento_origen_tipo);

        if (argumento_destino_tipo != INVALIDO && argumento_origen_tipo != INVALIDO)
        {

            if (argumento_destino_tipo == argumento_origen_tipo)
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_destino_anterior = *registro_destino;
                    *registro_destino = *registro_destino + *registro_origen;

                    log_debug(logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);

                    log_debug(logger, "Registros del proceso %d", cpu_proceso->pid);

                    imprimir_registros(logger, cpu_proceso);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino %s no encontrado", argumento_destino);
                        return -1;
                    }

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen %s no encontrado", argumento_origen);
                        return -1;
                    }
                    uint32_t registro_destino_anterior = *registro_destino;
                    *registro_destino += *registro_origen;

                    log_debug(logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);
                }
            }
            else
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {

                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_destino_anterior = *registro_destino;

                    uint32_t registro_origen_casteado = casteo_uint32_t(*registro_origen);

                    *registro_destino = *registro_destino + registro_origen_casteado;

                    log_debug(logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(logger, "Registros del proceso %d", cpu_proceso->pid);

                    imprimir_registros(logger, cpu_proceso);

                    log_debug(logger, "Argumento destino tipo: %d", argumento_destino_tipo);
                }
                if (argumento_destino_tipo == REGISTRO_8)
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint8_t registro_destino_anterior = *registro_destino;
                    uint8_t registro_origen_casteado;

                    if (*registro_origen > 0)
                    {
                        registro_origen_casteado = casteo_uint8_t(*registro_origen);

                        if (registro_origen_casteado == 0)
                        {
                            log_error(logger, "No se puede realizar la suma, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                            return -1;
                        }
                    }
                    else
                    { // Dado que son unsigned, son siempre positivos. Entonces, el unico caso que queda es que sea 0.
                        registro_origen_casteado = 0;
                    }

                    if (*registro_destino + registro_origen_casteado > UINT8_MAX)
                    {
                        log_error(logger, "No se puede realizar la suma, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                        return -1;
                    }

                    *registro_destino = *registro_destino + registro_origen_casteado;

                    log_debug(logger, "Suma realizada: %d + %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(logger, "Registros del proceso %d", cpu_proceso->pid);

                    imprimir_registros(logger, cpu_proceso);

                    log_debug(logger, "Argumento destino tipo: %d", argumento_destino_tipo);
                }
                break;
            }
        }
        else
        {
            log_error(logger, "Se reconocieron los registros %s y %s. No se pueden sumar porque al menos uno no es válido.", argumento_destino, argumento_origen);
            return -1;
        }

        free(argumento_destino);
        free(argumento_origen);
        return 0;
    }
    case SUB:
    {
        log_debug(logger, "Instruccion: '%s' '%s' '%s' reconocida", datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2]);

        char *argumento_destino = strdup(datos_instruccion->array[1]);
        char *argumento_origen = strdup(datos_instruccion->array[2]);

        // Determino si es un registro de 32 o 8 bits
        t_registro argumento_destino_tipo = obtener_tipo_registro(argumento_destino);
        t_registro argumento_origen_tipo = obtener_tipo_registro(argumento_origen);

        if (argumento_destino_tipo != INVALIDO && argumento_origen_tipo != INVALIDO)
        {
            if (argumento_destino_tipo == argumento_origen_tipo)
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento origen %s", argumento_origen);

                    if (*registro_destino < *registro_origen)
                    {
                        log_error(logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= *registro_origen;

                    log_debug(logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);

                    log_debug(logger, "Registros del proceso %d", cpu_proceso->pid);

                    imprimir_registros(logger, cpu_proceso);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino %s no encontrado", argumento_destino);
                        return -1;
                    }

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen %s no encontrado", argumento_origen);
                        return -1;
                    }

                    if (*registro_destino < *registro_origen)
                    {
                        log_error(logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino = *registro_destino - *registro_origen;

                    log_debug(logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, *registro_origen, *registro_destino);
                }
            }
            else
            {
                if (argumento_destino_tipo == REGISTRO_32)
                {
                    uint32_t *registro_destino = determinar_tipo_registro_uint32_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint8_t *registro_origen = determinar_tipo_registro_uint8_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint32_t registro_origen_casteado = casteo_uint32_t(*registro_origen);

                    if (*registro_destino < registro_origen_casteado)
                    {
                        log_error(logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= registro_origen_casteado;

                    log_debug(logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(logger, "Registros del proceso %d", cpu_proceso->pid);

                    imprimir_registros(logger, cpu_proceso);
                }
                else
                {
                    uint8_t *registro_destino = determinar_tipo_registro_uint8_t(argumento_destino, cpu_proceso);

                    if (registro_destino == NULL)
                    {
                        log_error(logger, "Registro destino invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento destino %s", argumento_destino);

                    uint32_t *registro_origen = determinar_tipo_registro_uint32_t(argumento_origen, cpu_proceso);

                    if (registro_origen == NULL)
                    {
                        log_error(logger, "Registro origen invalido");
                        return -1;
                    }

                    log_debug(logger, "Se reconocio el argumento origen %s", argumento_origen);

                    uint8_t registro_origen_casteado;

                    if (*registro_origen > 0)
                    {
                        registro_origen_casteado = casteo_uint8_t(*registro_origen);

                        if (registro_origen_casteado == 0)
                        {
                            log_error(logger, "No se puede realizar la resta, el valor a castear es mayor a 255 por lo cual no es compatible con uint8_t");
                            return -1;
                        }
                    }
                    else
                    { // Dado que son unsigned, son siempre positivos. Entonces, el unico caso que queda es que sea 0.
                        registro_origen_casteado = 0;
                    }

                    if (*registro_destino < registro_origen_casteado)
                    {
                        log_error(logger, "No se puede realizar la resta, el registro destino %s es menor al registro origen %s", argumento_destino, argumento_origen);
                        return -1;
                    }

                    uint8_t registro_destino_anterior = *registro_destino;
                    *registro_destino -= registro_origen_casteado;

                    log_debug(logger, "Resta realizada: %d - %d = %d", registro_destino_anterior, registro_origen_casteado, *registro_destino);

                    log_debug(logger, "Registros del proceso %d", cpu_proceso->pid);

                    imprimir_registros(logger, cpu_proceso);
                }
            }
        }
        else
        {
            log_error(logger, "Se reconocieron los registros %s y %s. No se pueden sumar porque al menos uno no es válido.", argumento_destino, argumento_origen);
            return -1;
        }

        free(argumento_destino);
        free(argumento_origen);
        return 0;
    }
    case JNZ:
    {
        // JNZ (Registro, Instrucción): Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro.
        char *registro = strdup(datos_instruccion->array[1]);
        char *instruccion = strdup(datos_instruccion->array[2]);

        t_registro registro_tipo_size = obtener_tipo_registro(registro);

        if (registro_tipo_size == REGISTRO_8)
        {
            uint8_t *registro_destino = determinar_tipo_registro_uint8_t(registro, cpu_proceso);
            if (*registro_destino != 0)
            {
                uint32_t valor_pc = strtoul(instruccion, NULL, 10);
                cpu_proceso->registros.pc = valor_pc;
            }
        }
        else if (registro_tipo_size == REGISTRO_32)
        {
            uint32_t *registro_destino = determinar_tipo_registro_uint32_t(registro, cpu_proceso);
            if (*registro_destino != 0)
            {
                uint32_t valor_pc = strtoul(instruccion, NULL, 10);
                cpu_proceso->registros.pc = valor_pc;
            }
        }
        else
        {
            log_error(logger, "Registro invalido");
            return -1;
        }
        return 0;
    }
    case IO_GEN_SLEEP:
    {
        // IO_GEN_SLEEP (Interfaz, Unidades de trabajo): Esta instrucción solicita al Kernel que se envíe a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo.

        char *interfaz = strdup(datos_instruccion->array[1]);
        uint32_t tiempo = atoi(datos_instruccion->array[2]);

        log_debug(logger, "Instruccion: '%s' '%s' '%s' reconocida", datos_instruccion->array[0], datos_instruccion->array[1], datos_instruccion->array[2]);

        log_debug(logger, "Interfaz: %s", interfaz);
        log_debug(logger, "Tiempo: %d", tiempo);

        log_debug(logger, "Se envia un sleep de %d a la Interfaz IO Generic ID %s", tiempo, interfaz);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_IO_GEN_SLEEP);

        t_cpu_kernel_io_gen_sleep *unidad = malloc(sizeof(t_cpu_kernel_io_gen_sleep));
        unidad->tiempo = tiempo;
        unidad->size_interfaz = strlen(interfaz) + 1;
        unidad->interfaz = strdup(interfaz);
        unidad->pid = cpu_proceso->pid;
        unidad->registros = cpu_proceso->registros;

        serializar_t_cpu_kernel_io_gen_sleep(&paquete, unidad);

        enviar_paquete(paquete, cpu_paquete.socket_kernel_interrupt);

        free(unidad->interfaz);
        free(unidad);
        eliminar_paquete(paquete);
        return 1;
    }
    case MOV_IN:
    {
        log_debug(logger, "reconoci un MOV_IN");
        break;
    }
    case MOV_OUT:
    {
        log_debug(logger, "reconoci un MOV_OUT");
        break;
    }
    case RESIZE:
    {
        log_debug(logger, "reconoci un RESIZE");
        break;
    }
    case COPY_STRING:
    {
        log_debug(logger, "reconoci un COPY_STRING");
        break;
    }
    case IO_STDIN_READ:
    {
        log_debug(logger, "reconoci un IO_STDIN_READ");
        return 1;
    }
    case IO_STDOUT_WRITE:
    {
        log_debug(logger, "reconoci un IO_STDOUT_WRITE");
        return 1;
    }
    case IO_FS_CREATE:
    {
        log_debug(logger, "reconoci un IO_FS_CREATE");
        return 1;
    }
    case IO_FS_DELETE:
    {
        log_debug(logger, "reconoci un IO_FS_DELETE");
        return 1;
    }
    case IO_FS_TRUNCATE:
    {
        log_debug(logger, "reconoci un IO_FS_TRUNCATE");
        return 1;
    }
    case IO_FS_WRITE:
    {
        log_debug(logger, "reconoci un IO_FS_WRITE");
        return 1;
    }
    case IO_FS_READ:
    {
        log_debug(logger, "reconoci un IO_FS_READ");
        return 1;
    }
    case WAIT:
    {
        char *nombre_recurso = strdup(datos_instruccion->array[1]);
        log_debug(logger, "Recurso a esperar: %s", nombre_recurso);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_WAIT);

        t_cpu_kernel_solicitud_recurso *contexto = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        contexto->pid = cpu_proceso->pid;
        contexto->registros = &(cpu_proceso->registros);
        contexto->size_nombre_recurso = strlen(nombre_recurso) + 1;
        contexto->nombre_recurso = strdup(nombre_recurso);

        serializar_t_cpu_kernel_solicitud_recurso(&paquete, contexto);

        enviar_paquete(paquete, cpu_paquete.socket_kernel_dispatch);

        free(nombre_recurso);
        free(contexto->nombre_recurso);
        free(contexto);

        eliminar_paquete(paquete);

        return 2; // 2 significa que el proceso debe ser desalojado por haberse ejecutado wait/signal
    }
    case SIGNAL:
    {
        char *nombre_recurso = strdup(datos_instruccion->array[1]);
        log_debug(logger, "Recurso a liberar: %s", nombre_recurso);

        t_paquete *paquete = crear_paquete(CPU_KERNEL_SIGNAL);

        t_cpu_kernel_solicitud_recurso *contexto = malloc(sizeof(t_cpu_kernel_solicitud_recurso));
        contexto->pid = cpu_proceso->pid;
        contexto->registros = &(cpu_proceso->registros);
        contexto->size_nombre_recurso = strlen(nombre_recurso) + 1;
        contexto->nombre_recurso = strdup(nombre_recurso);

        serializar_t_cpu_kernel_solicitud_recurso(&paquete, contexto);

        enviar_paquete(paquete, cpu_paquete.socket_kernel_dispatch);

        free(nombre_recurso);
        free(contexto->nombre_recurso);
        free(contexto);
        eliminar_paquete(paquete);

        return 2;
    }
    default:
    {
        log_error(logger, "Instruccion invalida");
        return 0;
    }
    }
    return 0;
};

int cpu_memoria_recibir_instruccion(t_buffer *buffer, t_log *logger, t_memoria_cpu_instruccion *datos_instruccion, t_instruccion *instruccion, t_cpu_proceso *proceso)
{
    t_memoria_cpu_instruccion *dato = deserializar_t_memoria_cpu_instruccion(buffer);
    log_info(logger, "PID : <%d> - FETCH - Program Counter : <%d>", proceso->pid, proceso->registros.pc); // Log oblitario
    *instruccion = determinar_codigo_instruccion(dato->array[0]);
    proceso->registros.pc += 1;
    if (*instruccion == -1)
    {
        log_error(logger, "Instruccion invalida");
        return -1;
    }
    if (*instruccion == EXIT)
    {
        log_debug(logger, "Reconoci el EXIT. Termino.");
        return 1;
    }
    *datos_instruccion = *dato;
    free(dato);
    return 0;
};

t_cpu_proceso cpu_kernel_recibir_proceso(t_buffer *buffer, t_log *logger)
{
    t_kernel_cpu_proceso *proceso_cpu = deserializar_t_kernel_cpu_proceso(buffer);
    log_debug(logger, "Registros cpu recibido de Kernel Dispatch");
    log_debug(logger, "PID: %d", proceso_cpu->pid);
    log_debug(logger, "PC: %d", proceso_cpu->registros.pc);
    log_debug(logger, "EAX: %d", proceso_cpu->registros.eax);
    log_debug(logger, "EBX: %d", proceso_cpu->registros.ebx);
    log_debug(logger, "ECX: %d", proceso_cpu->registros.ecx);
    log_debug(logger, "EDX: %d", proceso_cpu->registros.edx);
    log_debug(logger, "AX: %d", proceso_cpu->registros.ax);
    log_debug(logger, "BX: %d", proceso_cpu->registros.bx);
    log_debug(logger, "CX: %d", proceso_cpu->registros.cx);
    log_debug(logger, "DX: %d", proceso_cpu->registros.dx);
    log_debug(logger, "SI: %d", proceso_cpu->registros.si);
    log_debug(logger, "DI: %d", proceso_cpu->registros.di);

    /*GUARDAR REGISTROS*/
    t_cpu_proceso proceso = {
        .pid = proceso_cpu->pid,
        .registros = {
            .pc = proceso_cpu->registros.pc,
            .eax = proceso_cpu->registros.eax,
            .ebx = proceso_cpu->registros.ebx,
            .ecx = proceso_cpu->registros.ecx,
            .edx = proceso_cpu->registros.edx,
            .ax = proceso_cpu->registros.ax,
            .bx = proceso_cpu->registros.bx,
            .cx = proceso_cpu->registros.cx,
            .dx = proceso_cpu->registros.dx,
            .si = proceso_cpu->registros.si,
            .di = proceso_cpu->registros.di,
        },
    };
    return proceso;
};

void cpu_kernel_avisar_finalizacion(t_cpu_proceso proceso, int socket_kernel_dispatch)
{
    t_paquete *paquete_proceso = crear_paquete(CPU_KERNEL_PROCESO);
    t_cpu_kernel_proceso *fin_proceso = malloc(sizeof(t_cpu_kernel_proceso));
    fin_proceso->registros = malloc(sizeof(t_registros_cpu));

    fin_proceso->ejecutado = 1;
    fin_proceso->pid = proceso.pid;
    fin_proceso->registros->pc = proceso.registros.pc;
    fin_proceso->registros->eax = proceso.registros.eax;
    fin_proceso->registros->ebx = proceso.registros.ebx;
    fin_proceso->registros->ecx = proceso.registros.ecx;
    fin_proceso->registros->edx = proceso.registros.edx;
    fin_proceso->registros->si = proceso.registros.si;
    fin_proceso->registros->di = proceso.registros.di;
    fin_proceso->registros->ax = proceso.registros.ax;
    fin_proceso->registros->bx = proceso.registros.bx;
    fin_proceso->registros->cx = proceso.registros.cx;
    fin_proceso->registros->dx = proceso.registros.dx;

    serializar_t_cpu_kernel_proceso(&paquete_proceso, fin_proceso);
    enviar_paquete(paquete_proceso, socket_kernel_dispatch);

    eliminar_paquete(paquete_proceso);
    free(fin_proceso->registros);
    free(fin_proceso);
};

uint32_t *determinar_tipo_registro_uint32_t(char *instruccion, t_cpu_proceso *proceso)
{
    if (!strcmp(instruccion, "PC"))
    {
        return &proceso->registros.pc;
    }
    if (!strcmp(instruccion, "EAX"))
    {
        return &proceso->registros.eax;
    }
    if (!strcmp(instruccion, "EBX"))
    {
        return &proceso->registros.ebx;
    }
    if (!strcmp(instruccion, "ECX"))
    {
        return &proceso->registros.ecx;
    }
    if (!strcmp(instruccion, "EDX"))
    {
        return &proceso->registros.edx;
    }
    if (!strcmp(instruccion, "SI"))
    {
        return &proceso->registros.si;
    }
    if (!strcmp(instruccion, "DI"))
    {
        return &proceso->registros.di;
    }
    return NULL;
};

uint8_t *determinar_tipo_registro_uint8_t(char *instruccion, t_cpu_proceso *proceso)
{
    if (!strcmp(instruccion, "AX"))
    {
        return &proceso->registros.ax;
    }
    if (!strcmp(instruccion, "BX"))
    {
        return &proceso->registros.bx;
    }
    if (!strcmp(instruccion, "CX"))
    {
        return &proceso->registros.cx;
    }
    if (!strcmp(instruccion, "DX"))
    {
        return &proceso->registros.dx;
    }
    return NULL;
};

t_instruccion determinar_codigo_instruccion(char *instruccion)
{
    // Remover \n
    remover_salto_linea(instruccion);

    /* Todas las posibles instrucciones:
    {
        SET,
        SUM,
        SUB,
        JNZ,
        IO_GEN_SLEEP,
        MOV_IN,
        MOV_OUT,
        RESIZE,
        COPY_STRING,
        IO_STDIN_READ,
        IO_STDOUT_WRITE,
        IO_FS_CREATE,
        IO_FS_DELETE,
        IO_FS_TRUNCATE,
        IO_FS_WRITE,
        IO_FS_READ,
        WAIT,
        SIGNAL,
        EXIT

    */

    if (strcmp(instruccion, "SET") == 0)
    {
        return SET;
    }
    if (strcmp(instruccion, "SUM") == 0)
    {
        return SUM;
    }
    if (strcmp(instruccion, "SUB") == 0)
    {
        return SUB;
    }
    if (strcmp(instruccion, "JNZ") == 0)
    {
        return JNZ;
    }
    if (strcmp(instruccion, "IO_GEN_SLEEP") == 0)
    {
        return IO_GEN_SLEEP;
    }
    if (strcmp(instruccion, "EXIT") == 0)
    {
        return EXIT;
    }
    if (strcmp(instruccion, "MOV_IN") == 0)
    {
        return MOV_IN;
    }
    if (strcmp(instruccion, "MOV_OUT") == 0)
    {
        return MOV_OUT;
    }
    if (strcmp(instruccion, "RESIZE") == 0)
    {
        return RESIZE;
    }
    if (strcmp(instruccion, "COPY_STRING") == 0)
    {
        return COPY_STRING;
    }
    if (strcmp(instruccion, "IO_STDIN_READ") == 0)
    {
        return IO_STDIN_READ;
    }
    if (strcmp(instruccion, "IO_STDOUT_WRITE") == 0)
    {
        return IO_STDOUT_WRITE;
    }
    if (strcmp(instruccion, "IO_FS_CREATE") == 0)
    {
        return IO_FS_CREATE;
    }
    if (strcmp(instruccion, "IO_FS_DELETE") == 0)
    {
        return IO_FS_DELETE;
    }
    if (strcmp(instruccion, "IO_FS_TRUNCATE") == 0)
    {
        return IO_FS_TRUNCATE;
    }
    if (strcmp(instruccion, "IO_FS_WRITE") == 0)
    {
        return IO_FS_WRITE;
    }
    if (strcmp(instruccion, "IO_FS_READ") == 0)
    {
        return IO_FS_READ;
    }
    if (strcmp(instruccion, "WAIT") == 0)
    {
        return WAIT;
    }
    if (strcmp(instruccion, "SIGNAL") == 0)
    {
        return SIGNAL;
    }
    return -1;
};

t_registro obtener_tipo_registro(char *nombre_registro)
{
    if (!strcmp(nombre_registro, "PC"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "EAX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "EBX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "ECX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "EDX"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "AX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "BX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "CX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "DX"))
    {
        return REGISTRO_8;
    }
    if (!strcmp(nombre_registro, "DI"))
    {
        return REGISTRO_32;
    }
    if (!strcmp(nombre_registro, "SI"))
    {
        return REGISTRO_32;
    }
    else
    {
        return INVALIDO;
    }
}

void imprimir_registros(t_log *logger, t_cpu_proceso *cpu_proceso)
{
    log_debug(logger, "Registros del proceso %d", cpu_proceso->pid);
    log_debug(logger, "PC: %d", cpu_proceso->registros.pc);
    log_debug(logger, "EAX: %d", cpu_proceso->registros.eax);
    log_debug(logger, "EBX: %d", cpu_proceso->registros.ebx);
    log_debug(logger, "ECX: %d", cpu_proceso->registros.ecx);
    log_debug(logger, "EDX: %d", cpu_proceso->registros.edx);
    log_debug(logger, "SI: %d", cpu_proceso->registros.si);
    log_debug(logger, "DI: %d", cpu_proceso->registros.di);
    log_debug(logger, "AX: %d", cpu_proceso->registros.ax);
    log_debug(logger, "BX: %d", cpu_proceso->registros.bx);
    log_debug(logger, "CX: %d", cpu_proceso->registros.cx);
    log_debug(logger, "DX: %d", cpu_proceso->registros.dx);
}

void remover_salto_linea(char *argumento_origen)
{
    if (argumento_origen[strlen(argumento_origen) - 1] == '\n')
    {
        argumento_origen[strlen(argumento_origen) - 1] = '\0';
    }
}

bool casteo_verificar_uint_8t_valido(uint32_t valor)
{
    return valor <= UINT8_MAX;
}

uint8_t casteo_uint8_t(uint32_t valor)
{
    if (!casteo_verificar_uint_8t_valido(valor))
    {
        return 0;
    }
    return (uint8_t)valor;
}

uint32_t casteo_uint32_t(uint8_t valor)
{
    return (uint32_t)valor;
}

int cpu_recibir_interrupcion(t_log *logger, t_buffer *buffer, t_cpu_proceso proceso)
{
    t_kernel_cpu_interrupcion *interrupcion = deserializar_t_kernel_cpu_interrupcion(buffer);
    if (interrupcion->pid == proceso.pid)
    {
        log_debug(logger, "Se detecto una solicitud de interrupcion para el proceso de PID: %d. Actualizando flag de interrupt...", interrupcion->pid);
        free(interrupcion);
        return 1;
    }
    else
    {
        log_debug(logger, "El PID recibido (%d) no se corresponde con el que se esta ejecutando (%d). Se ignora la interrupcion.", interrupcion->pid, proceso.pid);
        free(interrupcion);
        return 0;
    }
}

void cpu_procesar_interrupt(t_log *logger, t_cpu cpu, t_cpu_proceso proceso)
{
    log_debug(logger, "Se atiende interrupcion");

    t_cpu_kernel_proceso *proceso_interrumpido = malloc(sizeof(t_cpu_kernel_proceso));
    proceso_interrumpido->ejecutado = 0; // Desalojado por interrupcion
    proceso_interrumpido->pid = proceso.pid;
    proceso_interrumpido->registros = &proceso.registros;

    // TODO: preguntar si este es el opcode que se usa
    t_paquete *paquete = crear_paquete(CPU_KERNEL_PROCESO);
    serializar_t_cpu_kernel_proceso(&paquete, proceso_interrumpido);
    // TODO: esto cambiarlo a interrupt en el testeo
    enviar_paquete(paquete, cpu.socket_kernel_dispatch);
}