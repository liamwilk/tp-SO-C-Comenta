#include "generic.h"

// void atender_entrada_salida_generic(t_entradasalida package, t_log *logger, char *nombre_modulo)
// {
//     log_debug(logger, "Iniciando modulo %s", nombre_modulo);

//     log_debug(logger, "Conectando a Kernel en %s:%d", package.ipKernel, package.puertoKernel);
//     int socket_kernel = crear_conexion(logger, package.ipKernel, package.puertoKernel);
    
//     if (socket_kernel == -1)
//     {
//         log_error(logger, "No se pudo crear la conexion con Kernel.");
//         return;
//     }
    
//     t_handshake resultado = crear_handshake(logger, socket_kernel, KERNEL_ENTRADA_SALIDA_GENERIC, "Kernel");
    
//     if (resultado != CORRECTO)
//     {
//         log_error(logger, "No se pudo realizar el handshake con Kernel.");
//         liberar_conexion(socket_kernel);
//         return;
//     }

//     identificar_modulo(nombre_modulo, socket_kernel);

//     log_debug(logger, "Conectado a Kernel en socket %d", socket_kernel);

//     int seguir_ejecutando = 1;

//     while (seguir_ejecutando)
//     {
//         log_debug(logger, "Esperando paquete de Kernel en socket %d", socket_kernel);
//         t_paquete *paquete = recibir_paquete(logger, socket_kernel);

//         if (paquete == NULL)
//         {
//             seguir_ejecutando = 0;
//             log_error(logger, "No se pudo recibir el paquete de Kernel.");
//             break;
//         }
        
//         switch (paquete->codigo_operacion)
//         {
//             case IO_GEN_SLEEP:
//             {   
//                 revisar_paquete(paquete, logger, seguir_ejecutando, nombre_modulo);

//                 uint32_t unidades_de_trabajo = deserializar_unidades_de_trabajo(paquete->buffer);

//                 log_info(logger, "PID : <%u> - Operacion a realizar: IO_GEN_SLEEP", process_getpid());
                                
//                 log_debug(logger, "Durmiendo %d unidades de trabajo", unidades_de_trabajo);
//                 usleep(unidades_de_trabajo * package.tiempoUnidadDeTrabajo);

//                 t_paquete *aviso_sleep = crear_paquete(IO_GEN_SLEEP_TERMINADO);
//                 enviar_paquete(aviso_sleep, socket_kernel);
            
//                 eliminar_paquete(aviso_sleep);
//                 break;
//             }
//             default:
//             {
//                 seguir_ejecutando = 0;
//                 log_warning(logger, "[Kernel] Se cerró el socket de conexion del lado de Kernel.", paquete->codigo_operacion);
//                 liberar_conexion(socket_kernel);
//                 break;
//             }
//         }
//         eliminar_paquete(paquete);
//     }
// }

// uint32_t deserializar_unidades_de_trabajo(t_buffer *buffer)
// {
//     uint32_t dato;
// 	void *stream = buffer->stream;
    
//     deserializar_uint32_t(&stream, &dato);
//     return dato;
// }