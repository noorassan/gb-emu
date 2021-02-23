#include <vector>
#include <cstdint>
#include <stdexcept>

#include "cpu.h"
#include "bus.h"

#define DR_16(X) (*(uint16_t *) X)
#define DR_8(X) (*(uint8_t *) X)


CPU::CPU() {
    // Initialize instruction lookup table
#ifdef LOGFILE
    file.open(LOGFILE, std::ofstream::out);
#endif

    reset();
    lookup = {
        // name, base_clock, data_len, args, fn_pointer
        {"NOP",             4,  0,   0,             0,              &CPU::NOP},
        {"LD BC, {d16}",    12, 2,   &bc,           &fetched,       &CPU::LD_REG_16_VAL_16},
        {"LD (BC), A",      8,  0,   &bc,           &a,             &CPU::LD_MEM_VAL_8},
        {"INC BC",          8,  0,   &bc,           0,              &CPU::INC_REG_16},
        {"INC B",           4,  0,   &b,            0,              &CPU::INC_REG_8},
        {"DEC B",           4,  0,   &b,            0,              &CPU::DEC_REG_8},
        {"LD B, {d8}",      8,  1,   &b,            &fetched,       &CPU::LD_REG_8_VAL_8},
        {"RLCA",            4,  0,   &a,            0,              &CPU::RLCA},
        {"LD ({d16}), SP",  20, 2,   &fetched,      &sp,            &CPU::LD_MEM_VAL_16},
        {"ADD HL, BC",      8,  0,   &hl,           &bc,            &CPU::ADD_REG_16_VAL_16},
        {"LD A, (BC)",      8,  0,   &a,            &bc,            &CPU::LD_REG_8_MEM},
        {"DEC BC",          8,  0,   &bc,           0,              &CPU::DEC_REG_16},
        {"INC C",           4,  0,   &c,            0,              &CPU::INC_REG_8},
        {"DEC C",           4,  0,   &c,            0,              &CPU::DEC_REG_8},
        {"LD C, {d8}",      8,  1,   &c,            &fetched,       &CPU::LD_REG_8_VAL_8},
        {"RRCA",            4,  0,   &a,            0,              &CPU::RRCA},
        // 0x10
        {"STOP",            4,  1,   0,             0,              &CPU::STOP},
        {"LD DE, {d16}",    12, 2,   &de,           &fetched,       &CPU::LD_REG_16_VAL_16},
        {"LD (DE), A",      8,  0,   &de,           &a,             &CPU::LD_MEM_VAL_8},
        {"INC DE",          8,  0,   &de,           0,              &CPU::INC_REG_16},
        {"INC D",           4,  0,   &d,            0,              &CPU::INC_REG_8},
        {"DEC D",           4,  0,   &d,            0,              &CPU::DEC_REG_8},
        {"LD D, {d8}",      8,  1,   &d,            &fetched,       &CPU::LD_REG_8_VAL_8},
        {"RLA",             4,  0,   &a,            0,              &CPU::RLA},
        {"JR {r8}",         8,  1,   (OpArg) NONE,  &fetched,       &CPU::JR},
        {"ADD HL, DE",      8,  0,   &hl,           &de,            &CPU::ADD_REG_16_VAL_16},
        {"LD A, (DE)",      8,  0,   &a,            &de,            &CPU::LD_REG_8_MEM},
        {"DEC DE",          8,  0,   &de,           0,              &CPU::DEC_REG_16},
        {"INC E",           4,  0,   &e,            0,              &CPU::INC_REG_8},
        {"DEC E",           4,  0,   &e,            0,              &CPU::DEC_REG_8},
        {"LD E, {d8}",      8,  1,   &e,            &fetched,       &CPU::LD_REG_8_VAL_8},
        {"RRA",             4,  0,   &a,            0,              &CPU::RRA},
        // 0x20
        {"JR NZ, {r8}",     8,  1,   (OpArg) IS_NZ, &fetched,       &CPU::JR},
        {"LD HL, {d16}",    12, 2,   &hl,           &fetched,       &CPU::LD_REG_16_VAL_16},
        {"LDI (HL), A",     8,  0,   &hl,           &a,             &CPU::LDI_MEM_VAL_8},
        {"INC HL",          8,  0,   &hl,           0,              &CPU::INC_REG_16},
        {"INC H",           4,  0,   &h,            0,              &CPU::INC_REG_8},
        {"DEC H",           4,  0,   &h,            0,              &CPU::DEC_REG_8},
        {"LD H, {d8}",      8,  1,   &h,            &fetched,       &CPU::LD_REG_8_VAL_8},
        {"DAA",             4,  0,   &a,            0,              &CPU::DA},
        {"JR Z, {r8}",      8,  1,   (OpArg) IS_Z,  &fetched,       &CPU::JR},
        {"ADD HL, HL",      8,  0,   &hl,           &hl,            &CPU::ADD_REG_16_VAL_16},
        {"LDI A, (HL)",     8,  0,   &a,            &hl,            &CPU::LDI_REG_8_MEM},
        {"DEC HL",          8,  0,   &hl,           0,              &CPU::DEC_REG_16},
        {"INC L",           4,  0,   &l,            0,              &CPU::INC_REG_8},
        {"DEC L",           4,  0,   &l,            0,              &CPU::DEC_REG_8},
        {"LD L, {d8}",      8,  1,   &l,            &fetched,       &CPU::LD_REG_8_VAL_8},
        {"CPL",             4,  0,   &a,            0,              &CPU::CPL},
        // 0x30
        {"JR NC, {r8}",     8,  1,   (OpArg) IS_NC, &fetched,       &CPU::JR},
        {"LD SP, {d16}",    12, 2,   &sp,           &fetched,       &CPU::LD_REG_16_VAL_16},
        {"LDD (HL), A",     8,  0,   &hl,           &a,             &CPU::LDD_MEM_VAL_8},
        {"INC SP",          8,  0,   &sp,           0,              &CPU::INC_REG_16},
        {"INC (HL)",        12, 0,   &hl,           0,              &CPU::INC_MEM},
        {"DEC (HL)",        12, 0,   &hl,           0,              &CPU::DEC_MEM},
        {"LD (HL), {d8}",   12, 1,   &hl,           &fetched,       &CPU::LD_MEM_VAL_8},
        {"SCF",             4,  0,   &f,            (OpArg) C,      &CPU::SCF},
        {"JR C, {r8}",      8,  1,   (OpArg) IS_C,  &fetched,       &CPU::JR},
        {"ADD HL, SP",      8,  0,   &hl,           &sp,            &CPU::ADD_REG_16_VAL_16},
        {"LDD A, (HL)",     8,  0,   &a,            &hl,            &CPU::LDD_REG_8_MEM},
        {"DEC SP",          8,  0,   &sp,           0,              &CPU::DEC_REG_16},
        {"INC A",           4,  0,   &a,            0,              &CPU::INC_REG_8},
        {"DEC A",           4,  0,   &a,            0,              &CPU::DEC_REG_8},
        {"LD A, {d8}",      8,  1,   &a,            &fetched,       &CPU::LD_REG_8_VAL_8},
        {"CCF",             4,  0,   &f,            (OpArg) C,      &CPU::CCF},
        // 0x40
        {"LD B, B",         4,  0,   &b,            &b,             &CPU::LD_REG_8_VAL_8},
        {"LD B, C",         4,  0,   &b,            &c,             &CPU::LD_REG_8_VAL_8},
        {"LD B, D",         4,  0,   &b,            &d,             &CPU::LD_REG_8_VAL_8},
        {"LD B, E",         4,  0,   &b,            &e,             &CPU::LD_REG_8_VAL_8},
        {"LD B, H",         4,  0,   &b,            &h,             &CPU::LD_REG_8_VAL_8},
        {"LD B, L",         4,  0,   &b,            &l,             &CPU::LD_REG_8_VAL_8},
        {"LD B, (HL)",      8,  0,   &b,            &hl,            &CPU::LD_REG_8_MEM},
        {"LD B, A",         4,  0,   &b,            &a,             &CPU::LD_REG_8_VAL_8},
        {"LD C, B",         4,  0,   &c,            &b,             &CPU::LD_REG_8_VAL_8},
        {"LD C, C",         4,  0,   &c,            &c,             &CPU::LD_REG_8_VAL_8},
        {"LD C, D",         4,  0,   &c,            &d,             &CPU::LD_REG_8_VAL_8},
        {"LD C, E",         4,  0,   &c,            &e,             &CPU::LD_REG_8_VAL_8},
        {"LD C, H",         4,  0,   &c,            &h,             &CPU::LD_REG_8_VAL_8},
        {"LD C, L",         4,  0,   &c,            &l,             &CPU::LD_REG_8_VAL_8},
        {"LD C, (HL)",      8,  0,   &c,            &hl,            &CPU::LD_REG_8_MEM},
        {"LD C, A",         4,  0,   &c,            &a,             &CPU::LD_REG_8_VAL_8},
        // 0x50
        {"LD D, B",         4,  0,   &d,            &b,             &CPU::LD_REG_8_VAL_8},
        {"LD D, C",         4,  0,   &d,            &c,             &CPU::LD_REG_8_VAL_8},
        {"LD D, D",         4,  0,   &d,            &d,             &CPU::LD_REG_8_VAL_8},
        {"LD D, E",         4,  0,   &d,            &e,             &CPU::LD_REG_8_VAL_8},
        {"LD D, H",         4,  0,   &d,            &h,             &CPU::LD_REG_8_VAL_8},
        {"LD D, L",         4,  0,   &d,            &l,             &CPU::LD_REG_8_VAL_8},
        {"LD D, (HL)",      8,  0,   &d,            &hl,            &CPU::LD_REG_8_MEM},
        {"LD D, A",         4,  0,   &d,            &a,             &CPU::LD_REG_8_VAL_8},
        {"LD E, B",         4,  0,   &e,            &b,             &CPU::LD_REG_8_VAL_8},
        {"LD E, C",         4,  0,   &e,            &c,             &CPU::LD_REG_8_VAL_8},
        {"LD E, D",         4,  0,   &e,            &d,             &CPU::LD_REG_8_VAL_8},
        {"LD E, E",         4,  0,   &e,            &e,             &CPU::LD_REG_8_VAL_8},
        {"LD E, H",         4,  0,   &e,            &h,             &CPU::LD_REG_8_VAL_8},
        {"LD E, L",         4,  0,   &e,            &l,             &CPU::LD_REG_8_VAL_8},
        {"LD E, (HL)",      8,  0,   &e,            &hl,            &CPU::LD_REG_8_MEM},
        {"LD E, A",         4,  0,   &e,            &a,             &CPU::LD_REG_8_VAL_8},
        // 0x60
        {"LD H, B",         4,  0,   &h,            &b,             &CPU::LD_REG_8_VAL_8},
        {"LD H, C",         4,  0,   &h,            &c,             &CPU::LD_REG_8_VAL_8},
        {"LD H, D",         4,  0,   &h,            &d,             &CPU::LD_REG_8_VAL_8},
        {"LD H, E",         4,  0,   &h,            &e,             &CPU::LD_REG_8_VAL_8},
        {"LD H, H",         4,  0,   &h,            &h,             &CPU::LD_REG_8_VAL_8},
        {"LD H, L",         4,  0,   &h,            &l,             &CPU::LD_REG_8_VAL_8},
        {"LD H, (HL)",      8,  0,   &h,            &hl,            &CPU::LD_REG_8_MEM},
        {"LD H, A",         4,  0,   &h,            &a,             &CPU::LD_REG_8_VAL_8},
        {"LD L, B",         4,  0,   &l,            &b,             &CPU::LD_REG_8_VAL_8},
        {"LD L, C",         4,  0,   &l,            &c,             &CPU::LD_REG_8_VAL_8},
        {"LD L, D",         4,  0,   &l,            &d,             &CPU::LD_REG_8_VAL_8},
        {"LD L, E",         4,  0,   &l,            &e,             &CPU::LD_REG_8_VAL_8},
        {"LD L, H",         4,  0,   &l,            &h,             &CPU::LD_REG_8_VAL_8},
        {"LD L, L",         4,  0,   &l,            &l,             &CPU::LD_REG_8_VAL_8},
        {"LD L, (HL)",      8,  0,   &l,            &hl,            &CPU::LD_REG_8_MEM},
        {"LD L, A",         4,  0,   &l,            &a,             &CPU::LD_REG_8_VAL_8},
        // 0x70
        {"LD (HL), B",      8,  0,   &hl,           &b,             &CPU::LD_MEM_VAL_8},
        {"LD (HL), C",      8,  0,   &hl,           &c,             &CPU::LD_MEM_VAL_8},
        {"LD (HL), D",      8,  0,   &hl,           &d,             &CPU::LD_MEM_VAL_8},
        {"LD (HL), E",      8,  0,   &hl,           &e,             &CPU::LD_MEM_VAL_8},
        {"LD (HL), H",      8,  0,   &hl,           &h,             &CPU::LD_MEM_VAL_8},
        {"LD (HL), L",      8,  0,   &hl,           &l,             &CPU::LD_MEM_VAL_8},
        {"HALT",            4,  0,   0,             0,              &CPU::HALT},
        {"LD (HL), A",      8,  0,   &hl,           &a,             &CPU::LD_MEM_VAL_8},
        {"LD A, B",         4,  0,   &a,            &b,             &CPU::LD_REG_8_VAL_8},
        {"LD A, C",         4,  0,   &a,            &c,             &CPU::LD_REG_8_VAL_8},
        {"LD A, D",         4,  0,   &a,            &d,             &CPU::LD_REG_8_VAL_8},
        {"LD A, E",         4,  0,   &a,            &e,             &CPU::LD_REG_8_VAL_8},
        {"LD A, H",         4,  0,   &a,            &h,             &CPU::LD_REG_8_VAL_8},
        {"LD A, L",         4,  0,   &a,            &l,             &CPU::LD_REG_8_VAL_8},
        {"LD A, (HL)",      8,  0,   &a,            &hl,            &CPU::LD_REG_8_MEM},
        {"LD A, A",         4,  0,   &a,            &a,             &CPU::LD_REG_8_VAL_8},
        // 0x80
        {"ADD A, B",        4,  0,   &a,            &b,             &CPU::ADD_REG_8_VAL_8},
        {"ADD A, C",        4,  0,   &a,            &c,             &CPU::ADD_REG_8_VAL_8},
        {"ADD A, D",        4,  0,   &a,            &d,             &CPU::ADD_REG_8_VAL_8},
        {"ADD A, E",        4,  0,   &a,            &e,             &CPU::ADD_REG_8_VAL_8},
        {"ADD A, H",        4,  0,   &a,            &h,             &CPU::ADD_REG_8_VAL_8},
        {"ADD A, L",        4,  0,   &a,            &l,             &CPU::ADD_REG_8_VAL_8},
        {"ADD A, (HL)",     8,  0,   &a,            &hl,            &CPU::ADD_REG_8_MEM},
        {"ADD A, A",        4,  0,   &a,            &a,             &CPU::ADD_REG_8_VAL_8},
        {"ADC A, B",        4,  0,   &a,            &b,             &CPU::ADC_REG_8_VAL_8},
        {"ADC A, C",        4,  0,   &a,            &c,             &CPU::ADC_REG_8_VAL_8},
        {"ADC A, D",        4,  0,   &a,            &d,             &CPU::ADC_REG_8_VAL_8},
        {"ADC A, E",        4,  0,   &a,            &e,             &CPU::ADC_REG_8_VAL_8},
        {"ADC A, H",        4,  0,   &a,            &h,             &CPU::ADC_REG_8_VAL_8},
        {"ADC A, L",        4,  0,   &a,            &l,             &CPU::ADC_REG_8_VAL_8},
        {"ADC A, (HL)",     8,  0,   &a,            &hl,            &CPU::ADC_REG_8_MEM},
        {"ADC A, A",        4,  0,   &a,            &a,             &CPU::ADC_REG_8_VAL_8},
        // 0x90
        {"SUB B",           4,  0,   &a,            &b,             &CPU::SUB_REG_8_VAL_8},
        {"SUB C",           4,  0,   &a,            &c,             &CPU::SUB_REG_8_VAL_8},
        {"SUB D",           4,  0,   &a,            &d,             &CPU::SUB_REG_8_VAL_8},
        {"SUB E",           4,  0,   &a,            &e,             &CPU::SUB_REG_8_VAL_8},
        {"SUB H",           4,  0,   &a,            &h,             &CPU::SUB_REG_8_VAL_8},
        {"SUB L",           4,  0,   &a,            &l,             &CPU::SUB_REG_8_VAL_8},
        {"SUB (HL)",        8,  0,   &a,            &hl,            &CPU::SUB_REG_8_MEM},
        {"SUB A",           4,  0,   &a,            &a,             &CPU::SUB_REG_8_VAL_8},
        {"SBC A, B",        4,  0,   &a,            &b,             &CPU::SBC_REG_8_VAL_8},
        {"SBC A, C",        4,  0,   &a,            &c,             &CPU::SBC_REG_8_VAL_8},
        {"SBC A, D",        4,  0,   &a,            &d,             &CPU::SBC_REG_8_VAL_8},
        {"SBC A, E",        4,  0,   &a,            &e,             &CPU::SBC_REG_8_VAL_8},
        {"SBC A, H",        4,  0,   &a,            &h,             &CPU::SBC_REG_8_VAL_8},
        {"SBC A, L",        4,  0,   &a,            &l,             &CPU::SBC_REG_8_VAL_8},
        {"SBC A, (HL)",     8,  0,   &a,            &hl,            &CPU::SBC_REG_8_MEM},
        {"SBC A, A",        4,  0,   &a,            &a,             &CPU::SBC_REG_8_VAL_8},
        // 0xA0
        {"AND B",           4,  0,   &a,            &b,             &CPU::AND_REG_8_VAL_8},
        {"AND C",           4,  0,   &a,            &c,             &CPU::AND_REG_8_VAL_8},
        {"AND D",           4,  0,   &a,            &d,             &CPU::AND_REG_8_VAL_8},
        {"AND E",           4,  0,   &a,            &e,             &CPU::AND_REG_8_VAL_8},
        {"AND H",           4,  0,   &a,            &h,             &CPU::AND_REG_8_VAL_8},
        {"AND L",           4,  0,   &a,            &l,             &CPU::AND_REG_8_VAL_8},
        {"AND (HL)",        8,  0,   &a,            &hl,            &CPU::AND_REG_8_MEM},
        {"AND A",           4,  0,   &a,            &a,             &CPU::AND_REG_8_VAL_8},
        {"XOR B",           4,  0,   &a,            &b,             &CPU::XOR_REG_8_VAL_8},
        {"XOR C",           4,  0,   &a,            &c,             &CPU::XOR_REG_8_VAL_8},
        {"XOR D",           4,  0,   &a,            &d,             &CPU::XOR_REG_8_VAL_8},
        {"XOR E",           4,  0,   &a,            &e,             &CPU::XOR_REG_8_VAL_8},
        {"XOR H",           4,  0,   &a,            &h,             &CPU::XOR_REG_8_VAL_8},
        {"XOR L",           4,  0,   &a,            &l,             &CPU::XOR_REG_8_VAL_8},
        {"XOR (HL)",        8,  0,   &a,            &hl,            &CPU::XOR_REG_8_MEM},
        {"XOR A",           4,  0,   &a,            &a,             &CPU::XOR_REG_8_VAL_8},
        // 0xB0
        {"OR B",            4,  0,   &a,            &b,             &CPU::OR_REG_8_VAL_8},
        {"OR C",            4,  0,   &a,            &c,             &CPU::OR_REG_8_VAL_8},
        {"OR D",            4,  0,   &a,            &d,             &CPU::OR_REG_8_VAL_8},
        {"OR E",            4,  0,   &a,            &e,             &CPU::OR_REG_8_VAL_8},
        {"OR H",            4,  0,   &a,            &h,             &CPU::OR_REG_8_VAL_8},
        {"OR L",            4,  0,   &a,            &l,             &CPU::OR_REG_8_VAL_8},
        {"OR (HL)",         8,  0,   &a,            &hl,            &CPU::OR_REG_8_MEM},
        {"OR A",            4,  0,   &a,            &a,             &CPU::OR_REG_8_VAL_8},
        {"CP B",            4,  0,   &a,            &b,             &CPU::CP_REG_8_VAL_8},
        {"CP C",            4,  0,   &a,            &c,             &CPU::CP_REG_8_VAL_8},
        {"CP D",            4,  0,   &a,            &d,             &CPU::CP_REG_8_VAL_8},
        {"CP E",            4,  0,   &a,            &e,             &CPU::CP_REG_8_VAL_8},
        {"CP H",            4,  0,   &a,            &h,             &CPU::CP_REG_8_VAL_8},
        {"CP L",            4,  0,   &a,            &l,             &CPU::CP_REG_8_VAL_8},
        {"CP (HL)",         8,  0,   &a,            &hl,            &CPU::CP_REG_8_MEM},
        {"CP A",            4,  0,   &a,            &a,             &CPU::CP_REG_8_VAL_8},
        // 0xC0
        {"RET NZ",          8,  0,   (OpArg) IS_NZ, 0,              &CPU::RET}, 
        {"POP BC",          12, 0,   &bc,           0,              &CPU::POP},
        {"JP NZ, {d16}",    12, 2,   (OpArg) IS_NZ, &fetched,       &CPU::JP},
        {"JP {d16}",        12, 2,   (OpArg) NONE,  &fetched,       &CPU::JP},
        {"CALL NZ, {d16}",  12, 2,   (OpArg) IS_NZ, &fetched,       &CPU::CALL},
        {"PUSH BC",         16, 0,   &bc,           0,              &CPU::PUSH},
        {"ADD A, {d8}",     8,  1,   &a,            &fetched,       &CPU::ADD_REG_8_VAL_8},
        {"RST 0x00",        16, 0,   (OpArg) NONE,  0x00,           &CPU::RST},
        {"RET Z",           8,  0,   (OpArg) IS_Z,  0,              &CPU::RET},
        {"RET",             4,  0,   (OpArg) NONE,  0,              &CPU::RET},
        {"JP Z, {d16}",     12, 2,   (OpArg) IS_Z,  &fetched,       &CPU::JP},
        {"PREFIX CB",       0,  1,   &fetched,      0,              &CPU::PREFIX_CB},
        {"CALL Z, {d16}",   12, 2,   (OpArg) IS_Z,  &fetched,       &CPU::CALL},
        {"CALL {d16}",      12, 2,   (OpArg) NONE,  &fetched,       &CPU::CALL},
        {"ADC A, {d8}",     8,  1,   &a,            &fetched,       &CPU::ADC_REG_8_VAL_8},
        {"RST 0x08",        16, 0,   (OpArg) NONE,  (OpArg) 0x08,   &CPU::RST},
        // 0xD0
        {"RET NC",          8,  0,   (OpArg) IS_NC, 0,              &CPU::RET},
        {"POP DE",          12, 0,   &de,           0,              &CPU::POP},
        {"JP NC, {d16}",    12, 2,   (OpArg) IS_NC, &fetched,       &CPU::JP},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"CALL NC, {d16}",  12, 2,   (OpArg) IS_NC, &fetched,       &CPU::CALL},
        {"PUSH DE",         16, 0,   &de,           0,              &CPU::PUSH},
        {"SUB {d8}",        8,  1,   &a,            &fetched,       &CPU::SUB_REG_8_VAL_8},
        {"RST 0x10",        16, 0,   (OpArg) NONE,  (OpArg) 0x10,   &CPU::RST},
        {"RET C",           8,  0,   (OpArg) IS_C,  0,              &CPU::RET},
        {"RETI",            16, 0,   (OpArg) NONE,  0,              &CPU::RETI},
        {"JP C, {d16}",     12, 2,   (OpArg) IS_C,  &fetched,       &CPU::JP},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"CALL C, {d16}",   12, 2,   (OpArg) IS_C,  &fetched,       &CPU::CALL},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"SBC A, {d8}",     8,  1,   &a,            &fetched,       &CPU::SBC_REG_8_VAL_8},
        {"RST 0x18",        16, 0,   (OpArg) NONE,  (OpArg) 0x18,   &CPU::RST},
        // 0xE0
        {"LDH ({d8}), A",   12, 1,   &fetched,      &a,             &CPU::LDH_MEM_VAL_8},
        {"POP HL",          12, 0,   &hl,           0,              &CPU::POP},
        {"LD (C), A",       8,  0,   &c,            &a,             &CPU::LDH_MEM_VAL_8},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"PUSH HL",         16, 0,   &hl,           0,              &CPU::PUSH},
        {"AND {d8}",        8,  1,   &a,            &fetched,       &CPU::AND_REG_8_VAL_8},
        {"RST 0x20",        16, 0,   (OpArg) NONE,  (OpArg) 0x20,   &CPU::RST},
        {"ADD SP, {r8}",    16, 1,   &sp,           &fetched,       &CPU::ADD_REG_16_VAL_8},
        {"JP (HL)",         4,  0,   &hl,           0,              &CPU::JP_MEM},
        {"LD ({d16}), A",   16, 2,   &fetched,      &a,             &CPU::LD_MEM_VAL_8},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"XOR {d8}",        8,  1,   &a,            &fetched,       &CPU::XOR_REG_8_VAL_8},
        {"RST 0x28",        16, 0,   (OpArg) NONE,  (OpArg) 0x28,   &CPU::RST},
        // 0xF0
        {"LDH A, ({d8})",   12, 1,   &a,            &fetched,       &CPU::LDH_REG_8_MEM},
        {"POP AF",          12, 0,   &af,           0,              &CPU::POP},
        {"LD A, (C)",       8,  0,   &a,            &c,             &CPU::LDH_REG_8_MEM},
        {"DI",              4,  0,   0,             0,              &CPU::DI},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"PUSH AF",         16, 0,   &af,           0,              &CPU::PUSH},
        {"OR {d8}",         8,  1,   &a,            &fetched,       &CPU::OR_REG_8_VAL_8},
        {"RST 0x30",        16, 0,   (OpArg) NONE,  (OpArg) 0x30,   &CPU::RST},
        {"LDHL SP, {r8}",   12, 1,   &sp,           &fetched,       &CPU::LDHL_REG_16_VAL_8},
        {"LD SP, HL",       8,  0,   &sp,           &hl,            &CPU::LD_REG_16_VAL_16},
        {"LD A, ({d16})",   16, 2,   &a,            &fetched,       &CPU::LD_REG_8_MEM},
        {"EI",              4,  0,   0,             0,              &CPU::EI},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"???",             0,  0,   0,             0,              &CPU::UNKNOWN},
        {"CP {d8}",         8,  1,   &a,            &fetched,       &CPU::CP_REG_8_VAL_8},
        {"RST 0x38",        16, 0,   (OpArg) NONE,  (OpArg) 0x38,   &CPU::RST}
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
    ime = false;
    af = 0x01B0;
    bc = 0x0013;
    de = 0x00D8;
    hl = 0x014D;
    sp = 0xFFFE;
    pc = 0x0100;
}


void CPU::clock() {
    // If there are no more cycles left to complete, then we can execute the next instruction
    if (cycles == 0) {
        // Check for interrupts and service
        if (ime && handleInterrupt()) {
            return;
        }

        // Fetch next instruction and increment pc
        uint8_t opcode = read(pc);
        INSTRUCTION instr = lookup[opcode];

        // Fetch data -- 0, 1, or 2 bytes
        if (instr.data_len >= 1) {
            fetched = read(pc + 1);
        }
        if (instr.data_len == 2) {
            fetched |= read(pc + 2) << 8;
        }

#ifdef LOGFILE
        print_log(opcode, instr);
#endif

        // Increment pc
        pc += 1 + instr.data_len;

        // Set arguments
        arg1 = instr.arg1;
        arg2 = instr.arg2;
        
        // Increment clock
        cycles += instr.base_clock;

        // Instructions will return the number of extra cycles necessary
        cycles += (this->*instr.operate)();
    }

    cycles--;
}

#ifdef LOGFILE
void CPU::print_log(uint8_t opcode, INSTRUCTION instr) {
    //file << std::hex << std::showbase;
    //file << "Opcode: " << unsigned(opcode) << std::endl;
    //file << "Instruction: " << instr.name << ", " <<
    //        "Fetched: " << unsigned(fetched) << std::endl;
    //file << "REGISTER STATES PRIOR TO EXECUTION: " << std::endl <<
    //        "a: " << unsigned(a) << ", " << "f: " << unsigned(f) << std::endl <<
    //        "b: " << unsigned(b) << ", " << "c: " << unsigned(c) << std::endl <<
    //        "d: " << unsigned(d) << ", " << "e: " << unsigned(e) << std::endl <<
    //        "h: " << unsigned(h) << ", " << "l: " << unsigned(l) << std::endl <<
    //        "sp: " << unsigned(sp) << std::endl <<
    //        "pc: " << unsigned(pc) << std::endl;
    //file << "FLAGS: Z N H C" << std::endl << "       ";
    //file << std::dec << std::noshowbase;
    //file << getFlag(Z) << " " << getFlag(N) << " " << getFlag(H) <<  " " << getFlag(C) << std::endl;
    //file << std::endl;
    file << std::hex << std::uppercase;
    file << "A: " << std::setfill('0') << std::setw(2) << unsigned(a) << " "; 
    file << "F: " << std::setfill('0') << std::setw(2) << unsigned(f) << " ";
    file << "B: " << std::setfill('0') << std::setw(2) << unsigned(b) << " ";
    file << "C: " << std::setfill('0') << std::setw(2) << unsigned(c) << " ";
    file << "D: " << std::setfill('0') << std::setw(2) << unsigned(d) << " ";
    file << "E: " << std::setfill('0') << std::setw(2) << unsigned(e) << " ";
    file << "H: " << std::setfill('0') << std::setw(2) << unsigned(h) << " ";
    file << "L: " << std::setfill('0') << std::setw(2) << unsigned(l) << " ";
    file << "SP: " << std::setfill('0') << std::setw(4) << unsigned(sp) << " ";
    file << "PC: " << "00:" << std::setfill('0') << std::setw(4) << unsigned(pc) << " ";
    file << "(";
    file << std::setfill('0') << std::setw(2) << unsigned(read(pc));
    file << " " << std::setfill('0') << std::setw(2) << unsigned(read(pc+1));
    file << " " << std::setfill('0') << std::setw(2) << unsigned(read(pc+2));
    file << " " << std::setfill('0') << std::setw(2) << unsigned(read(pc+3));
    file << ")";
    file << std::endl;
    
    return;
}
#endif

// Flag operations
void CPU::setFlag(FLAG flag, bool val) {
    if (val) {
        f |= flag;
    } else {
        f &= ~flag;
    }
}

bool CPU::getFlag(FLAG flag) {
    return f & flag;
}

bool CPU::checkCond(COND cond) {
    switch (cond) {
        case NONE:
            return true;
        case IS_Z:
            return getFlag(Z);
        case IS_NZ:
            return !getFlag(Z);
        case IS_C:
            return getFlag(C);
        case IS_NC:
            return !getFlag(C);
    }
}

bool CPU::handleInterrupt() {
    uint8_t intr_flags = read(IF);

    // Interrupts that are both enabled and requested
    uint8_t intrs = intr_flags & read(IE);
    uint16_t jump_addr = 0x0000;

    if (intrs & V_BLANK) {
        write(IF, intr_flags & ~V_BLANK);
        jump_addr = VBLANK_A;
    } else if (intrs & LCD_STAT) {
        write(IF, intr_flags & ~LCD_STAT);
        jump_addr = LCDS_A;
    } else if (intrs & TIMER) {
        write(IF, intr_flags & ~TIMER);
        jump_addr = TIMER_A;
    } else if (intrs & SERIAL) {
        write(IF, intr_flags & ~SERIAL);
        jump_addr = SERIAL_A;
    } else if (intrs & JOYPAD) {
        write(IF, intr_flags & ~JOYPAD);
        jump_addr = JOYPAD_A;
    }

    // Check if jump_addr was not adjusted -> no interrupts need to occur
    if (jump_addr != 0x0000) {
        ime = false; // So that interrupts aren't interrupted

        // Set up a call to CALL instruction for convenience
        fetched = jump_addr;
        arg1 = (OpArg) NONE;
        arg2 = &fetched;
        CALL();

        return true;
    }

    return false;
}

// OPCODE IMPLEMENTATIONS
// Many of these use arg1 and arg2 in different ways or not at all (eg. NOP)
// Most commonly arg1 and arg2 will be the memory address of a register or immediate value (fetched)

uint8_t CPU::PREFIX_CB() {
    return 0;
}

uint8_t CPU::UNKNOWN() {
    throw std::invalid_argument("Invalid Opcode.");
}

uint8_t CPU::NOP() {
    return 0;
}

// TODO: Implement -- should halt processor until a button is pressed
uint8_t CPU::STOP() {
    return 0;
}

// TODO: Implement -- should halt processor until an interrupt is initiated
uint8_t CPU::HALT() {
    return 0;
}

// TODO: Implement -- this one is pretty nasty. need to implement N and C flags for this
uint8_t CPU::DA() {
    return 0;
}

uint8_t CPU::EI() {
    ime = true;
    return 0;
}

uint8_t CPU::DI() {
    ime = false;
    return 0;
}

// For LD instructions, arg1 = destination, arg2 = source
// LD_XXX_XXX refers to how first and second arg will be treated respectively
// i.e. REG or VAL will simply be dereferenced, MEM will be dereferenced and used to read from memory
uint8_t CPU::LD_REG_16_VAL_16() {
    DR_16(arg1) = DR_16(arg2);
    return 0;
}

uint8_t CPU::LD_REG_8_VAL_8() {
    DR_8(arg1) = DR_8(arg2);
    return 0;
}

uint8_t CPU::LD_MEM_VAL_16() {
    uint16_t data = DR_16(arg2);
    write(DR_16(arg1) + 0, (uint8_t) (data >> 0));
    write(DR_16(arg1) + 1, (uint8_t) (data >> 8));
    return 0;
}

uint8_t CPU::LD_MEM_VAL_8() {
    write(DR_16(arg1), DR_8(arg2));
    return 0;
}

uint8_t CPU::LD_REG_8_MEM() {
    DR_8(arg1) = read(DR_16(arg2));
    return 0;
}

uint8_t CPU::LDH_MEM_VAL_8() {
    fetched = DR_8(arg1) + 0xFF00;
    arg1 = &fetched;
    LD_MEM_VAL_8();
    return 0;
}

uint8_t CPU::LDH_REG_8_MEM() {
    fetched = DR_8(arg2) + 0xFF00;
    arg2 = &fetched;
    LD_REG_8_MEM();
    return 0;
}

// arg2 is treated as a signed value for this instruction
// The two arguments are summed and the resulting value is loaded into HL
uint8_t CPU::LDHL_REG_16_VAL_8() {
    // So that the register argument isn't altered
    uint16_t temp = DR_16(arg1);
    arg1 = &temp;

    // This instruction is perfect as it treats arg2 as signed and sets
    // flags exactly how we need
    ADD_REG_16_VAL_8();

    // switch around args for LD call
    arg2 = arg1;
    arg1 = &hl;
    LD_REG_16_VAL_16();
    return 0;
}

// For INC and DEC instructions, arg1 = location, arg2 is unused
uint8_t CPU::INC_REG_16() {
    DR_16(arg1)++;
    return 0;
}

uint8_t CPU::INC_REG_8() {
    bool half_carry = (DR_8(arg1) & 0x0F) == 0x0F;
    DR_8(arg1)++;

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 0);
    setFlag(H, half_carry);
    return 0;
}

uint8_t CPU::INC_MEM() {
    uint8_t val = read(DR_16(arg1));
    bool half_carry = (val & 0x0F) == 0x0F;
    write(DR_16(arg1), val + 1);

    setFlag(Z, val == 0);
    setFlag(N, 0);
    setFlag(H, half_carry);
    return 0;
}

uint8_t CPU::DEC_REG_16() {
    DR_16(arg1)--;
    return 0;
}

uint8_t CPU::DEC_REG_8() {
    bool half_carry = (DR_8(arg1) & 0x0F) == 0x00;
    DR_8(arg1)--;

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, half_carry); // set if borrow from bit 4
    return 0;
}

uint8_t CPU::DEC_MEM() {
    uint8_t val = read(DR_16(arg1));
    bool half_carry = (val & 0x0F) == 0x00;
    write(DR_16(arg1), val - 1);

    setFlag(Z, val == 0);
    setFlag(N, 1);
    setFlag(H, half_carry); // set if borrow from bit 4
    return 0;
}

// LD followed by INC/DEC and implemented as such
uint8_t CPU::LDI_MEM_VAL_8() {
    LD_MEM_VAL_8();
    // MEM is always a register whose address is used to index memory. 
    // We increment this register
    INC_REG_16();
    return 0;
}

uint8_t CPU::LDI_REG_8_MEM() {
    LD_REG_8_MEM();
    // MEM is always a register whose address is used to index memory. 
    // We increment this register
    arg1 = arg2;
    INC_REG_16();
    return 0;
}

uint8_t CPU::LDD_MEM_VAL_8() {
    LD_MEM_VAL_8();
    // MEM is always a register whose address is used to index memory. 
    // We decrement this register
    DEC_REG_16();
    return 0;
}

uint8_t CPU::LDD_REG_8_MEM() {
    LD_REG_8_MEM();
    // MEM is always a register whose address is used to index memory. 
    // We decrement this register
    arg1 = arg2;
    DEC_REG_16();
    return 0;
}

// For ADD instructions, arg1 is the destination, arg2 is the source value
uint8_t CPU::ADD_REG_16_VAL_16() {
    bool half_carry = (DR_16(arg1) & 0x0F) + (DR_16(arg2) & 0x0F) > 0x0F;
    uint32_t sum = DR_16(arg1) + DR_16(arg2);
    DR_16(arg1) = (uint16_t) sum;

    setFlag(N, 0);
    setFlag(H, half_carry);
    setFlag(C, sum >= 1 << 16);
    return 0;
}

// For this instruction, arg2 is a signed value
uint8_t CPU::ADD_REG_16_VAL_8() {
    bool carry, half_carry;
    uint8_t val = DR_8(arg2) & 0x7F;

    // check sign
    if (DR_8(arg2) >> 7) {
        carry = DR_16(arg1) < val;
        half_carry = (DR_16(arg1) & 0x0F) < (val & 0x0F);
        DR_16(arg1) -= val;
    } else {
        half_carry = (DR_16(arg1) & 0x0F) + (val & 0x0F) > 0x0F;
        uint32_t sum = DR_16(arg1) + val;
        carry = sum >= 1 << 16;
        DR_16(arg1) = (uint16_t) sum;
    }

    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, half_carry);
    setFlag(C, carry);
    return 0;
}

uint8_t CPU::ADD_REG_8_VAL_8 () {
    bool half_carry = (DR_8(arg1) & 0x0F) + (DR_8(arg2) & 0x0F) > 0x0F;
    uint16_t sum = DR_8(arg1) + DR_8(arg2);
    DR_8(arg1) = (uint8_t) sum;

    setFlag(Z, (uint8_t) sum == 0);
    setFlag(N, 0);
    setFlag(H, half_carry);
    setFlag(C, sum >= 1 << 8);
    return 0;
}

uint8_t CPU::ADD_REG_8_MEM () {
    uint8_t val = read(DR_16(arg2));
    bool half_carry = (DR_8(arg1) & 0x0F) + (val & 0x0F) > 0x0F;
    uint16_t sum = DR_8(arg1) + val;
    *(uint8_t *) arg1 = (uint8_t) sum;

    setFlag(Z, (uint8_t) sum == 0);
    setFlag(N, 0);
    setFlag(H, half_carry);
    setFlag(C, sum >= 1 << 8);
    return 0;
}

uint8_t CPU::ADC_REG_8_VAL_8() {
    uint8_t c = (f & C) >> 4;
    bool half_carry = (DR_8(arg1) & 0x0F) + (DR_8(arg2) & 0x0F) + c > 0x0F;
    uint16_t sum = DR_8(arg1) + DR_8(arg2) + c;
    DR_8(arg1) = (uint8_t) sum;

    setFlag(Z, (uint8_t) sum == 0);
    setFlag(N, 0);
    setFlag(H, half_carry);
    setFlag(C, sum >= 1 << 8);
    return 0;
}

uint8_t CPU::ADC_REG_8_MEM() {
    uint8_t c = (f & C) >> 4;
    uint8_t val = read(DR_16(arg2));
    bool half_carry = (DR_8(arg1) & 0x0F) + (val & 0x0F) + c > 0x0F;
    uint16_t sum = DR_8(arg1) + val + c;
    *(uint8_t *) arg1 = (uint8_t) sum;

    setFlag(Z, (uint8_t) sum == 0);
    setFlag(N, 0);
    setFlag(H, half_carry);
    setFlag(C, sum >= 1 << 8);
    return 0;
}

// For SUB instructions, arg1 is always A, but we take it as an arg for consistency
// arg2 is the value we subtract with
uint8_t CPU::SUB_REG_8_VAL_8() {
    uint8_t val1 = DR_8(arg1);
    uint8_t val2 = DR_8(arg2);
    bool half_carry = (val1 & 0x0F) < (val2 & 0x0F);
    DR_8(arg1) = val1 - val2;

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, half_carry);
    setFlag(C, val1 < val2);
    return 0;
}

uint8_t CPU::SUB_REG_8_MEM() {
    uint8_t val1 = DR_8(arg1);
    uint8_t val2 = read(DR_16(arg2));
    bool half_carry = (val1 & 0x0F) < (val2 & 0x0F);
    DR_8(arg1) = val1 - val2;

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, half_carry);
    setFlag(C, val1 < val2);
    return 0;
}

uint8_t CPU::SBC_REG_8_VAL_8() {
    uint8_t c = (f & C) >> 4;
    uint8_t val1 = DR_8(arg1);
    uint8_t val2 = DR_8(arg2);
    bool half_carry = (val1 & 0x0F) < (val2 & 0x0F + c);
    DR_8(arg1) = val1 - (val2 + c);

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, half_carry);
    setFlag(C, val1 < val2 + c);
    return 0;
}

uint8_t CPU::SBC_REG_8_MEM() {
    uint8_t c = (f & C) >> 4;
    uint8_t val1 = DR_8(arg1);
    uint8_t val2 = read(DR_16(arg2));
    bool half_carry = (val1 & 0x0F) < (val2 & 0x0F + c);
    DR_8(arg1) = val1 - (val2 + c);

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, half_carry);
    setFlag(C, val1 < val2 + c);
    return 0;
}

// AND, XOR, OR take two registers as OpArgs
uint8_t CPU::AND_REG_8_VAL_8(){
    DR_8(arg1) &= DR_8(arg2);

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, 1);
    setFlag(C, 0);
    return 0;
}

uint8_t CPU::AND_REG_8_MEM() {
    DR_8(arg1) &= read(DR_8(arg2));

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 0);
    setFlag(H, 1);
    setFlag(C, 0);
    return 0;
}

uint8_t CPU::XOR_REG_8_VAL_8() {
    DR_8(arg1) ^= DR_8(arg2);

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

uint8_t CPU::XOR_REG_8_MEM() {
    DR_8(arg1) ^= read(DR_16(arg2));

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

uint8_t CPU::OR_REG_8_VAL_8() {
    DR_8(arg1) |= DR_8(arg2);

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

uint8_t CPU::OR_REG_8_MEM() {
    DR_8(arg1) |= read(DR_16(arg2));

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, 0);
    return 0;
}

uint8_t CPU::CP_REG_8_VAL_8() {
    uint8_t val1 = DR_8(arg1);
    uint8_t val2 = DR_8(arg2);
    bool half_carry = (val1 & 0x0F) < (val2 & 0x0F);

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, half_carry);
    setFlag(C, val1 < val2);
    return 0;
}

uint8_t CPU::CP_REG_8_MEM() {
    uint8_t val1 = DR_8(arg1);
    uint8_t val2 = read(DR_16(arg2));
    bool half_carry = (val1 & 0x0F) < (val2 & 0x0F);

    setFlag(Z, DR_8(arg1) == 0);
    setFlag(N, 1);
    setFlag(H, half_carry);
    setFlag(C, val1 < val2);
    return 0;
}

// CPL complements a register. arg1 is the register address. arg2 is always 0.
uint8_t CPU::CPL() {
    DR_8(arg1) = ~DR_8(arg1);

    setFlag(N, 1);
    setFlag(H, 1);
    return 0;
}

// SCF sets carry flag. This method takes register as arg1 and flag as arg2 by convention
uint8_t CPU::SCF() {
    setFlag(C, 1);
    return 0;
}

// CCF complements carry flag. Takes reg as arg1 and flag as arg2 by convention
uint8_t CPU::CCF() {
    setFlag(C, !getFlag(C));
    return 0;
}

// Rotate instructions for reg A. Takes reg as arg by convention
// RLA and RRA rotate through the carry bit (bit 7 -> C -> bit 0 or opposite)
// RLCA and RRCA rotate the integer and copy the bit that switched sides to the carry bit
uint8_t CPU::RLA() {
    uint8_t carry = DR_8(arg1) >> 7;
    DR_8(arg1) <<= 1;
    DR_8(arg1) |= getFlag(C);

    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, carry);
    return 0;
}

uint8_t CPU::RRA() {
    uint8_t carry = DR_8(arg1) & 0x01;
    DR_8(arg1) >>= 1;
    DR_8(arg1) |= getFlag(C) << 7;

    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, carry);
    return 0;
}

uint8_t CPU::RLCA() {
    uint8_t carry = DR_8(arg1) >> 7;
    DR_8(arg1) <<= 1;
    DR_8(arg1) |= carry;

    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, carry);
    return 0;
}

uint8_t CPU::RRCA() {
    uint8_t carry = DR_8(arg1) & 0x01;
    DR_8(arg1) >>= 1;
    DR_8(arg1) |= carry;

    setFlag(Z, 0);
    setFlag(N, 0);
    setFlag(H, 0);
    setFlag(C, carry);
    return 0;
}

// arg1 is the condition upon which we jump; arg2 is the relative address displacement
uint8_t CPU::JR() {
    if (checkCond((COND) (uint64_t) arg1)) {
        pc += *(int8_t *) arg2;
        return 4;
    }

    return 0;
}

uint8_t CPU::JP() {
    if (checkCond((COND) (uint64_t) arg1)) {
        pc = DR_16(arg2);
        return 4;
    }

    return 0;
}

uint8_t CPU::JP_MEM() {
    pc = read(DR_16(arg1));
    return 0;
}

// arg1 is the register we're pushing to the stack
uint8_t CPU::PUSH() {
    sp -= 2;
    write(sp + 0, (uint8_t) (DR_16(arg1) >> 0));
    write(sp + 1, (uint8_t) (DR_16(arg1) >> 8));

    return 0;
}

// arg1 is the register we're popping the value to
uint8_t CPU::POP() {
    DR_16(arg1) = read(sp);
    DR_16(arg1) |= read(sp + 1) << 8;
    sp += 2;

    return 0;
}

// arg1 is a condition; arg2 is the address we're jumping to
uint8_t CPU::CALL() {
    if (checkCond((COND) (uint64_t) arg1)) {
        // Uses PUSH to push program counter to stack
        arg1 = &pc;
        PUSH();
        pc = DR_16(arg2);
        return 12;
    }

    return 0;
}

// arg1 is a condition
uint8_t CPU::RET() {
    if (checkCond((COND) (uint64_t) arg1)) {
        arg1 = &pc;
        POP();
        return 12;
    }

    return 0;
}

uint8_t CPU::RETI() {
    ime = true;
    RET();
    return 0;
}

// arg1 is the reset address (constant, not a variable's address)
uint8_t CPU::RST() {
    fetched = (uint16_t) (uint64_t) arg2;
    arg2 = &fetched;
    CALL();
    return 0;
}