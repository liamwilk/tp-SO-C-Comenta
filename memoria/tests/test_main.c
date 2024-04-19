#include <stdio.h>
#include <utils/tests.h>
#include "../src/main.h"


int test_inicializacion(){
    describe("[MEMORIA] Test de module.config != NULL\n");
    config = iniciar_config(logger_info);
    memoria = memoria_inicializar(config);
    test(sizeof(memoria) > 0);
    return 0;
};

int main() {
    test_inicializacion();
    return 0;
}