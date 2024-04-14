#include "hilos.h"

pthread_t hilo_crear_d(void *fn, void *args, t_log *logger)
{
    pthread_t h1;
    int val = pthread_create(&h1, NULL, fn, &args);

    if (val < 0)
    {
        log_error(logger, "No se pudo crear el hilo.");
    };

    pthread_detach(h1);
    return h1;
};

void hilo_crear_j(void *fn, void *args, t_log *logger)
{
    pthread_t h1;
    int val_h1 = pthread_create(&h1, NULL, fn, &args);
    if (val_h1 < 0)
    {
        log_error(logger, "No se pudo crear el hilo.");
    };

    int val_join = pthread_join(h1, NULL);

    if (val_join < 0)
    {
        log_error(logger, "No se pudo hacer join al hilo.");
    };
};

void *hilo_handshake(void *args)
{
    t_handshake data = *(t_handshake *)args;
    handshake_crear(&data, data.logger);
    return NULL;
};
