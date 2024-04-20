#include "main.h"

int main()
{
    printf("[TESTS] Corriendo tests...\n");
    test_cpu();
    test_io();
    test_kernel();
    test_memoria();
    return 0;
};