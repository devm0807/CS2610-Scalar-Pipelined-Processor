int reg_file[16] = {0};
int pc = 0;
int ifbuff;
int idbuff;
int exbuff  ;
int membuff;
int opcode;
int rs;
int rt;
int rd;
int funct;
int imm;
int funct;

void instruction_fetch(int *instruction, int *pc)
{
    // *instruction = memory[*pc];
    // *pc = *pc + 1;
}

void instruction_decode(int instruction, int *opcode, int *rs, int *rt, int *rd, int *shamt, int *funct, int *imm)
{
    // *opcode = (instruction >> 26) & 0x3F;
    // *rs = (instruction >> 21) & 0x1F;
    // *rt = (instruction >> 16) & 0x1F;
    // *rd = (instruction >> 11) & 0x1F;
    // *shamt = (instruction >> 6) & 0x1F;
    // *funct = instruction & 0x3F;
    // *imm = instruction & 0xFFFF;
}


void execution(){

}

void memory_access(){

}


void write_back(){

}


void pipeline_unit(){

}

void mem_ref(){

}

void reg_reg_ALU(){

}

void branch(){

}