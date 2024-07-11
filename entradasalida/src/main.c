#include <utils/entradasalida.h>

timer_args_io_t temporizador;
t_io entradasalida;

int main(int argc, char *argv[])
{	
	if(inicializar_modulo_interfaz(&entradasalida, argc, argv, &temporizador)){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}