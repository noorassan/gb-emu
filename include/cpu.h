#pragma once

#include <cstdint>
#include <string>
#include <vector>

typedef void * OpArg;

// To avoid circular dependencies
class Bus;


class CPU {
public:
    CPU();
    ~CPU() = default;

public:
    void clock();
    void reset();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
    void connectBus(Bus *bus);

public:
    Bus *bus;

    union {struct {uint8_t f, a;}; uint16_t af;};
    union {struct {uint8_t c, b;}; uint16_t bc;};
    union {struct {uint8_t e, d;}; uint16_t de;};
    union {struct {uint8_t l, h;}; uint16_t hl;};
    uint16_t pc, sp;

    uint8_t cycles;
    uint64_t clock_count;

    uint16_t fetched;

    OpArg arg1, arg2;

    struct INSTRUCTION {
        std::string name;
        uint8_t base_clock;
        uint8_t data_len;
        OpArg arg1, arg2;
        uint8_t (CPU::*operate)();
    };

    std::vector<INSTRUCTION> lookup;

    enum FLAG {
        Z = 1 << 7,
        N = 1 << 6,
        H = 1 << 5,
        C = 1 << 4,
    };

    enum COND {
        NONE,
        IS_Z,
        IS_NZ,
        IS_C,
        IS_NC,
    };

public:
    void setFlag(FLAG flag, bool val);
    bool getFlag(FLAG flag);
    bool checkCond(COND cond);

private:
    // Opcode implementations
    uint8_t NOP();
    uint8_t STOP();
    uint8_t HALT();

    uint8_t DA();

    uint8_t LD_REG_16_VAL_16();
    uint8_t LD_REG_8_VAL_8();
    uint8_t LD_MEM_VAL_16();
    uint8_t LD_MEM_VAL_8();
    uint8_t LD_REG_8_MEM();

    uint8_t INC_REG_16();
    uint8_t INC_REG_8();
    uint8_t INC_MEM();

    uint8_t DEC_REG_16();
    uint8_t DEC_REG_8();
    uint8_t DEC_MEM();

    uint8_t LDI_MEM_VAL_8();
    uint8_t LDI_REG_8_MEM();
    uint8_t LDD_MEM_VAL_8();
    uint8_t LDD_REG_8_MEM();

    uint8_t ADD_REG_16_VAL_16();
    uint8_t ADD_REG_16_VAL_8();
    uint8_t ADD_REG_8_VAL_8();
    uint8_t ADD_REG_8_MEM();
    uint8_t ADC_REG_8_VAL_8();
    uint8_t ADC_REG_8_MEM();

    uint8_t SUB_REG_8_VAL_8();
    uint8_t SUB_REG_8_MEM();
    uint8_t SBC_REG_8_VAL_8();
    uint8_t SBC_REG_8_MEM();

    uint8_t AND_REG_8_VAL_8();
    uint8_t AND_REG_8_MEM();

    uint8_t XOR_REG_8_VAL_8();
    uint8_t XOR_REG_8_MEM();

    uint8_t OR_REG_8_VAL_8();
    uint8_t OR_REG_8_MEM();

    uint8_t CP_REG_8_VAL_8();
    uint8_t CP_REG_8_MEM();

    uint8_t CPL();
    uint8_t SCF();
    uint8_t CCF();

    uint8_t RLA();
    uint8_t RRA();
    uint8_t RLCA();
    uint8_t RRCA();

    uint8_t JR();
    uint8_t JP();
    uint8_t POP();
    uint8_t RET();
};