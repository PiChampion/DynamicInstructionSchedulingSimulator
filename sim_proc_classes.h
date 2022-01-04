#ifndef SIM_PROC_CLASSES_H
#define SIM_PROC_CLASSES_H

#include "sim_proc.h"
#include <vector>

class CPU {
public:
    CPU(FILE * FP, proc_params params);
    void Fetch(void);
    void Fetch1(void);
    void Decode(void);
    void Rename(void);
    void RegRead(void);
    void Dispatch(void);
    void Issue(void);
    void Execute(void);
    void Writeback(void);
    void Retire(void);

    bool Advance_Cycle(void);
    void print(char* argv[]);
    ~CPU();
private:
    
    int **fetch_bundle;
    int **decode_bundle;
    int **rename_bundle;
    int **regread_bundle;
    int **dispatch_bundle;
    int **writeback_bundle;
    int **issue_queue;
    int **reorder_buffer;
    int **execute_list;

    int rename_map_table[67][2] = {{0}};
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;

    int head = 0;	// Index for the ROB
    int tail = 0;	// Index for the ROB
    unsigned int instruction_number = 0;				// Number for each instruction from the trace

    std::vector<int*> vec = std::vector<int*>();		// Vector of vector information used for printing
    FILE * file_pointer;								
    unsigned int simulator_cycle;						// Cycle number of the simulation
    bool inserted_before;								// Variable for seeing if the ROB has had something inserted before
    bool trace_empty;
};

#endif

