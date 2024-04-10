#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>

/*--------KERNEL--------*/


/*Estructura basica del kernel*/
typedef struct Kernel{
    int puertoEscucha;
    char* ipMemoria;
    int puertoMemoria;
    char* ipCpu;
    int puertoCpuDispatch;
    int puertoCpuInterrupt;
    char* algoritmoPlanificador;
    int quantum;
    char* recursos;
    char* instanciasRecursos;
    int gradoMultiprogramacion;
} Kernel;

/**
* @fn    inicializar_kernel
* @brief Inicializa el kernel junto con todas sus configuraciones
* @param config Instancia de module.config
* @return Kernel: Struct de kernel
*/	
Kernel inicializar_kernel(t_config *config);

/**
* @fn    log_kernel
* @brief Log obligatorios de kernel junto con su configuracion
* @param kernel Instancia de kernel (inicializar_kernel)
* @param logger Instancia de t_log
*/	
void log_kernel(Kernel kernel,t_log logger){
    log_info(logger,"PUERTO_ESCUCHA: %d",kernel.puertoEscucha);
	log_info(logger,"IP_MEMORIA: %s",kernel.ipMemoria);
	log_info(logger,"PUERTO_MEMORIA: %d",kernel.puertoMemoria);
    log_info(logger,"IP_CPU: %s",kernel.ipCpu);
    log_info(logger,"PUERTO_CPU_DISPATCH: %d",kernel.puertoCpuDispatch);
    log_info(logger,"PUERTO_CPU_INTERRUPT: %d",kernel.puertoCpuInterrupt);
    log_info(logger,"ALGORITMO_PLANIFICADOR: %s",kernel.algoritmoPlanificador);
    log_info(logger,"QUANTUM: %d",kernel.quantum);
    log_info(logger,"RECURSOS: %s",kernel.recursos);
    log_info(logger,"INSTANCIAS_RECURSOS: %s",kernel.instanciasRecursos);
    log_info(logger,"GRADO_MULTIPROGRAMACION: %d",kernel.gradoMultiprogramacion);
};


// macros para funciones de casting usando structs
#define CASTING(T)  \
void casting_##T(struct T *arg, void **casted) { \
    *casted = (void*) arg;   \
}

#define DECASTING(T) \
void decasting_##T(struct T** arg , void* decasted){ \
    *arg = (struct T* ) decasted; \
}

// Ejemplo de implementación 
//struct dato{
//    int dato1;
//};
//
//CASTING(dato);
//DECASTING(dato);


// Ejemplo de uso 
//
//pthread_t hilo1;
//
//struct dato v1 = {1 , 'g' , 1}; // parámetro que recibe la función 
//void* c1 = NULL;  // puntero a void usado en el casting
//casting_dato(&v1 , &c1); 
//pthread_create(&hilo1 , NULL , func1 , c1);
//pthread_detach(hilo1);




