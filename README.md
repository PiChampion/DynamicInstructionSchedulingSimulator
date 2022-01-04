# Dynamic Instruction Scheduling Simulator

The project was completed as part of ECE 563 (Microprocessor Architecture) at NC State. In this project, I implemented an out-of-order superscalar processor simulator in C++ and used it to evaluate different configurations of out-of-order superscalar processors.

Command-line parameters which govern the specifics of the processor being simulated include reorder buffer size, instruction queue size, and width of the pipeline.

### Microarchitecture Specifications:
* Parameters:
  * Number of architectural registers: The number of architectural registers specified in the ISA is 67 (r0-r66). The number of architectural registers determines the number of entries in the Rename Map Table (RMT) and Architectural Register File (ARF).
  * WIDTH: This is the superscalar width of all pipeline stages, in terms of the maximum number of instructions in each pipeline stage.
  * IQ_SIZE: This is the number of entries in the Issue Queue (IQ).
  * ROB_SIZE: This is the number of entries in the Reorder Buffer (ROB).
* Function units:
  * There are WIDTH universal pipelined function units (FUs). Each FU can execute any type of instruction (hence the term “universal”). The operation type of an instruction indicates its execution latency.
* Pipeline registers:
  * The pipeline stages (Fetch, Decode, Rename, Register Read, Dispatch, Issue, Execute, Writeback, Retire) are separated by pipeline registers.

### The simulator outputs the following:
* The simulator command.
* The processor configuration.
* Simulation results:
  * dynamic instruction count
  * cycles
  * instructions per cycle
