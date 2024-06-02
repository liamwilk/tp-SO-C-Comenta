#include <utils/cpu.h>

void switch_case_kernel_dispatch(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case KERNEL_CPU_EJECUTAR_PROCESO:
	{
		proceso_recibir(args, buffer);
		instruccion_solicitar(args);
		break;
	}
	default:
	{
		log_warning(args->logger, "[Kernel Dispatch] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&args->config_leida.socket_kernel_dispatch);
		break;
	}
	}
}