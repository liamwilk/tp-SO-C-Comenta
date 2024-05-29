/* Módulo Entrada-Salida */

#include "main.h"

int main(int argc, char *argv[])
{	
	if(inicializar_modulo_interfaz(&entradasalida, argc, argv) == -1){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}