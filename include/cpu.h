#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "interrupt.h"

//#define LOGFILE "log.txt"

#ifdef LOGFILE
#include <fstream>
#include <iomanip>
#endif

typedef void * OpArg;

// To avoid circular dependencies
class Bus;


class CPU {
public:
    CPU();
    ~CPU() = default;

public:
    uint8_t clock();
    void reset();

    void connectBus(Bus *bus);

private:
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    Bus *bus;

    union {struct {uint8_t f, a;}; uint16_t af;};
    union {struct {uint8_t c, b;}; uint16_t bc;};
    union {struct {uint8_t e, d;}; uint16_t de;};
    union {struct {uint8_t l, h;}; uint16_t hl;};
    uint16_t pc, sp;

    // Interrupt Master Enable Flag
    // Set and reset with EI and DI instructions
    bool ime;

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
    std::vector<INSTRUCTION> lookup_cb;

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

#ifdef LOGFILE
    std::ofstream file;
    void print_log(uint8_t opcode, INSTRUCTION instr);
#endif


public:
    std::string disassemble(INSTRUCTION instr);

private:
    void flipFlag(FLAG flag);
    void setFlag(FLAG flag, bool val);
    bool getFlag(FLAG flag);
    bool checkCond(COND cond);

    // Checks for interrupts that need to be handled and runs a CALL instruction if necesssary
    // Returns true if an interrupt is being serviced
    bool handleInterrupt();

private:
    // Opcode implementations
    uint8_t UNKNOWN();

    uint8_t NOP();
    uint8_t STOP();
    uint8_t HALT();

    uint8_t EI();
    uint8_t DI();

    uint8_t DAA();

    uint8_t LD_REG_16_VAL_16();
    uint8_t LD_REG_8_VAL_8();
    uint8_t LD_MEM_VAL_16();
    uint8_t LD_MEM_VAL_8();
    uint8_t LD_REG_8_MEM();
    uint8_t LDH_MEM_VAL_8();
    uint8_t LDH_REG_8_MEM();
    uint8_t LDHL_REG_16_VAL_8();

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

    uint8_t PUSH();
    uint8_t POP();

    uint8_t CALL();
    uint8_t RET();
    uint8_t RETI();

    uint8_t RST();

    // Prefix CB opcodes
    uint8_t RLC_REG_8();
    uint8_t RLC_MEM();

    uint8_t RRC_REG_8();
    uint8_t RRC_MEM();

    uint8_t RL_REG_8();
    uint8_t RL_MEM();

    uint8_t RR_REG_8();
    uint8_t RR_MEM();

    uint8_t SLA_REG_8();
    uint8_t SLA_MEM();

    uint8_t SRA_REG_8();
    uint8_t SRA_MEM();

    uint8_t SWAP_REG_8();
    uint8_t SWAP_MEM();

    uint8_t SRL_REG_8();
    uint8_t SRL_MEM();

    uint8_t BIT_REG_8();
    uint8_t BIT_MEM();

    uint8_t RES_REG_8();
    uint8_t RES_MEM();

    uint8_t SET_REG_8();
    uint8_t SET_MEM();
};