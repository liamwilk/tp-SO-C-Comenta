#include <utils/cpu.h>

void switch_case_kernel_dispatch(t_cpu *args, t_op_code codigo_operacion, t_buffer *buffer)
{
	switch (codigo_operacion)
	{
	case KERNEL_CPU_IO_STDIN_READ:
	{
		t_kernel_cpu_io_stdin_read *proceso_recibido = deserializar_t_kernel_cpu_io_stdin_read(buffer);

		if (proceso_recibido->resultado)
		{

			log_debug(args->logger, "Se completo la instruccion IO_STDIN_READ asociada al proceso PID <%d>.", proceso_recibido->pid);
		}
		else
		{

			log_error(args->logger, "Se produjo un error al ejecutar la instruccion IO_STDIN_READ asociada al proceso PID <%d> en la conexion entre Kernel y la interfaz.", proceso_recibido->pid);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);
		}

		free(proceso_recibido);
		break;
	}
	case KERNEL_CPU_IO_STDOUT_WRITE:
	{
		t_kernel_cpu_io_stdout_write *proceso_recibido = deserializar_t_kernel_cpu_io_stdout_write(buffer);

		if (proceso_recibido->resultado)
		{

			log_debug(args->logger, "Se completo la instruccion IO_STDOUT_WRITE asociada al proceso PID <%d>.", proceso_recibido->pid);
			log_debug(args->logger, "Mensaje recuperado de Kernel: %s", proceso_recibido->motivo);
		}
		else
		{

			log_error(args->logger, "Se produjo un error al ejecutar la instruccion IO_STDOUT_WRITE asociada al proceso PID <%d> en la conexion entre Kernel y la interfaz.", proceso_recibido->pid);

			log_debug(args->logger, "Mensaje recuperado de Kernel: %s", proceso_recibido->motivo);

			args->proceso.ejecutado = 0;
			instruccion_finalizar(args);
		}

		free(proceso_recibido);
		break;
	}
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