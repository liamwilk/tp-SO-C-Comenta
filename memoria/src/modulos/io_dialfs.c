#include <utils/memoria.h>

void switch_case_entrada_salida_dialfs(t_args_hilo* argumentos, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
		case MEMORIA_ENTRADA_SALIDA_IDENTIFICACION:
        {
            t_entrada_salida_identificacion *identificacion = deserializar_t_entrada_salida_identificacion(buffer);

            agregar_identificador(argumentos, identificacion->identificador);

            log_debug(argumentos->argumentos->logger, "[IO DialFS/Interfaz %s/Orden %d] Se recibio identificador.",identificacion->identificador, argumentos->entrada_salida->orden);

            break;
        }
		default:
		{
			log_warning(argumentos->argumentos->logger, "[DialFS] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&argumentos->argumentos->memoria.sockets.socket_entrada_salida_stdin);
			break;
		}
	}
}