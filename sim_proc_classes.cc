#include <iostream>
#include <iomanip>
#include "sim_proc_classes.h"

CPU::CPU(FILE * FP, proc_params params) {

    unsigned int i, j;

    file_pointer = FP;
    
    fetch_bundle = new int*[params.width];
    for(i = 0; i < params.width; ++i) {
        fetch_bundle[i] = new int[7];
        for(j = 0; j < 7; j++) {
            fetch_bundle[i][j] = 0;
        }
    }

    decode_bundle = new int*[params.width];
    for(i = 0; i < params.width; ++i) {
        decode_bundle[i] = new int[7];
        for(j = 0; j < 7; j++) {
            decode_bundle[i][j] = 0;
        }
    }

    rename_bundle = new int*[params.width];
    for(i = 0; i < params.width; ++i) {
        rename_bundle[i] = new int[7];
        for(j = 0; j < 7; j++) {
            rename_bundle[i][j] = 0;
        }
    }

    regread_bundle = new int*[params.width];
    for(i = 0; i < params.width; ++i) {
        regread_bundle[i] = new int[9];
        for(j = 0; j < 9; j++) {
            regread_bundle[i][j] = 0;
        }
    }

    dispatch_bundle = new int*[params.width];
    for(i = 0; i < params.width; ++i) {
        dispatch_bundle[i] = new int[11];
        for(j = 0; j < 11; j++) {
            dispatch_bundle[i][j] = 0;
        }
    }

    writeback_bundle = new int*[params.width * 5];
    for(i = 0; i < params.width * 5; ++i) {
        writeback_bundle[i] = new int[3];
        for(j = 0; j < 3; j++) {
            writeback_bundle[i][j] = 0;
        }
    }

    issue_queue = new int*[params.iq_size];
    for(i = 0; i < params.iq_size; ++i) {
        issue_queue[i] = new int[8];
        for(j = 0; j < 8; j++) {
            issue_queue[i][j] = 0;
        }
    }

    reorder_buffer = new int*[params.rob_size];
    for(i = 0; i < params.rob_size; ++i) {
        reorder_buffer[i] = new int[6];
        for(j = 0; j < 6; j++) {
            reorder_buffer[i][j] = 0;
        }
    }

    execute_list = new int*[params.width * 5];
    for(i = 0; i < params.width * 5; ++i) {
        execute_list[i] = new int[4];
        for(j = 0; j < 4; j++) {
            execute_list[i][j] = 0;
        }
    }
    

    simulator_cycle = 0;
    width = params.width;
    rob_size = params.rob_size;
    iq_size = params.iq_size;
    trace_empty = false;
	inserted_before = false;
}

void CPU::Fetch(void) {
    int op_type, dest, src1, src2;  // Variables are read from trace file
    unsigned long int pc; 			// Variable holds the pc read from input file
    unsigned int i;
    
	// Check to see if the fetch bundle can accept new instructions
    for (i = 0; i < width; i++) {
        if (fetch_bundle[i][FETCH_VALID]) return;
    }

	// Take in instructions until the end of file is encountered
    for (i = 0; i < width; i++) {
        if (fscanf(file_pointer, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) == EOF) {
            if (!i) trace_empty = true;		// If no instruction was read, indicate that the trace is empty
            return;
        } 
		else {
			// Add a new vector to hold information for printing for this instruction
            auto *new_array = new int[15];
            vec.push_back(new_array);
            vec[instruction_number][0] = instruction_number;
            vec[instruction_number][1] = op_type;
            vec[instruction_number][2] = src1;
            vec[instruction_number][3] = src2;
            vec[instruction_number][4] = dest;

			// Add the instruction to the fetch bundle
            fetch_bundle[i][FETCH_VALID] = VALID;
            fetch_bundle[i][FETCH_PC] = pc;
            fetch_bundle[i][FETCH_OP] = op_type;
            fetch_bundle[i][FETCH_RD] = dest;
            fetch_bundle[i][FETCH_RS1] = src1;
            fetch_bundle[i][FETCH_RS2] = src2;
            fetch_bundle[i][FETCH_SEQ] = instruction_number;
            instruction_number++;
        }
    }
}
void CPU::Fetch1(void) {
    bool has_bundle = false;
    unsigned int i;

	// Check to see if the fetch bundle has instructions
    for(i = 0; i < width; i++) {
        if(fetch_bundle[i][FETCH_VALID]) has_bundle = true;
    }
    if(has_bundle) {
		// Check to see if the decode bundle can accept new instructions
        for (i = 0; i < width; i++) {
            if (decode_bundle[i][DECODE_VALID]) return;
        }

		// Place the instructions in the fetch bundle in the decode bundle
        for (i = 0; i < width; i++) {
            if (fetch_bundle[i][FETCH_VALID] == VALID) {
                decode_bundle[i][DECODE_VALID] = fetch_bundle[i][FETCH_VALID];
                decode_bundle[i][DECODE_PC] = fetch_bundle[i][FETCH_PC];
                decode_bundle[i][DECODE_OP] = fetch_bundle[i][FETCH_OP];
                decode_bundle[i][DECODE_RD] = fetch_bundle[i][FETCH_RD];
                decode_bundle[i][DECODE_RS1] = fetch_bundle[i][FETCH_RS1];
                decode_bundle[i][DECODE_RS2] = fetch_bundle[i][FETCH_RS2];
                decode_bundle[i][DECODE_SEQ] = fetch_bundle[i][FETCH_SEQ];
				
				vec[fetch_bundle[i][FETCH_SEQ]][5] = simulator_cycle - 1;
                vec[fetch_bundle[i][FETCH_SEQ]][6] = simulator_cycle;
            }
        }
		// Invalidate the instructions which were in the fetch bundle
        for(i = 0; i < width; i++) {
            fetch_bundle[i][FETCH_VALID] = INVALID;
        }
    }
}

void CPU::Decode(void) {
    unsigned int i;
    bool has_bundle = false;

	// Check to see if the decode bundle has instructions
    for(i = 0; i < width; i++) {
        if(decode_bundle[i][DECODE_VALID]) has_bundle = true;
    }

    if(has_bundle) {
		// Check to see if the rename bundle can accept new instructions
        for(i = 0; i < width; i++) {
            if(rename_bundle[i][RENAME_VALID]) return;
        }
		
		// Place the instructions in the decode bundle in the rename bundle
        for(i = 0; i < width; i++) {
          if(decode_bundle[i][DECODE_VALID]) {
            rename_bundle[i][RENAME_VALID] = decode_bundle[i][DECODE_VALID];
            rename_bundle[i][RENAME_PC] = decode_bundle[i][DECODE_PC];
            rename_bundle[i][RENAME_OP] = decode_bundle[i][DECODE_OP];
            rename_bundle[i][RENAME_RD] = decode_bundle[i][DECODE_RD];
            rename_bundle[i][RENAME_RS1] = decode_bundle[i][DECODE_RS1];
            rename_bundle[i][RENAME_RS2] = decode_bundle[i][DECODE_RS2];
            rename_bundle[i][RENAME_SEQ] = decode_bundle[i][DECODE_SEQ];

            vec[decode_bundle[i][DECODE_SEQ]][7] = simulator_cycle;
            }
        }
		// Invalidate the instructions which were in the decode bundle
        for(i = 0; i < width; i++) {
            decode_bundle[i][DECODE_VALID] = INVALID;
        }
    }
}

void CPU::Rename(void) {
    unsigned int i;
    int number_of_instructions = 0;
    bool has_bundle = false;

	// Check to see if the rename bundle has instructions and counts the instructions
    for(i = 0; i < width; i++) {
        if(rename_bundle[i][RENAME_VALID]) {
            has_bundle = true;
            number_of_instructions++;
        }
    }
	
    if(has_bundle) {
		// Check to see if the regread bundle can accept new instructions
        for(i = 0; i < width; i++) {
            if(regread_bundle[i][REGREAD_VALID]) return;
        }
		
		// Check to see if the ROB can hold the number of instructions in the rename bundle
        if(tail - head > 0) {
            if((int(rob_size) - (tail - head)) < number_of_instructions) return;
        }
        else if(tail - head == 0) {
            if(inserted_before) return;
        }
        else {
            if((head - tail) < number_of_instructions) return;
        }
		
		// Place the instructions in the regread bundle in the rename bundle
        for(i = 0; i < width; i++) {
            if(rename_bundle[i][RENAME_VALID]) {			// Check for validity before moving the instruction
                // Check to make sure there is a source register 1 before renaming
				if(rename_bundle[i][RENAME_RS1] >= 0) {
                    // Look for the register in the rename map and update it accordingly
					if (rename_map_table[rename_bundle[i][RENAME_RS1]][RENAME_MAP_VALID] == VALID) {
                        regread_bundle[i][REGREAD_IS_ROB1] = 1;
                        regread_bundle[i][REGREAD_S1] = rename_map_table[rename_bundle[i][RENAME_RS1]][RENAME_MAP_ROB];
                    }
                    else {
                        regread_bundle[i][REGREAD_IS_ROB1] = 0;
                        regread_bundle[i][REGREAD_S1] = rename_bundle[i][RENAME_RS1];
                    }
                }
				
				// If no source register, it is not a ROB entry
                else {
                    regread_bundle[i][REGREAD_IS_ROB1] = 0;
                    regread_bundle[i][REGREAD_S1] = rename_bundle[i][RENAME_RS1];
                }
				// Check to make sure there is a source register 2 before renaming
                if(rename_bundle[i][RENAME_RS2] >= 0) {
                    if (rename_map_table[rename_bundle[i][RENAME_RS2]][RENAME_MAP_VALID] == VALID) {
                        regread_bundle[i][REGREAD_IS_ROB2] = 1;
                        regread_bundle[i][REGREAD_S2] = rename_map_table[rename_bundle[i][RENAME_RS2]][RENAME_MAP_ROB];
                    }
					// Look for the register in the rename map and update it accordingly
                    else {
                        regread_bundle[i][REGREAD_IS_ROB2] = 0;
                        regread_bundle[i][REGREAD_S2] = rename_bundle[i][RENAME_RS2];
                    }
                }
				
				// If no source register, it is not a ROB entry
                else {
                    regread_bundle[i][REGREAD_IS_ROB2] = 0;
                    regread_bundle[i][REGREAD_S2] = rename_bundle[i][RENAME_RS2];
                }
				
				// Update the destination register in the rename map
                if(rename_bundle[i][RENAME_RD] >= 0) {
                    rename_map_table[rename_bundle[i][RENAME_RD]][RENAME_MAP_VALID] = VALID;
                    rename_map_table[rename_bundle[i][RENAME_RD]][RENAME_MAP_ROB] = tail;
                }
				
				// Update the destination to be the ROB location
				regread_bundle[i][REGREAD_DEST] = tail;
                
				// Variable for determining if when the tail and head are equal it is full or the beginning of the simulation
				inserted_before = true;
				
				// Update ROB with information
                reorder_buffer[tail][REORDER_DEST] = rename_bundle[i][RENAME_RD];
                reorder_buffer[tail][REORDER_RDY] = 0;
                reorder_buffer[tail][REORDER_EXC] = 0;
                reorder_buffer[tail][REORDER_MIS] = 0;
                reorder_buffer[tail][REORDER_PC] = rename_bundle[i][RENAME_PC];
                reorder_buffer[tail][REORDER_SEQ] = rename_bundle[i][RENAME_SEQ];
				
				// Update tail
                if(unsigned(tail) >= (rob_size - 1)) tail = 0;
                else tail++;

				// Place the instructions in the rename bundle in the regread bundle
                regread_bundle[i][REGREAD_VALID] = rename_bundle[i][RENAME_VALID];
                regread_bundle[i][REGREAD_PC] = rename_bundle[i][RENAME_PC];
                regread_bundle[i][REGREAD_OP] = rename_bundle[i][RENAME_OP];
                regread_bundle[i][REGREAD_SEQ] = rename_bundle[i][RENAME_SEQ];
            
				vec[rename_bundle[i][RENAME_SEQ]][8] = simulator_cycle;
            }
        }
		
		// Invalidate the instructions which were in the rename bundle
        for(i = 0; i < width; i++) {
            rename_bundle[i][RENAME_VALID] = INVALID;
        }
    }
}

void CPU::RegRead(void) {
    unsigned int i;
    bool has_bundle = false;

	// Check to see if the regread bundle has instructions
    for(i = 0; i < width; i++) {
        if(regread_bundle[i][REGREAD_VALID]) has_bundle = true;
    }

    if(has_bundle) {
		// Check to see if the dispatch bundle can accept new instructions
        for(i = 0; i < width; i++) {
            if(dispatch_bundle[i][DISPATCH_VALID]) return;
        }
		
		// Place the instructions in the regread bundle in the dispatch bundle
        for(i = 0; i < width; i++) {
          if(regread_bundle[i][REGREAD_VALID]) {
            dispatch_bundle[i][DISPATCH_VALID] = regread_bundle[i][REGREAD_VALID];
            dispatch_bundle[i][DISPATCH_PC] = regread_bundle[i][REGREAD_PC];
            dispatch_bundle[i][DISPATCH_OP] = regread_bundle[i][REGREAD_OP];
            dispatch_bundle[i][DISPATCH_DEST] = regread_bundle[i][REGREAD_DEST];
			
			// If source register 1 is a ROB entry, check to see if the ROB entry is ready
            if(regread_bundle[i][REGREAD_S1] >= 0) {
                if (regread_bundle[i][REGREAD_IS_ROB1]) {
					// If ready, change the source to be ready and no longer a ROB entry
                    if (reorder_buffer[regread_bundle[i][REGREAD_S1]][REORDER_RDY]) {
                        dispatch_bundle[i][DISPATCH_S1_RDY] = 1;
                        dispatch_bundle[i][DISPATCH_IS_ROB1] = 0;
                    }
					// If not ready, change the source to be not ready and still a ROB entry
                    else {
                        dispatch_bundle[i][DISPATCH_S1_RDY] = 0;
                        dispatch_bundle[i][DISPATCH_IS_ROB1] = 1;
                    }
                }
				// If not a ROB entry, make sure it's ready any not a ROB entry
                else {
                    dispatch_bundle[i][DISPATCH_S1_RDY] = 1;
                    dispatch_bundle[i][DISPATCH_IS_ROB1] = 0;
                }
            }
			// If no source register, it is ready and not a ROB entry
            else {
                dispatch_bundle[i][DISPATCH_S1_RDY] = 1;
                dispatch_bundle[i][DISPATCH_IS_ROB1] = 0;
            }
            dispatch_bundle[i][DISPATCH_S1] = regread_bundle[i][REGREAD_S1];
			
			// If source register 2 is a ROB entry, check to see if the ROB entry is ready
            if(regread_bundle[i][REGREAD_S2] >= 0) {
                if (regread_bundle[i][REGREAD_IS_ROB2]) {
					// If ready, change the source to be ready and no longer a ROB entry
                    if (reorder_buffer[regread_bundle[i][REGREAD_S2]][REORDER_RDY]) {
                        dispatch_bundle[i][DISPATCH_S2_RDY] = 1;
                        dispatch_bundle[i][DISPATCH_IS_ROB2] = 0;
                    }
					// If not ready, change the source to be not ready and still a ROB entry
                    else {
                        dispatch_bundle[i][DISPATCH_S2_RDY] = 0;
                        dispatch_bundle[i][DISPATCH_IS_ROB2] = 1;
                    }
                }
				// If not a ROB entry, make sure it's ready any not a ROB entry
                else {
                    dispatch_bundle[i][DISPATCH_S2_RDY] = 1;
                    dispatch_bundle[i][DISPATCH_IS_ROB2] = 0;
                }
            }
			// If no source register, it is ready and not a ROB entry
            else {
                dispatch_bundle[i][DISPATCH_S2_RDY] = 1;
                dispatch_bundle[i][DISPATCH_IS_ROB2] = 0;
            }
            dispatch_bundle[i][DISPATCH_S2] = regread_bundle[i][REGREAD_S2];
			
            dispatch_bundle[i][DISPATCH_SEQ] = regread_bundle[i][REGREAD_SEQ];

            vec[regread_bundle[i][REGREAD_SEQ]][9] = simulator_cycle;
            }
        }
		// Invalidate the instructions which were in the regread bundle
        for(i = 0; i < width; i++) {
            regread_bundle[i][REGREAD_VALID] = INVALID;
        }
    }
}

void CPU::Dispatch(void) {
    unsigned int i, j;
    bool has_bundle = false;
    unsigned int number_of_queued = 0;
    unsigned int number_of_instructions = 0;

	// Check to see if the dispatch bundle has instructions and counts the instructions
    for(i = 0; i < width; i++) {
        if(dispatch_bundle[i][DISPATCH_VALID]) {
            has_bundle = true;
            number_of_instructions++;
        }
    }
	
	// Count the number of instructions in the issue queue
    for(i = 0; i < iq_size; i++) {
        if(issue_queue[i][ISSUE_VALID])
            number_of_queued++;
    }

    if(has_bundle) {
        if(number_of_instructions > (iq_size - number_of_queued)) return;
        for(i = 0; i < width; i++) {
            if(dispatch_bundle[i][DISPATCH_VALID]) {
                for(j = 0; j < iq_size; j++) {
					// Look for a free spot in the issue queue to put the instruction
                    if(!issue_queue[j][ISSUE_VALID]) {
                        issue_queue[j][ISSUE_VALID] = VALID;
                        issue_queue[j][ISSUE_DEST] = dispatch_bundle[i][DISPATCH_DEST];
                        issue_queue[j][ISSUE_RS1_RDY] = dispatch_bundle[i][DISPATCH_S1_RDY];
						
						// Check to see if there is a ROB entry and if it's ready in the ROB
                        if(dispatch_bundle[i][DISPATCH_S1] >= 0) {
                            if (dispatch_bundle[i][DISPATCH_IS_ROB1]) {
                                if (reorder_buffer[dispatch_bundle[i][DISPATCH_S1]][REORDER_RDY]) {
                                    issue_queue[j][ISSUE_RS1_RDY] = 1;
                                }
                                else {
                                    issue_queue[j][ISSUE_RS1_RDY] = 0;
                                }
                            }
                            else {
                                issue_queue[j][ISSUE_RS1_RDY] = 1;
                            }
                        }
                        else {
                            issue_queue[j][ISSUE_RS1_RDY] = 1;
                        }
                        issue_queue[j][ISSUE_RS1] = dispatch_bundle[i][DISPATCH_S1];
						
						// Check to see if there is a ROB entry and if it's ready in the ROB
                        if(dispatch_bundle[i][DISPATCH_S2] >= 0) {
                            if (dispatch_bundle[i][DISPATCH_IS_ROB2]) {
                                if (reorder_buffer[dispatch_bundle[i][DISPATCH_S2]][REORDER_RDY]) {
                                    issue_queue[j][ISSUE_RS2_RDY] = 1;
                                }
                                else {
                                    issue_queue[j][ISSUE_RS2_RDY] = 0;
                                }
                            }
                            else {
                                issue_queue[j][ISSUE_RS2_RDY] = 1;
                            }
                        }
                        else {
                            issue_queue[j][ISSUE_RS2_RDY] = 1;
                        }
                        issue_queue[j][ISSUE_RS2] = dispatch_bundle[i][DISPATCH_S2];
						
                        issue_queue[j][ISSUE_SEQ] = dispatch_bundle[i][DISPATCH_SEQ];
                        issue_queue[j][ISSUE_OP] = dispatch_bundle[i][DISPATCH_OP];
                        break;
                    }
                }
                vec[dispatch_bundle[i][DISPATCH_SEQ]][10] = simulator_cycle;
            }
        }
		
		// Invalidate the instructions which were in the dispatch bundle
        for(i = 0; i < width; i++) {
            dispatch_bundle[i][DISPATCH_VALID] = INVALID;
        }
    }
}

void CPU::Issue(void) {
    unsigned int i, j;
	int oldest_instruction;
	int execute_spot = 0;
    int oldest_seq = INT32_MAX;
    unsigned int found_instruction = 0;

    for(i = 0; i < width; i++) {
        for(j = 0; j < iq_size; j++) {
			// Look for the oldest instruction in the issue which is ready
            if((issue_queue[j][ISSUE_SEQ] < oldest_seq) && issue_queue[j][ISSUE_VALID] && issue_queue[j][ISSUE_RS1_RDY] && issue_queue[j][ISSUE_RS2_RDY]) {
                oldest_instruction = j;
                found_instruction = 1;
                oldest_seq = issue_queue[j][ISSUE_SEQ];
            }
        }

		// If there was no instruction found, return
        if(!found_instruction) return;
        else found_instruction = 0;

        vec[issue_queue[oldest_instruction][ISSUE_SEQ]][11] = simulator_cycle;

        for(j = 0; j < width * 5; j++) {
			// Look for a free spot in the execute list to place the instruction
            if(execute_list[j][EXECUTE_VALID] == INVALID) {
                execute_spot = j;
                break;
            }
        }
        execute_list[execute_spot][EXECUTE_VALID] = VALID;
        execute_list[execute_spot][EXECUTE_SEQ] = issue_queue[oldest_instruction][ISSUE_SEQ];
        execute_list[execute_spot][EXECUTE_DEST] = issue_queue[oldest_instruction][ISSUE_DEST];
		
		// Determine the execution time based on the OP
        switch(issue_queue[oldest_instruction][ISSUE_OP]) {
            case 0:
                execute_list[execute_spot][EXECUTE_TIME_LEFT] = 1;
                break;
            case 1:
                execute_list[execute_spot][EXECUTE_TIME_LEFT] = 2;
                break;
            case 2:
                execute_list[execute_spot][EXECUTE_TIME_LEFT] = 5;
                break;
            default:
                std::cout << "ERROR INVALID OP" << std::endl;
                break;
        }
        oldest_seq = INT32_MAX;				// Reset the oldest instruction sequence so it can be searched for again
        issue_queue[oldest_instruction][ISSUE_VALID] = INVALID;		// Invalidate the issue queue instruction which was executed
    }
}

void CPU::Execute(void) {
    unsigned int i, j, k;

	// Decrease the time left for all the instructions being executed
    for(i = 0; i < width * 5; i++) {
        if(execute_list[i][EXECUTE_VALID])
            execute_list[i][EXECUTE_TIME_LEFT]--;
    }

	// Look for insructions which had finished executing
    for(i = 0; i < width * 5; i++) {
        if(execute_list[i][EXECUTE_VALID] && (execute_list[i][EXECUTE_TIME_LEFT] == 0)) {
            for(k = 0; k < width * 5; k++) {
				// Look for a spot in the writeback bundle to place the instruction
                if (writeback_bundle[k][WRITEBACK_VALID] == INVALID) {
                    writeback_bundle[k][WRITEBACK_VALID] = VALID;
                    writeback_bundle[k][WRITEBACK_ROB] = execute_list[i][EXECUTE_DEST];
                    writeback_bundle[k][WRITEBACK_SEQ] = execute_list[i][EXECUTE_SEQ];
                    execute_list[i][EXECUTE_VALID] = INVALID;		// Invalidate the instruction in the execute list
					
					// Update instructions in the issue queue which have a source that 
					// matched the destination of the instruction that completed execution
                    for (j = 0; j < iq_size; j++) {
                        if ((issue_queue[j][ISSUE_RS1] == execute_list[i][EXECUTE_DEST]) &&
                            (issue_queue[j][ISSUE_VALID] == 1)) {
                            issue_queue[j][ISSUE_RS1_RDY] = 1;
                        }
                        if ((issue_queue[j][ISSUE_RS2] == execute_list[i][EXECUTE_DEST]) &&
                            (issue_queue[j][ISSUE_VALID] == 1)) {
                            issue_queue[j][ISSUE_RS2_RDY] = 1;
                        }
                    }

					// Update instructions in the dispatch bundle which have a source that 
					// matched the destination of the instruction that completed execution
                    for (j = 0; j < width; j++) {
                        if ((dispatch_bundle[j][DISPATCH_S1] == execute_list[i][EXECUTE_DEST]) &&
                            (dispatch_bundle[j][DISPATCH_VALID] == VALID) && (dispatch_bundle[j][DISPATCH_IS_ROB1] == 1)) {
                            dispatch_bundle[j][DISPATCH_S1_RDY] = 1;
                            dispatch_bundle[j][DISPATCH_IS_ROB1] = 0;
                        }
                        if ((dispatch_bundle[j][DISPATCH_S2] == execute_list[i][EXECUTE_DEST]) &&
                            (dispatch_bundle[j][DISPATCH_VALID] == VALID) && (dispatch_bundle[j][DISPATCH_IS_ROB2] == 1)) {
                            dispatch_bundle[j][DISPATCH_S2_RDY] = 1;
                            dispatch_bundle[j][DISPATCH_IS_ROB2] = 0;
                        }
                    }

					// Update instructions in the regread bundle which have a source that 
					// matched the destination of the instruction that completed execution
                    for (j = 0; j < width; j++) {
                        if ((regread_bundle[j][REGREAD_S1] == execute_list[i][EXECUTE_DEST]) &&
                            (regread_bundle[j][REGREAD_VALID] == VALID) && (regread_bundle[j][REGREAD_IS_ROB1] == 1)) {
                            regread_bundle[j][REGREAD_IS_ROB1] = 0;
                        }
                        if ((regread_bundle[j][REGREAD_S2] == execute_list[i][EXECUTE_DEST]) &&
                            (regread_bundle[j][REGREAD_VALID] == VALID) && (regread_bundle[j][REGREAD_IS_ROB2] == 1)) {
                            regread_bundle[j][REGREAD_IS_ROB2] = 0;
                        }
                    }

                    vec[execute_list[i][EXECUTE_SEQ]][12] = simulator_cycle;

                    break;
                }
            }
        }
    }
}

void CPU::Writeback(void) {
    unsigned int i;
	
	// Make the ROB entry ready for each instruction in the writeback bundle
    for(i = 0; i < width * 5; i++) {
        if(writeback_bundle[i][WRITEBACK_VALID] == VALID) {
            reorder_buffer[writeback_bundle[i][WRITEBACK_ROB]][REORDER_RDY] = 1;
            writeback_bundle[i][WRITEBACK_VALID] = INVALID;

            vec[writeback_bundle[i][WRITEBACK_SEQ]][13] = simulator_cycle;
        }
    }
}

void CPU::Retire(void) {
    unsigned int i, j;

    for(i = 0; i < width; i++) {
		// Look to see if the head of the ROB is ready
        if(reorder_buffer[head][REORDER_RDY]) {
            reorder_buffer[head][REORDER_RDY] = 0;
            vec[reorder_buffer[head][REORDER_SEQ]][14] = simulator_cycle;
			
			// If there is a destination register for the ROB entry, look to see if 
			// the destination register in the rename table has been renamed to this ROB.
			// If so, reset the entry in the rename table
            if(reorder_buffer[head][REORDER_DEST] >= 1) {
                if (rename_map_table[reorder_buffer[head][REORDER_DEST]][RENAME_MAP_ROB] == head) {
                    rename_map_table[reorder_buffer[head][REORDER_DEST]][RENAME_MAP_VALID] = 0;
                }
            }

			// Look for dependent ROB sources in the dispatch bundle and make then ready
            for (j = 0; j < width; j++) {
                if ((dispatch_bundle[j][DISPATCH_S1] == head) &&
                    (dispatch_bundle[j][DISPATCH_VALID] == VALID) && (dispatch_bundle[j][DISPATCH_IS_ROB1] == 1)) {
                    dispatch_bundle[j][DISPATCH_S1_RDY] = 1;
                    dispatch_bundle[j][DISPATCH_IS_ROB1] = 0;
                }
                if ((dispatch_bundle[j][DISPATCH_S2] == head) &&
                    (dispatch_bundle[j][DISPATCH_VALID] == VALID) && (dispatch_bundle[j][DISPATCH_IS_ROB2] == 1)) {
                    dispatch_bundle[j][DISPATCH_S2_RDY] = 1;
                    dispatch_bundle[j][DISPATCH_IS_ROB2] = 0;
                }
            }

			// Look for dependent ROB sources in the regread bundle and make then ready
            for (j = 0; j < width; j++) {
                if ((regread_bundle[j][REGREAD_S1] == head) &&
                    (regread_bundle[j][REGREAD_VALID] == VALID) && (regread_bundle[j][REGREAD_IS_ROB1] == 1)) {
                    regread_bundle[j][REGREAD_IS_ROB1] = 0;
                }
                if ((regread_bundle[j][REGREAD_S2] == head) &&
                    (regread_bundle[j][REGREAD_VALID] == VALID) && (regread_bundle[j][REGREAD_IS_ROB2] == 1)) {
                    regread_bundle[j][REGREAD_IS_ROB2] = 0;
                }
            }

            if(unsigned(head) >= (rob_size - 1)) head = 0;
            else head++;
        }
        else break;
    }
}

bool CPU::Advance_Cycle(void) {
    unsigned int i;
    bool has_bundle = false;
	
	// Search all the bundles, ROB, issue queue, and execute list for instructions
    for(i = 0; i < width; i++) {
        if(decode_bundle[i][DECODE_VALID]) has_bundle = true;
    }
    for(i = 0; i < width; i++) {
        if(rename_bundle[i][RENAME_VALID]) has_bundle = true;
    }
    for(i = 0; i < width; i++) {
        if(regread_bundle[i][REGREAD_VALID]) has_bundle = true;
    }
    for(i = 0; i < width; i++) {
        if(dispatch_bundle[i][DISPATCH_VALID]) has_bundle = true;
    }
    for(i = 0; i < iq_size; i++) {
        if(issue_queue[i][ISSUE_VALID]) has_bundle = true;
    }
    for(i = 0; i < width*5; i++) {
        if(execute_list[i][EXECUTE_VALID]) has_bundle = true;
    }
    for(i = 0; i < width*5; i++) {
        if(writeback_bundle[i][WRITEBACK_VALID]) has_bundle = true;
    }
    if(head != tail) has_bundle = true;

    simulator_cycle++;
	
	// If there are no more instructions in the trace and there are no instructions in the pipeline, don't advance
    if(trace_empty && !has_bundle) return false;
    else return true;
}

void CPU::print(char* argv[]) {
    unsigned int i;

    for(i = 0; i < vec.size(); i++) {
        std::cout << vec[i][0] << " fu{" << vec[i][1] << "} src{" << vec[i][2] << "," << vec[i][3] << "} dst{" <<
        vec[i][4] << "} FE{" << vec[i][5] << "," << vec[i][6] - vec[i][5] << "} DE{" << vec[i][6] << "," <<
        vec[i][7] - vec[i][6] << "} RN{" <<  vec[i][7] << "," << vec[i][8] - vec[i][7] << "} RR{" <<
        vec[i][8] << "," << vec[i][9] - vec[i][8] << "} DI{" <<  vec[i][9] << "," << vec[i][10] - vec[i][9] << "} IS{"
        << vec[i][10] << "," << vec[i][11] - vec[i][10] << "} EX{" << vec[i][11] << "," << vec[i][12] - vec[i][11] << "} WB{"
        << vec[i][12] << "," << vec[i][13] - vec[i][12] << "} RT{" << vec[i][13] << "," << vec[i][14] - vec[i][13] << "}"
        << std::endl;
    }
    std::cout << "# === Simulator Command =========" << std::endl << "# " << argv[0] << " " << argv[1] << " " << argv[2]
              << " " << argv[3] << " " << argv[4] << std::endl << "# === Processor Configuration ===" << std::endl << "# ROB_SIZE = " << argv[1]
              << std::endl << "# IQ_SIZE  = " << argv[2] << std::endl << "# WIDTH    = " << argv[3] << std::endl << "# === Simulation Results ========"
              << std::endl << "# Dynamic Instruction Count    = " << instruction_number << std::endl << "# Cycles                       = " <<
              simulator_cycle - 1 << std::endl << "# Instructions Per Cycle (IPC) = " << std::fixed << std::setprecision(2) 
              << float(instruction_number)/float(simulator_cycle - 1) << std::endl;
}

CPU::~CPU() {
	unsigned int i;
	
    for(i = 0; i < width; ++i) {
        delete[] fetch_bundle[i];
    }

    for(i = 0; i < width; ++i) {
		delete[] decode_bundle[i];
    }

    for(i = 0; i < width; ++i) {
		delete[] rename_bundle[i];
    }

    for(i = 0; i < width; ++i) {
		delete[] regread_bundle[i];
    }

    for(i = 0; i < width; ++i) {
		delete[] dispatch_bundle[i];
    }

    for(i = 0; i < width * 5; ++i) {
		delete[] writeback_bundle[i];
    }

    for(i = 0; i < iq_size; ++i) {
		delete[] issue_queue[i];
    }

    for(i = 0; i < rob_size; ++i) {
		delete[] reorder_buffer[i];
    }

    for(i = 0; i < width * 5; ++i) {
		delete[] execute_list[i];
    }
}