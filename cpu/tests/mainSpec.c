#include <stdio.h>
#include <stdbool.h>
#include <cspecs/cspec.h>
#include <utils/instrucciones.h>

context(test_instrucciones)
{
    describe("Pruebas unitarias de las instrucciones SET, SUM, SUB Y JNZ"){

        describe("Pruebas de la instrucción SET"){

            it("Asigna a un registro de 1 byte el valor recibido"){
                uint8_t a;
    set(&a, 3, sizeof(uint8_t));
    should_bool(a == 3) be equal to(true);
}
end

    it("Asigna a un registro de 4 bytes el valor recibido")
{
    uint32_t ea;
    set(&ea, 3, sizeof(uint32_t));
    should_bool(ea == 3) be equal to(true);
}
end

    it("En caso de recibir otro tamaño distinto a estos, devolverá un exit status 1")
{
    uint32_t ea;
    int exit;
    exit = set(&ea, 3, sizeof(uint64_t));
    should_bool(exit == 1) be equal to(true);
}
end

    describe("Pruebas de la instrucción SUM"){

        it("Suma dos registros de 1 byte cada uno"){
            uint8_t a = 5;
uint8_t b = 3;
sum(&a, &b, sizeof(uint8_t));
should_int(b) be equal to(5 + 3);
}
end

    it("Suma dos registros de 4 bytes cada uno")
{
    uint32_t ea = 5;
    uint32_t eb = 3;
    sum(&ea, &eb, sizeof(uint32_t));
    should_int(eb) be equal to(5 + 3);
}
end

    it("Le suma a un registro de 4 bytes el contenido de uno de 1 byte")
{
    uint32_t ea = 5;
    uint8_t b = 3;
    sum(&ea, &b, sizeof(uint32_t));
    should_int(b) be equal to(5 + 3);
}
end

    it("Le suma a un registro de 1 byte el contenido de uno de 4 bytes")
{
    uint8_t a = 5;
    uint32_t eb = 3;
    sum(&a, &eb, sizeof(uint8_t));
    should_int(eb) be equal to(5 + 3);
}
end

    it("En caso de recibir otro tamaño distinto a los soportados, devolverá un exit status 1")
{
    uint32_t ea = 5;
    uint32_t eb = 3;
    int exit;
    exit = sum(&ea, &eb, sizeof(uint64_t));
    should_bool(exit == 1) be equal to(true);
}
end
}
end

    describe("Pruebas de la instrucción SUB"){

        it("Resta dos registros de 1 byte cada uno"){
            uint8_t a = 3;
uint8_t b = 5;
sub(&a, &b, sizeof(uint8_t));
should_int(b) be equal to(5 - 3);
}
end

    it("Resta dos registros de 4 bytes cada uno")
{
    uint32_t ea = 3;
    uint32_t eb = 5;
    sub(&ea, &eb, sizeof(uint32_t));
    should_int(eb) be equal to(5 - 3);
}
end

    it("Le resta a un registro de 4 bytes el contenido de uno de 1 byte")
{
    uint32_t ea = 3;
    uint8_t b = 5;
    sub(&ea, &b, sizeof(uint32_t));
    should_int(b) be equal to(5 - 3);
}
end

    it("Le resta a un registro de 1 byte el contenido de uno de 4 bytes")
{
    uint8_t a = 3;
    uint32_t eb = 5;
    sub(&a, &eb, sizeof(uint8_t));
    should_int(eb) be equal to(5 - 3);
}
end

    it("En caso de recibir otro tamaño distinto a los soportados, devolverá un exit status 1")
{
    uint32_t ea = 5;
    uint32_t eb = 3;
    int exit;
    exit = sub(&ea, &eb, sizeof(uint64_t));
    should_bool(exit == 1) be equal to(true);
}
end
}
end

    describe("Pruebas de la instrucción JNZ"){

        it("Si el registro no sea igual a 0, no se actualiza el contador de programa"){
            uint32_t ea = 5;
uint32_t pc = 10;
int exit;
exit = jnz(&pc, &ea, 10);
should_bool(exit == 1) be equal to(true);
}
end

    it("Caso contrario, se le asigna el valor entregado")
{
    uint32_t ea = 0;
    uint32_t pc = 10;
    int exit;
    exit = jnz(&pc, &ea, 10);
    should_bool(exit == 0) be equal to(true);
}
end
}
end
}
end
}
end
}