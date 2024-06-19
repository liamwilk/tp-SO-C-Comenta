#include <utils/cpu.h>

void switch_case_kernel_interrupt(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case KERNEL_CPU_INTERRUPCION:
	{
		recibir_interrupcion(args, buffer);
		break;
	}
	case FINALIZAR_SISTEMA:
	{
		pthread_cancel(args->threads.thread_mmu);
		pthread_cancel(args->threads.thread_atender_memoria);
		pthread_cancel(args->threads.thread_atender_kernel_dispatch);
		pthread_cancel(args->threads.thread_atender_kernel_interrupt);
		liberar_conexion(&args->config_leida.socket_memoria);
		liberar_conexion(&args->config_leida.socket_kernel_dispatch);
		liberar_conexion(&args->config_leida.socket_kernel_interrupt);
		break;
	}
	default:
	{
		log_warning(args->logger, "[Kernel Interrupt] Se recibio un codigo de operacion desconocido. Cierro hilo");
		liberar_conexion(&args->config_leida.socket_kernel_interrupt);
		break;
	}
	}
}