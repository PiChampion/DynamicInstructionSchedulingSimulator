#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_proc.h"
#include "sim_proc_classes.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the definition of struct proc_params

    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];

    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
	
	// Create a CPU for executing instructions
    CPU my_CPU(FP, params);

    do {
        my_CPU.Retire();       
		// Retire up to WIDTH consecutive
        // “ready” instructions from the head of
        // the ROB.

        my_CPU.Writeback();    
		// Process the writeback bundle in WB:
        // For each instruction in WB, mark the
        // instruction as “ready” in its entry in
        // the ROB.

        my_CPU.Execute();      
		// From the execute_list, check for
        // instructions that are finishing
        // execution this cycle, and:
        // 1)   Remove the instruction from
        //      the execute_list.
        // 2)   Add the instruction to WB.
        // 3)   Wakeup dependent instructions (set
        //      their source operand ready flags) in
        //      the IQ, DI (the dispatch bundle), and
        //      RR (the register-read bundle).

        my_CPU.Issue();        
		// Issue up to WIDTH oldest instructions
        // from the IQ. (One approach to implement
        // oldest-first issuing, is to make multiple
        // passes through the IQ, each time finding
        // the next oldest ready instruction and
        // then issuing it. One way to annotate the
        // age of an instruction is to assign an
        // incrementing sequence number to each
        // instruction as it is fetched from the
        // trace file.)

        // To issue an instruction:
        // 1) Remove the instruction from the IQ.
        // 2) Add the instruction to the
        //      execute_list. Set a timer for the
        //      instruction in the execute_list that
        //      will allow you to model its execution
        //      latency.

        my_CPU.Dispatch();     
		// If DI contains a dispatch bundle:
        // If the number of free IQ entries is less
        // than the size of the dispatch bundle in
        // DI, then do nothing. If the number of
        // free IQ entries is greater than or equal
        // to the size of the dispatch bundle in DI,
        // then dispatch all instructions from DI to
        // the IQ.

        my_CPU.RegRead();      
		// If RR contains a register-read bundle:
        // If DI is not empty (cannot accept a
        // new dispatch bundle), then do nothing.
        // If DI is empty (can accept a new dispatch
        // bundle), then process (see below) the
        // register-read bundle and advance it from
        // RR to DI.
        //
        // Since values are not explicitly modeled,
        // the sole purpose of the Register Read
        // stage is to ascertain the readiness of
        // the renamed source operands. Apply your
        // learning from the class lectures/notes on
        // this topic.
        //
        // Also take care that producers in their
        // last cycle of execution wakeup dependent
        // operands not just in the IQ, but also in
        // two other stages including RegRead()
        // (this is required to avoid deadlock). See
        // Execute() description above.

        my_CPU.Rename();       
		// If RN contains a rename bundle:
        // If either RR is not empty (cannot accept
        // a new register-read bundle) or the ROB
        // does not have enough free entries to
        // accept the entire rename bundle, then do
        // nothing.
        // If RR is empty (can accept a new
        // register-read bundle) and the ROB has
        // enough free entries to accept the entire
        // rename bundle, then process (see below)
        // the rename bundle and advance it from
        // RN to RR.
        //
        // Apply your learning from the class
        // lectures/notes on the steps for renaming:
        // (1) allocate an entry in the ROB for the
        // instruction, (2) rename its source
        // registers, and (3) rename its destination
        // register (if it has one). Note that the
        // rename bundle must be renamed in program
        // order (fortunately the instructions in
        // the rename bundle are in program order).

        my_CPU.Decode();       
		// If DE contains a decode bundle:
        // If RN is not empty (cannot accept a new
        // rename bundle), then do nothing.
        // If RN is empty (can accept a new rename
        // bundle), then advance the decode bundle
        // from DE to RN.
        my_CPU.Fetch1();
        my_CPU.Fetch();        
		// Do nothing if either (1) there are no
        // more instructions in the trace file or
        // (2) DE is not empty (cannot accept a new
        // decode bundle).
        //
        // If there are more instructions in the
        // trace file and if DE is empty (can accept
        // a new decode bundle), then fetch up to
        // WIDTH instructions from the trace file
        // into DE. Fewer than WIDTH instructions
        // will be fetched only if the trace file
        // has fewer than WIDTH instructions left.        
    } while (my_CPU.Advance_Cycle());
    // Advance_Cycle performs several functions.
    // First, it advances the simulator cycle.
    // Second, when it becomes known that the
    // pipeline is empty AND the trace is depleted,
    // the function returns “false” to terminate
    // the loop.

    my_CPU.print(argv);

    return 0;
}
