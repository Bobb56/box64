#include "core.h"

#include "emu/extensions.h"

int main(int argc, const char **argv, char **env) {
    init_chronos();
    init_print_opcode("opcodes_log.txt");

    x64emu_t* emu = NULL;
    elfheader_t* elf_header = NULL;
    if (initialize(argc, argv, env, &emu, &elf_header, 1)) {
        return -1;
    }

    save_measure_results();

    int return_code = emulate(emu, elf_header);

    save_measure_results();
    finish_print_opcode();

    return return_code;
}
