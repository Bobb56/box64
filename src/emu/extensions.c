#include "extensions.h"


chrono_t blocks;
chrono_t translating;
chrono_t switching;
chrono_t interpreter;
chrono_t* current_chrono = &switching;
chrono_t* previous_chrono = &switching;

FILE* opcodes_log = NULL;
FILE* blocks_log = NULL;
FILE* time_details = NULL;

int block_counter = 0;
int block_length = 0;

bool in_block = false;



void init_time_details(char* filename) {
#ifdef TIME_DETAILS
    time_details = fopen(filename, "w");
    fprintf(time_details, "# chrono translating : %p\n", &translating);
    fprintf(time_details, "# chrono switching : %p\n", &switching);
    fprintf(time_details, "# chrono blocks : %p\n", &blocks);
    fprintf(time_details, "# chrono interpreter : %p\n", &interpreter);

#endif
}


void finish_time_details(void) {
#ifdef TIME_DETAILS
    fclose(time_details);
#endif
}



void new_block(int size) {
#ifdef BLOCKS_STATS
    block_length += size;
    block_counter++;
    fprintf(blocks_log, "    %d,\n", size);
#endif
}

void print_block_stats(void) {
#ifdef BLOCKS_STATS
    printf("Nombre de blocs : %d\nMoyenne des taille de blocs : %lf\n", block_counter, (double)block_length/(double)block_counter);
    fprintf(blocks_log, "    \"END\"\n]");
    fclose(blocks_log);
#endif
}


void init_print_blocks_stats(char* filename) {
#ifdef BLOCKS_STATS
    blocks_log = fopen(filename, "a");
    fprintf(blocks_log, "[\n");
    return;
#endif
}


void begin_block_trad(void) {
#ifdef PRINT_INSTS
    fprintf(opcodes_log, "['STARTBLOCK'],\n");
#endif
}

void end_block_trad(void) {}

void init_print_opcode(char* filename) {
#ifdef PRINT_INSTS
    opcodes_log = fopen(filename, "a");
    fprintf(opcodes_log, "[\n");
    return;
#endif
}

void finish_print_opcode(void) {
#ifdef PRINT_INSTS
    fprintf(opcodes_log, "    \"END\"\n]");
    fclose(opcodes_log);
#endif
}

/*
This function takes the address of the actual opcode and writes it in the file, assuming that the first byte is not a prefix,
except 0x0F
*/
void print_instruction(uintptr_t addr, int rep, uint8_t rex) {
#ifdef PRINT_INSTS
    fprintf(opcodes_log, "    ");
    uint8_t byte0 = *(uint8_t*)addr;
    if (byte0 == 0x0F) {
        uint8_t byte1 = *(uint8_t*)(addr+1);
        if (byte1 == 0x3A || byte1 == 0x38) {
            uint8_t byte2 = *(uint8_t*)(addr+2);
            fprintf(opcodes_log, "[0x%02x, 0x%02x, 0x%02x, %d, 0x%02x],\n", 0x0F, byte1, byte2, rep, rex);
        }
        else {
            fprintf(opcodes_log, "[0x%02x, 0x%02x, %d, 0x%02x],\n", 0x0F, byte1, rep, rex);
        }
    }
    else {
        fprintf(opcodes_log, "[0x%02x, %d, 0x%02x],\n", byte0, rep, rex);
    }
#endif
}


void init_chrono(chrono_t* chrono) {
#ifdef CHRONO
    chrono->initial = 0;
    chrono->acc = 0;
    chrono->state = Paused;
#endif
}


void start_chrono(chrono_t* chrono) {
#ifdef CHRONO
    if (chrono->state == Paused) {
        previous_chrono = current_chrono;
        current_chrono = chrono;

        chrono->initial = clock();
        chrono->state = Running;
    }
#endif
}



void pause_chrono(chrono_t* chrono) {
#ifdef CHRONO
    if (chrono->state == Running) {
        chrono->acc += clock() - chrono->initial;
        chrono->state = Paused;
#ifdef TIME_DETAILS
        fprintf(time_details, "End period %p of duration %ld\n", chrono, clock() - chrono->initial);
#endif
    }
#endif
}

void back_previous_chrono(void) {
    pause_chrono(current_chrono);
    start_chrono(previous_chrono);
}


void init_chronos(void) {
#ifdef CHRONO
    init_chrono(&blocks);
    init_chrono(&translating);
    init_chrono(&switching);
    init_chrono(&interpreter);
#endif
}

void start_translating(void) {
#ifdef CHRONO
    pause_chrono(&switching);
    pause_chrono(&blocks);
    pause_chrono(&interpreter);
    start_chrono(&translating);
#endif
}

void start_switching(void) {
#ifdef CHRONO
    pause_chrono(&translating);
    pause_chrono(&blocks);
    pause_chrono(&interpreter);
    start_chrono(&switching);
#endif
}

void start_interpreter(void) {
#ifdef CHRONO
    pause_chrono(&translating);
    pause_chrono(&blocks);
    pause_chrono(&switching);
    start_chrono(&interpreter);
#endif
}

void start_block(void) {
#ifdef CHRONO
    pause_chrono(&translating);
    pause_chrono(&switching);
    pause_chrono(&interpreter);
    start_chrono(&blocks);
#endif
}

void pause_all_chronos(void) {
#ifdef CHRONO
    pause_chrono(&translating);
    pause_chrono(&switching);
    pause_chrono(&blocks);
    pause_chrono(&interpreter);
#endif
}



void save_measure_results(void) {
#ifdef CHRONO
    pause_all_chronos();
    FILE* file = fopen("chrono.txt", "w");
    fprintf(file, "[%ld, ", translating.acc);
    fprintf(file, "%ld, ", switching.acc);
    fprintf(file, "%ld, ", blocks.acc);
    fprintf(file, "%ld]", interpreter.acc);
    fclose(file);
#endif
}







/*
IMPLEMENTATION DES CHRONOMETRES DANS BOX64

Initialisation de tous les chronomètres dans la fonction main, enregistrement des résultats dans la fonction main
Démarrage du chrono interpreter dans la fonction Run, passage au chrono switching à la toute fin de Run
Démarrage du chrono block au début de rv64_prolog
Démarrage du chrono switching à la fin de rv64_epilog
Démarrage du chrono translating dans InternalDBGetBlock, juste après le return dans le cas non create
*/