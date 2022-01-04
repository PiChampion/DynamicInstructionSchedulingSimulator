#ifndef SIM_PROC_H
#define SIM_PROC_H

#define VALID 1
#define INVALID 0

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

enum{
    FETCH_VALID = 0,
    FETCH_PC,
    FETCH_OP,
    FETCH_RD,
    FETCH_RS1,
    FETCH_RS2,
    FETCH_SEQ
};

enum{
    DECODE_VALID = 0,
    DECODE_PC,
    DECODE_OP,
    DECODE_RD,
    DECODE_RS1,
    DECODE_RS2,
    DECODE_SEQ
};

enum{
    RENAME_VALID = 0,
    RENAME_PC,
    RENAME_OP,
    RENAME_RD,
    RENAME_RS1,
    RENAME_RS2,
    RENAME_SEQ
};

enum{
    REGREAD_VALID = 0,
    REGREAD_PC,
    REGREAD_OP,
    REGREAD_DEST,
    REGREAD_IS_ROB1,
    REGREAD_S1,
    REGREAD_IS_ROB2,
    REGREAD_S2,
    REGREAD_SEQ
};

enum{
    DISPATCH_VALID = 0,
    DISPATCH_PC,
    DISPATCH_OP,
    DISPATCH_DEST,
    DISPATCH_IS_ROB1,
    DISPATCH_S1_RDY,
    DISPATCH_S1,
    DISPATCH_IS_ROB2,
    DISPATCH_S2_RDY,
    DISPATCH_S2,
    DISPATCH_SEQ
};

enum{
    WRITEBACK_VALID = 0,
    WRITEBACK_ROB,
    WRITEBACK_SEQ
};

enum{
    RENAME_MAP_VALID = 0,
    RENAME_MAP_ROB
};

enum{
    ISSUE_VALID = 0,
    ISSUE_DEST,
    ISSUE_OP,
    ISSUE_RS1_RDY,
    ISSUE_RS1,
    ISSUE_RS2_RDY,
    ISSUE_RS2,
    ISSUE_SEQ
};

enum{
    EXECUTE_VALID = 0,
    EXECUTE_DEST,
    EXECUTE_TIME_LEFT,
    EXECUTE_SEQ
};

enum{
    REORDER_DEST = 0,
    REORDER_RDY,
    REORDER_EXC,
    REORDER_MIS,
    REORDER_PC,
    REORDER_SEQ
};

#endif
