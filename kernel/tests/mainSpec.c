#ifndef MAINSPEC_KERNEL_H_
#define MAINSPEC_KERNEL_H_
#include <utils/kernel.h>
#include <cspecs/cspec.h>

context(test_kernel)
{
    t_log *logger = log_create("test_kernel.log", "test_kernel", true, LOG_LEVEL_DEBUG);
    t_diagrama_estados estados;
    estados = kernel_inicializar_estados(&estados);
    describe("Manejo cola de procesos")
    {
        it("Se agregan 3 procesos a new")
        {
            t_pcb *proceso1 = pcb_crear(logger, 2000);
            t_pcb *proceso2 = pcb_crear(logger, 2000);
            t_pcb *proceso3 = pcb_crear(logger, 2000);
            proceso_push_new(&estados, proceso1);
            proceso_push_new(&estados, proceso2);
            proceso_push_new(&estados, proceso3);

            should_int(proceso1->pid) be equal to(1);
            should_int(proceso2->pid) be equal to(2);
            should_int(proceso3->pid) be equal to(3);
            should_int(list_size(estados.new)) be equal to(3);
        }
        end;
        it("Verifica que los 3 PIDs esten en new")
        {
            t_pcb *proceso1 = proceso_buscar_new(&estados, 1);
            t_pcb *proceso2 = proceso_buscar_new(&estados, 2);
            t_pcb *proceso3 = proceso_buscar_new(&estados, 3);
            should_int(proceso1->pid) be equal to(1);
            should_int(proceso2->pid) be equal to(2);
            should_int(proceso3->pid) be equal to(3);
        }
        end;
        it("Muevo un proceso de new a ready")
        {
            t_pcb *proceso1 = proceso_pop_new(&estados);
            proceso_push_ready(&estados, proceso1, logger);
            // Tamaño de la cola de new tiene que ser 2
            should_int(list_size(estados.new)) be equal to(2);
            // Tamaño de la cola de ready tiene que ser 1
            should_int(list_size(estados.ready)) be equal to(1);
            should_string(proceso_estado(&estados, 1)) be equal to("READY");
            should_string(proceso_estado(&estados, 2)) be equal to("NEW");
            should_string(proceso_estado(&estados, 3)) be equal to("NEW");
        }
        end;
        it("Matar proceso número 2 en estado NEW")
        {
            proceso_matar(&estados, "2");
            should_int(list_size(estados.new)) be equal to(1);
        }
        end;
        it("Transicion ready->exec")
        {
            t_pcb *proceso = proceso_transicion_ready_exec(&estados);
            should_int(list_size(estados.ready)) be equal to(0);
            should_int(list_size(estados.exec)) be equal to(1);
        }
        end;
        it("Transicion exec->block")
        {
            t_pcb *proceso = proceso_transicion_exec_block(&estados);
            should_int(list_size(estados.exec)) be equal to(0);
            should_int(list_size(estados.block)) be equal to(1);
        }
        end;
        it("Transicion block->ready")
        {
            t_pcb *proceso = proceso_transicion_block_ready(&estados, logger);
            should_int(list_size(estados.block)) be equal to(0);
            should_int(list_size(estados.ready)) be equal to(1);
        }
        end;
    }
    end;
}

#endif
