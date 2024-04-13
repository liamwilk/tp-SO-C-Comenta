#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <utils/conexiones.h>
#include <utils/configs.h>
#include <utils/hilos.h>
#include <utils/modulos.h>

void comunicarConCliente(t_paqueteCliente *);
#endif /* MAIN_H_ */