#include <vector>

#include "cpu.h"
#include "bus.h"


CPU::CPU() {
    // Initialize instruction lookup table
    lookup = {
        // name, base_clock, data_len, args, fn_pointer
        {"NOP",             4,  0,   0,             0,              &CPU::NOP},
        {"LD BC, {d16}",    12, 2,   &bc,           &fetched,       &CPU::LD_REG_VAL_16},
        {"LD (BC), A",      8,  0,   &bc,           &a,             &CPU::LD_MEM_VAL_8},
        {"INC BC",          8,  0,   &bc,           (OpArg)  1,     &CPU::INC_REG_16},
        {"INC B",           4,  0,   &b,            (OpArg)  1,     &CPU::INC_REG_8},
        {"DEC B",           4,  0,   &b,            (OpArg) -1,     &CPU::INC_REG_8},
        {"LD B, {d8}",      8,  1,   &b,            &fetched,       &CPU::LD_REG_VAL_8},
        {"RLCA",            4,  0,   &a,            (OpArg) 1,      &CPU::ROT_C_REG},
        {"LD ({a16}), SP",  20, 2,   &fetched,      &sp,            &CPU::LD_MEM_VAL_16},
        {"ADD HL, BC",      8,  0,   &hl,           &bc,            &CPU::ADD_REG_VAL_16},
        {"LD A, (BC)",      8,  0,   &a,            &bc,            &CPU::LD_REG_MEM_8},
        {"DEC BC",          8,  0,   &bc,           (OpArg) -1,     &CPU::INC_REG_16},
        {"INC C",           4,  0,   &c,            (OpArg)  1,     &CPU::INC_REG_8},
        {"DEC C",           4,  0,   &c,            (OpArg) -1,     &CPU::INC_REG_8},
        {"LD C, {d8}",      8,  1,   &c,            &fetched,       &CPU::LD_REG_VAL_8},
        {"RRCA",            4,  0,   &a,            (OpArg) -1,     &CPU::ROT_C_REG},

        {"STOP",            4,  1,   0,             0,              &CPU::STOP},
        {"LD DE, {d16}",    12, 2,   &de,           &fetched,       &CPU::LD_REG_VAL_16},
        {"LD (DE), A",      8,  0,   &de,           &a,             &CPU::LD_MEM_VAL_8},
        {"INC DE",          8,  0,   &de,           (OpArg)  1,     &CPU::INC_REG_16},
        {"INC D",           4,  0,   &d,            (OpArg)  1,     &CPU::INC_REG_8},
        {"DEC D",           4,  0,   &d,            (OpArg) -1,     &CPU::INC_REG_8},
        {"LD D, {d8}",      8,  1,   &d,            &fetched,       &CPU::LD_REG_VAL_8},
        {"RLA",             4,  0,   &a,            (OpArg)  1,     &CPU::ROT_REG},
        {"JR {d8}",         12, 1,   (OpArg) NONE,  &fetched,       &CPU::JR},
        {"ADD HL, DE",      8,  0,   &hl,           &de,            &CPU::ADD_REG_VAL_16},
        {"LD A, (DE)",      8,  0,   &a,            &de,            &CPU::LD_REG_MEM_8},
        {"DEC DE",          8,  0,   &de,           (OpArg) -1,     &CPU::INC_REG_16},
        {"INC E",           4,  0,   &e,            (OpArg)  1,     &CPU::INC_REG_8},
        {"DEC E",           4,  0,   &e,            (OpArg) -1,     &CPU::INC_REG_8},
        {"LD E, {d8}",      8,  1,   &e,            &fetched,       &CPU::LD_REG_VAL_8},
        {"RRA",             4,  0,   &a,            (OpArg) -1,     &CPU::ROT_REG},
        
        {"JR NZ, {d8}",     8,  1,   (OpArg) IS_NZ, &fetched,       &CPU::JR},
        {"LD HL, {d16}",    12, 2,   &hl,           &fetched,       &CPU::LD_REG_VAL_16},
        //{"LDI (HL), A",     8,  0,   &hl,           &fetched,       &CPU::LDI_MEM_VAL_16},
        //{"INC HL",          8,  0,   &hl,           (OpArg)  1,     &CPU::INC_REG_16},
        //{"INC H",           4,  0,   &h,            (OpArg)  1,     &CPU::INC_REG_8},
        //{"DEC H",           4,  0,   &h,            (OpArg)  1,     &CPU::INC_REG_8},
        //{"LD H, {d8}",      8,  1,   &h,            &fetched,       &CPU::LD_REG_VAL_8},
        //{"DAA",             4,  1    &a,            0,              &CPU::DA},
        //{"JR Z, {d8}",      8,  1,   (OpArg) IS_Z , &fetched,       &CPU::JR},
        //{"ADD HL, HL",      8,  0,   &hl,           &hl,            &CPU::ADD_REG_VAL_16},
        //{"LDI HL, A",       8,  0,   &hl,           &fetched,       &CPU::LDI_REG_VAL_16},
    };
}

// read & write to/from bus
uint8_t CPU::read(uint16_t addr) {
    return bus->read(addr);
}

void CPU::write(uint16_t addr, uint8_t data) {
    bus->write(addr, data);
}

void CPU::connectBus(Bus *bus) {
    this->bus = bus;
}


// CPU functions
void CPU::reset() {
    f = 0xB0;
    af = 0x0001;
    bc = 0x0013;
    de = 0x00D8;
    hl = 0x014D;
    pc = 0x0100;
    sp = 0xFFFE;
}

void CPU::clock() {
    // If there are no more cycles left to complete, then we can execute the next instruction
    if (cycles == 0) {
        // Fetch next instruction and increment pc
        uint8_t opcode = read(pc++);
        INSTRUCTION instr = lookup[opcode];

        // Fetch data -- 0, 1, or 2 bytes
        for (int8_t i = 0; i < instr.data_len; i++) {
            fetched <<= 8;
            fetched += read(pc++);
        }

        // Set arguments
        arg1 = instr.arg1;
        arg2 = instr.arg2;
        
        // Increment clock
        cycles += instr.base_clock;
    
        // Instructions will return the number of extra cycles necessary
        cycles += (this->*instr.operate)();
    }

    cycles--;
    clock_count++;
}


// OPCODE IMPLEMENTATIONS
// Many of these use arg1 and arg2 in different ways or not at all (eg. NOP)
// Most commonly arg1 and arg2 will be the memory address of a register or immediate value (fetched)

uint8_t CPU::NOP() {
    return 0;
}

// TODO
uint8_t CPU::STOP() {
    return 0;
}

// for LD instructions, arg1 = destination, arg2 = source
// LD_XXX_XXX refers to how first and second arg will be treated respectively
// i.e. REG or VAL will simply be dereferenced, MEM will be dereferenced and used to read from memory
uint8_t CPU::LD_REG_VAL_16() {
    *(uint16_t *) arg1 = *(uint16_t *) arg2;
    return 0;
}

uint8_t CPU::LD_REG_VAL_8() {
    *(uint8_t *) arg1 = *(uint8_t *) arg2;
    return 0;
}

uint8_t CPU::LD_MEM_VAL_16() {
    return 0;
}

uint8_t CPU::LD_MEM_VAL_8() {
    write(*(uint16_t *) arg1, *(uint8_t *) arg2);
    return 0;
}

uint8_t CPU::LD_REG_MEM_8() {
    *(uint8_t *) arg1 = read(*(uint16_t *) arg2);
    return 0;
}

// for INC and DEC instructions, arg1 = location, arg2 = +/- 1 to represent INC/DEC
// these could have been merged with ADD & SUB instructions, but they do not alter flags
uint8_t CPU::INC_REG_16() {
    *(uint16_t *) arg1 += (int64_t) arg2;
    return 0;
}

uint8_t CPU::INC_REG_8() {
    *(uint8_t *) arg1 += (int64_t) arg2;
    return 0;
}

uint8_t CPU::INC_MEM_8() {
    uint8_t val = read(*(uint16_t *) arg1);
    val += (int64_t) arg2;
    write(*(uint16_t *) arg1, val);

    return 0;
}

uint8_t CPU::ADD_REG_VAL_16() {}
uint8_t CPU::ADD_REG_VAL_8 () {}
uint8_t CPU::ADD_REG_MEM_8 () {}

uint8_t CPU::SUB_REG_VAL_8() {}

uint8_t CPU::ROT_C_REG() {}
uint8_t CPU::ROT_C_MEM() {}
uint8_t CPU::ROT_REG() {}
uint8_t CPU::ROT_MEM() {}

uint8_t CPU::JR() {}