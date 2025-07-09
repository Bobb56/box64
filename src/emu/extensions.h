#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>


#define PRINT_INSTS // pour affiche les instructions traduites ou interprétées
//#define BLOCKS_STATS // pour afficher le nombre et les tailles des blocs traduits
#define CHRONO // pour activer le chronométrage des zones du programme
//#define TIME_DETAILS // pour détailler toutes les étapes et le temps qu'elles prennent

void new_block(int size);
void print_block_stats(void);
void init_print_blocks_stats(char* filename);
void begin_block_trad(void);
void end_block_trad(void);

void init_print_opcode(char* filename);
void finish_print_opcode(void);
void print_instruction(uintptr_t addr, int rep, uint8_t rex);

void init_time_details(char* filename);
void finish_time_details(void);

enum chrono_state {Running, Paused};


typedef struct chrono_s {
    clock_t initial;
    clock_t acc;
    enum chrono_state state;
} chrono_t;

void init_chronos(void);
void back_previous_chrono(void);
void start_translating(void);
void start_switching(void);
void start_block(void);
void start_interpreter(void);
void pause_all_chronos(void);
void save_measure_results(void);