#include <utils/cpu.h>

t_cpu argumentos;

int main(int argc, char *argv[])
{
	inicializar_modulo_cpu(&argumentos, LOG_LEVEL_INFO, argc, argv);
	return EXIT_SUCCESS;
}