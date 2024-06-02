#include <utils/cpu.h>

void instruccion_solicitar(t_cpu *args)
{
    t_paquete *paquete = crear_paquete(CPU_MEMORIA_PROXIMA_INSTRUCCION);
    actualizar_buffer(paquete, sizeof(uint32_t) + sizeof(uint32_t));
    serializar_uint32_t(args->proceso.registros.pc, paquete);
    serializar_uint32_t(args->proceso.pid, paquete);
    enviar_paquete(paquete, args->config_leida.socket_memoria);
    eliminar_paquete(paquete);
    log_debug(args->logger, "Instruccion PC %d de PID <%d> pedida a Memoria", args->proceso.registros.pc, args->proceso.pid);
}