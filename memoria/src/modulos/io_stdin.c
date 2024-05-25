#include <utils/memoria.h>

void switch_case_entrada_salida_stdin(t_args *argumentos, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
		case PLACEHOLDER:
		{
			break;
		}
		default:
		{
			log_warning(argumentos->logger, "[STDIN] Se recibio un codigo de operacion desconocido. Cierro hilo");
			liberar_conexion(&argumentos->memoria.sockets.socket_entrada_salida_stdin);
			break;
		}
	}
}