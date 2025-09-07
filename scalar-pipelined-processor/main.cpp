#include <bits/stdc++.h>
using namespace std;

int RF[16] = {0};
int ICache[256];
int DCache[256];

int reg_occupied[16] = {0};

int PC = 0;
int IR, opcode, rd, rs1, rs2, ALUOutput, LMD;
int A, B, Imm;

// ########### initialising counts ##########
int total_inst = 0, arithmetic_inst = 0, logical_inst = 0, shift_inst = 0, memory_inst = 0, control_inst = 0, halt_inst = 0, li_inst = 0;
int total_cycles = 0;
int total_stalls = 0, data_stalls = 0, control_stalls = 0;

ifstream icache, dcache, rf;

int raw_stall = 0;
int stall_counter = 0;

int halt = 0;
int dependency = 0;

// Initialising the buffers
struct buffer
{
    int IR = 0, opcode = 16, rd = 0, rs1 = 0, rs2 = 0, ALUOutput = 0, LMD = 0;
    int A = 0, B = 0, Imm = 0;
    bool stalled = false;
};

buffer fetch_buffer, decode_buffer, execute_buffer, mem_buffer;

int hex_to_decimal(char ch)
{
    switch (ch)
    {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
        return 10;
    case 'b':
        return 11;
    case 'c':
        return 12;
    case 'd':
        return 13;
    case 'e':
        return 14;
    case 'f':
        return 15;
    default:
        return -1;
    }
}

// Function to read the contents of the file
void file_read()
{
    icache.open("input/ICache.txt");
    dcache.open("input/DCache.txt");
    rf.open("input/RF.txt");
    string line;
    int i = 0;
    while (icache >> line)
    {
        int dec_ins = 0;
        int p = line.length();
        for (int j = 0; j < p; j++)
        {
            dec_ins += hex_to_decimal(line[j]) * pow(16, p - j - 1);
        }
        ICache[i] = dec_ins;
        i++;
    }
    i = 0;
    while (dcache >> line)
    {
        int dec_data = 0;
        int p = line.length();
        for (int j = 0; j < p; j++)
        {
            dec_data += hex_to_decimal(line[j]) * pow(16, p - j - 1);
        }
        DCache[i] = dec_data;
        i++;
    }
    i = 0;
    while (rf >> line)
    {
        int dec_reg = 0;
        int p = line.length();
        for (int j = 0; j < p; j++)
        {
            dec_reg += hex_to_decimal(line[j]) * pow(16, p - j - 1);
        }
        RF[i] = dec_reg;
        i++;
    }
    icache.close();
    dcache.close();
    rf.close();
}

vector<int> ari_op;
void writeback_stage()
{
    // write-back stage
    if (mem_buffer.opcode < 11)
    {
        RF[mem_buffer.rd] = mem_buffer.ALUOutput;
        reg_occupied[mem_buffer.rd] = 0;
    }
    else if (mem_buffer.opcode == 11)
    {
        RF[mem_buffer.rd] = mem_buffer.LMD;
        reg_occupied[mem_buffer.rd] = 0;
    }
    switch (mem_buffer.opcode)
    {

    case 0: // ADD instruction
    case 1: // SUB instruction
    case 2: // MUL instruction
    case 3: // INC instruction
        ari_op.push_back(mem_buffer.opcode);
        arithmetic_inst++;
        total_inst++;
        break;
    case 4: // AND instruction
    case 5: // OR instruction
    case 6: // XOR instruction
    case 7: // NOT instruction
        total_inst++;
        logical_inst++;
        break;
    case 8: // SLLI instruction
    case 9: // SRLI instruction
        total_inst++;
        shift_inst++;
        break;
    case 10: // LI instruction
        total_inst++;
        li_inst++;
        break;
    case 11: // LD instruction
    case 12: // ST instruction
        total_inst++;
        memory_inst++;
        break;
    case 13: // JMP instruction
    case 14: // BEQZ instruction
    case 15: // HLT instruction;
        break;
    }
}

void unsigned_to_signed(int &k) // dealing with signed/unsignedness
{
    k += 128;
    k %= 256;
    k -= 128;
}
void mem_stage()
{
    // memory access stage
    mem_buffer = execute_buffer;
    if (mem_buffer.opcode == 11)
    {
        mem_buffer.LMD = DCache[RF[mem_buffer.ALUOutput]];
    }
    else if (mem_buffer.opcode == 12)
    {
        DCache[RF[mem_buffer.ALUOutput]] = mem_buffer.B;
    }
}

void execute_stage()
{
    // execute stage

    execute_buffer = decode_buffer;
    execute_buffer.A = RF[execute_buffer.rs1];
    execute_buffer.B = RF[execute_buffer.rs2];
    switch (execute_buffer.opcode)
    {
    case 0: // ADD instruction
        execute_buffer.ALUOutput = execute_buffer.A + execute_buffer.B;
        unsigned_to_signed(execute_buffer.ALUOutput);
        break;
    case 1: // SUB instruction
        execute_buffer.ALUOutput = execute_buffer.A - execute_buffer.B;
        unsigned_to_signed(execute_buffer.ALUOutput);
        break;
    case 2: // MUL instruction
        execute_buffer.ALUOutput = execute_buffer.A * execute_buffer.B;
        unsigned_to_signed(execute_buffer.ALUOutput);
        break;
    case 3: // INC instruction
        execute_buffer.ALUOutput = execute_buffer.A + 1;
        unsigned_to_signed(execute_buffer.ALUOutput);
        break;
    case 4: // AND instruction
        execute_buffer.ALUOutput = execute_buffer.A & execute_buffer.B;
        break;
    case 5: // OR instruction
        execute_buffer.ALUOutput = execute_buffer.A | execute_buffer.B;
        break;
    case 6: // XOR instruction
        execute_buffer.ALUOutput = execute_buffer.A ^ execute_buffer.B;
        break;
    case 7: // NOT instruction
        execute_buffer.ALUOutput = ~execute_buffer.A;
        break;
    case 8: // SLLI instruction
        execute_buffer.ALUOutput = execute_buffer.A << execute_buffer.Imm;
        break;
    case 9: // SRLI instruction
        execute_buffer.ALUOutput = execute_buffer.A >> execute_buffer.Imm;
        break;
    case 10: // LI instruction
        execute_buffer.ALUOutput = execute_buffer.Imm;
        if (execute_buffer.ALUOutput > 127)
        {
            execute_buffer.ALUOutput -= 256;
        }
        break;
    case 11: // LOAD instruction
        if (execute_buffer.Imm > 7)
        {
            execute_buffer.Imm -= 16;
        }
        execute_buffer.ALUOutput = execute_buffer.A + execute_buffer.Imm;
        break;
    case 12: // STORE instruction
        if (execute_buffer.Imm > 7)
        {
            execute_buffer.Imm -= 16;
        }
        execute_buffer.ALUOutput = execute_buffer.A + execute_buffer.Imm;
        break;
    case 13: // JMP instruction
        PC = execute_buffer.ALUOutput;
        break;
    case 14: // BEQZ instruction
        unsigned_to_signed(execute_buffer.Imm);
        if (execute_buffer.A == 0 && stall_counter == 1)
        {
            PC = PC + (execute_buffer.Imm << 1);
        }
        break;
    }
}
int n_jumps = 0;
void decode_stage()
{
    // Decode logic
    decode_buffer = fetch_buffer;
    vector<int> decode(4, 0);
    int temp = fetch_buffer.IR;
    decode[3] = temp & 15;
    temp = temp >> 4;
    decode[2] = temp & 15;
    temp = temp >> 4;
    decode[1] = temp & 15;
    temp = temp >> 4;
    decode[0] = temp & 15;
    temp = temp >> 4;
    if (PC == 0)
    {
        decode[0] = 16;
    }
    decode_buffer.opcode = decode[0];
    switch (decode[0])
    {
    case 0: // ADD instruction
    case 1: // SUB instruction
    case 2: // MUL instruction
    case 4: // AND instruction
    case 5: // OR instruction
    case 6: // XOR instruction
        decode_buffer.rd = decode[1];
        decode_buffer.rs1 = decode[2];
        decode_buffer.rs2 = decode[3];
        raw_stall = 0;
        if ((reg_occupied[decode[2]] != 0) || (reg_occupied[decode[3]] != 0))
        {
            data_stalls++;
            raw_stall = 1;
            // data_stalls++;
            decode_buffer.opcode = 16;
        }

        break;
    case 3: // INC instruction
    case 7: // NOT instruction
        decode_buffer.rd = decode[1];
        decode_buffer.rs1 = decode[2];
        raw_stall = 0;
        if (reg_occupied[decode[2]] > 0)
        {
            data_stalls++;
            raw_stall++;
            // data_stalls++;
            decode_buffer.opcode = 16;
        }
        break;
    case 8:  // SLLI instruction
    case 9:  // SRLI instruction
    case 11: // LD instruction
    case 12: // ST instruction
        raw_stall = 0;
        if (reg_occupied[decode[2]] > 0)
        {
            data_stalls++;
            raw_stall++;
            // data_stalls++;
            decode_buffer.opcode = 16;
        }
        decode_buffer.rd = decode[1];
        decode_buffer.rs1 = decode[2];
        decode_buffer.Imm = decode[3];
        break;
    case 10: // LI instruction
        decode_buffer.rd = decode[1];
        decode_buffer.Imm = (decode[2] << 4) + decode[3];
        break;
    case 13: // JMP instruction
        decode_buffer.Imm = (decode[1] << 4) + decode[2];
        if (decode_buffer.Imm > 127)
        {
            decode_buffer.Imm -= 256;
        }
        stall_counter += 2;

        control_stalls += 2;
        n_jumps++;
        decode_buffer.ALUOutput = PC + ((decode_buffer.Imm << 1));
        break;
    case 14: // BEQZ instruction
        raw_stall = 0;
        if (reg_occupied[decode[1]] >= 1)
        {
            raw_stall++;
            data_stalls++;
            decode_buffer.opcode = 16;
        }
        else
        {
            decode_buffer.rs1 = decode[1];
            decode_buffer.Imm = (decode[2] << 4) + decode[3];
            control_stalls += 2;
            stall_counter += 2;
        }
        break;
    case 15: // HLT instruction
        // halt_inst++;
        halt = 1;
    }
    if (decode_buffer.opcode < 12)
    {
        reg_occupied[decode_buffer.rd] += 1;
    }
}

void fetch_stage()
{
    // fetch stage
    fetch_buffer.IR = (ICache[PC] << 8) + ICache[PC + 1];
    // cout << fetch_buffer.IR<<endl;
    vector<int> decode(4, 0);
    int temp = fetch_buffer.IR;
    decode[3] = temp & 15;
    temp = temp >> 4;
    decode[2] = temp & 15;
    temp = temp >> 4;
    decode[1] = temp & 15;
    temp = temp >> 4;
    decode[0] = temp & 15;
    temp = temp >> 4;

    switch (decode[0])
    {
    case 13: // JMP instruction
    case 14: // BEQZ instruction

        control_inst++;
        total_inst++;
        break;
    case 15: // HLT instruction
        halt_inst++;
        total_inst++;
        break;
    }
    PC += 2;
}

int m1 = 0;
void flow()
{
    int cycles = 3;
    while (!halt || cycles)
    {
        if (halt)
        {
            cycles--;
        }
        total_cycles++;
        writeback_stage();
        mem_stage();
        execute_stage();
        if (stall_counter > 0)
        {
            m1++;
        }
        if (!stall_counter)
        {
            decode_stage();
        }
        else
        {
            stall_counter--;
            if (!stall_counter)
                decode_buffer.opcode = 16;
        }
        if (!raw_stall && !stall_counter && !halt)
        {
            fetch_stage();
        }
    }

    total_stalls = data_stalls + control_stalls;
}

int main()
{
    // ########### taking input #################
    file_read();

    // ###### instruction processing ############
    flow();

    // ########## output values ##################
    ofstream out;
    out.open("output/Output.txt");

    out << "Total number of instructions executed:" << total_inst << endl;
    out << "Number of instructions in each class" << endl;
    out << "Arithmetic instructions              :" << arithmetic_inst << endl;
    out << "Logical instructions                 :" << logical_inst << endl;
    out << "Shift instructions                   :" << shift_inst << endl;
    out << "Memory instructions                  :" << memory_inst << endl;
    out << "Load immediate instructions          :" << li_inst << endl;
    out << "Control instructions                 :" << control_inst << endl;
    out << "Halt instructions                    :" << halt_inst << endl;
    out << "Cycles Per Instruction               :" << (double)total_cycles / total_inst << endl;
    out << "Total number of stalls               :" << total_stalls << endl;
    out << "Data stalls (RAW)                    :" << data_stalls << endl;
    out << "Control stalls                       :" << control_stalls << endl;

    out.close();

    out.open("output/ODCache.txt");

    for (int i = 0; i < 256; i++)
    {
        if (DCache[i] >= 16)
            out << hex << DCache[i] << endl;
        else
        {
            out << "0" << hex << DCache[i] << endl;
        }
    }
    out.close();
}