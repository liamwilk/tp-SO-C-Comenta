#include <utils/memoria.h>

t_args args;

int main(int argc, char *argv[])
{	
	inicializar(&args,LOG_LEVEL_DEBUG,argc,argv);
	return EXIT_SUCCESS;
}