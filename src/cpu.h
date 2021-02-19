#pragma once

#include <cstdint>
#include <string>
#include <vector>

typedef void * OpArg;

// to avoid circular dependencies
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
    uint16_t hl, pc, sp;

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

    enum FLAGS {
        Z = 1 << 7,
        C = 1 << 4,
    };

    enum CONDS {
        NONE,
        IS_Z,
        IS_NZ,
        IS_C,
        IS_NC,
    };


private:
    // Opcode implementations
    uint8_t NOP();
    uint8_t STOP();

    uint8_t LD_REG_VAL_16();
    uint8_t LD_REG_VAL_8();
    uint8_t LD_MEM_VAL_16();
    uint8_t LD_MEM_VAL_8();
    uint8_t LD_REG_MEM_8();

    uint8_t INC_REG_16();
    uint8_t INC_REG_8();
    uint8_t INC_MEM_8();

    uint8_t ADD_REG_VAL_16();
    uint8_t ADD_REG_VAL_8 ();
    uint8_t ADD_REG_MEM_8 ();

    uint8_t SUB_REG_VAL_8();

    uint8_t ROT_C_REG();
    uint8_t ROT_C_MEM();
    uint8_t ROT_REG();
    uint8_t ROT_MEM();

    uint8_t JR();
};