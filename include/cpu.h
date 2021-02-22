#pragma once

#include <cstdint>
#include <string>
#include <vector>

#define LOGFILE "log.txt"

#ifdef LOGFILE
#include <fstream>
#endif

typedef void * OpArg;

// To avoid circular dependencies
class Bus;

// interrupt flag/enable register locations
#define IE 0xFFFF
#define IF 0xFF0F

// interrupt start addresses
#define VBLANK_A 0x0040
#define LCDS_A 0x0048
#define TIMER_A 0x0050
#define SERIAL_A 0x0058
#define JOYPAD_A 0x0060


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

    enum INTERRUPT {
        V_BLANK = 1 << 0,
        LCD_STAT = 1 << 1,
        TIMER = 1 << 2,
        SERIAL = 1 << 3,
        JOYPAD = 1 << 4,
    };

    void irq(INTERRUPT intr);

private:
    Bus *bus;

    union {struct {uint8_t f, a;}; uint16_t af;};
    union {struct {uint8_t c, b;}; uint16_t bc;};
    union {struct {uint8_t e, d;}; uint16_t de;};
    union {struct {uint8_t l, h;}; uint16_t hl;};
    uint16_t pc, sp;

    // Interrupt Master Enable Flag
    // Set and reset with EI and DI instructions
    bool ime;

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
    void setFlag(FLAG flag, bool val);
    bool getFlag(FLAG flag);
    bool checkCond(COND cond);

    // Checks for interrupts that need to be handled and runs a CALL instruction if necesssary
    // Returns true if an interrupt is being serviced
    bool handleInterrupt();

private:
    // Opcode implementations
    uint8_t PREFIX_CB();
    uint8_t UNKNOWN();

    uint8_t NOP();
    uint8_t STOP();
    uint8_t HALT();

    uint8_t EI();
    uint8_t DI();

    uint8_t DA();

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
    uint8_t JP_MEM();

    uint8_t PUSH();
    uint8_t POP();

    uint8_t CALL();
    uint8_t RET();
    uint8_t RETI();

    uint8_t RST();
};