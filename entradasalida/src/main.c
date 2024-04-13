/* Módulo Entrada-Salida */

#include "main.h"

int main()
{
	logger = iniciar_logger("entradasalida");
	config = iniciar_config(logger);

	// Se pasan los datos de config a local
	t_entradasalida entradasalida = entradasalida_inicializar(config);

	// Se loggean los valores de config
	entradasalida_log(entradasalida, logger);

	// Se realizan los handshakes pertinentes
	t_handshake handshakeKernel = entradasalida_handshake_kernel(entradasalida, hilo_handshake, logger);
	t_handshake handshakeMemoria = entradasalida_handshake_memoria(entradasalida, hilo_handshake, logger);

	hilo_exit();

	// Se liberan las conexiones
	liberar_conexion(handshakeKernel.socketDestino);
	liberar_conexion(handshakeMemoria.socketDestino);

	// Se destruyen las estructuras
	config_destroy(config);
	log_destroy(logger);

	return 0;
}