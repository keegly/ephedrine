#pragma once

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

enum class AddressingMode {
  kNone = 0,
  kImmediate,  // one byte immediately following
  kDirect,     // two bytes immediately following
  kIndirect
};

struct DecodedInstruction {
  uint8_t opcode;
  std::string name;
  AddressingMode mode;
  // source?
  // dest?
  int length;
  int cycles;
  std::optional<uint16_t> operand;
  // uint16_t operand;
  // void (*fp)(void);
};

class Instructions {
  inline static DecodedInstruction instructions_[]{
      {0x00, "NOP", AddressingMode::kNone, 1, 4},
      {0x01, "LD BC, d16", AddressingMode::kDirect, 3, 12},
      {0x02, "LD (BC), A", AddressingMode::kNone, 1, 8},
      {0x03, "INC BC", AddressingMode::kNone, 1, 8},
      {0x04, "INC B", AddressingMode::kNone, 1, 4},
      {0x05, "DEC B", AddressingMode::kNone, 1, 5},
      {0x06, "LD B, d8", AddressingMode::kImmediate, 2, 8},
      {0x07, "RLCA", AddressingMode::kNone, 1, 4},
      {0x08, "LD (a16), SP", AddressingMode::kDirect, 3, 20},
      {0x09, "ADD HL, BC", AddressingMode::kNone, 1, 8},
      {0x0A, "LD A, (BC)", AddressingMode::kNone, 1, 8},
      {0x0B, "DEC BC", AddressingMode::kNone, 1, 8},
      {0x0C, "INC C", AddressingMode::kNone, 1, 4},
      {0x0D, "DEC C", AddressingMode::kNone, 1, 4},
      {0x0E, "LD C, d8", AddressingMode::kImmediate, 2, 8},
      {0x0F, "RRCA", AddressingMode::kNone, 1, 4},
      {0x10, "STOP", AddressingMode::kImmediate, 2, 4},
      {0x11, "LD DE, d16", AddressingMode::kDirect, 3, 12},
      {0x12, "LD (DE), A", AddressingMode::kNone, 1, 8},
      {0x13, "INC DE", AddressingMode::kNone, 1, 8},
      {0x14, "INC D", AddressingMode::kNone, 1, 4},
      {0x15, "DEC D", AddressingMode::kNone, 1, 4},
      {0x16, "LD D, d8", AddressingMode::kImmediate, 2, 8},
      {0x17, "RLA", AddressingMode::kNone, 1, 4},
      {0x18, "JR r8", AddressingMode::kImmediate, 2, 12},
      {0x19, "ADD HL, DE", AddressingMode::kNone, 1, 8},
      {0x1A, "LD A, (DE)", AddressingMode::kNone, 1, 8},
      {0x1B, "DEC DE", AddressingMode::kNone, 1, 8},
      {0x1C, "INC E", AddressingMode::kNone, 1, 4},
      {0x1D, "DEC E", AddressingMode::kNone, 1, 4},
      {0x1E, "LD E, d8", AddressingMode::kImmediate, 2, 8},
      {0x1F, "RRA", AddressingMode::kNone, 1, 4},
      {0x20, "JR NZ, r8", AddressingMode::kImmediate, 2,
       8},  // 12 cycles if jump taken
      {0x21, "LD HL, d16", AddressingMode::kDirect, 3, 12},
      {0x22, "LD (HL+), A", AddressingMode::kNone, 1, 8},
      {0x23, "INC HL", AddressingMode::kNone, 1, 8},
      {0x24, "INC H", AddressingMode::kNone, 1, 4},
      {0x25, "DEC H", AddressingMode::kNone, 1, 4},
      {0x26, "LD H, d8", AddressingMode::kImmediate, 2, 8},
      {0x27, "DAA", AddressingMode::kNone, 1, 4},
      {0x28, "JR Z, r8", AddressingMode::kImmediate, 2,
       8},  // 12 cycles if jump taken
      {0x29, "ADD HL, HL", AddressingMode::kNone, 1, 8},
      {0x2A, "LD A, (HL+)", AddressingMode::kNone, 1, 8},
      {0x2B, "DEC HL", AddressingMode::kNone, 1, 8},
      {0x2C, "INC L", AddressingMode::kNone, 1, 4},
      {0x2D, "DEC L", AddressingMode::kNone, 1, 4},
      {0x2E, "LD L, d8", AddressingMode::kImmediate, 2, 8},
      {0x2F, "CPL", AddressingMode::kNone, 1, 4},
      {0x30, "JR NC, r8", AddressingMode::kImmediate, 2,
       8},  // 12 cycles if jump taken
      {0x31, "LD SP, d16", AddressingMode::kDirect, 3, 12},
      {0x32, "LD (HL-), A", AddressingMode::kNone, 1, 8},
      {0x33, "INC SP", AddressingMode::kNone, 1, 8},
      {0x34, "INC (HL)", AddressingMode::kNone, 1, 12},
      {0x35, "DEC (HL)", AddressingMode::kNone, 1, 12},
      {0x36, "LD (HL), d8", AddressingMode::kImmediate, 2, 12},
      {0x37, "SCF", AddressingMode::kNone, 1, 4},
      {0x38, "JR C, r8", AddressingMode::kImmediate, 2,
       8},  // 12 cycles if jump taken
      {0x39, "ADD HL, SP", AddressingMode::kNone, 1, 8},
      {0x3A, "LD A, (HL-)", AddressingMode::kNone, 1, 8},
      {0x3B, "DEC SP", AddressingMode::kNone, 1, 8},
      {0x3C, "INC A", AddressingMode::kNone, 1, 4},
      {0x3D, "DEC A", AddressingMode::kNone, 1, 4},
      {0x3E, "LD A, d8", AddressingMode::kImmediate, 2, 8},
      {0x3F, "CCF", AddressingMode::kNone, 1, 4},
      {0x40, "LD B, B", AddressingMode::kNone, 1, 4},
      {0x41, "LD B, C", AddressingMode::kNone, 1, 4},
      {0x42, "LD B, D", AddressingMode::kNone, 1, 4},
      {0x43, "LD B, E", AddressingMode::kNone, 1, 4},
      {0x44, "LD B, H", AddressingMode::kNone, 1, 4},
      {0x45, "LD B, L", AddressingMode::kNone, 1, 4},
      {0x46, "LD B, (HL)", AddressingMode::kNone, 1, 8},
      {0x47, "LD B, A", AddressingMode::kNone, 1, 4},
      {0x48, "LD C, B", AddressingMode::kNone, 1, 4},
      {0x49, "LD C, C", AddressingMode::kNone, 1, 4},
      {0x4A, "LD C, D", AddressingMode::kNone, 1, 4},
      {0x4B, "LD C, E", AddressingMode::kNone, 1, 4},
      {0x4C, "LD C, H", AddressingMode::kNone, 1, 4},
      {0x4D, "LD C, L", AddressingMode::kNone, 1, 4},
      {0x4E, "LD C, (HL)", AddressingMode::kNone, 1, 8},
      {0x4F, "LD C, A", AddressingMode::kNone, 1, 4},
      {0x50, "LD D, B", AddressingMode::kNone, 1, 4},
      {0x51, "LD D, C", AddressingMode::kNone, 1, 4},
      {0x52, "LD D, D", AddressingMode::kNone, 1, 4},
      {0x53, "LD D, E", AddressingMode::kNone, 1, 4},
      {0x54, "LD D, H", AddressingMode::kNone, 1, 4},
      {0x55, "LD D, L", AddressingMode::kNone, 1, 4},
      {0x56, "LD D, (HL)", AddressingMode::kNone, 1, 8},
      {0x57, "LD D, A", AddressingMode::kNone, 1, 4},
      {0x58, "LD E, B", AddressingMode::kNone, 1, 4},
      {0x59, "LD E, C", AddressingMode::kNone, 1, 4},
      {0x5A, "LD E, D", AddressingMode::kNone, 1, 4},
      {0x5B, "LD E, E", AddressingMode::kNone, 1, 4},
      {0x5C, "LD E, H", AddressingMode::kNone, 1, 4},
      {0x5D, "LD E, L", AddressingMode::kNone, 1, 4},
      {0x5E, "LD E, (HL)", AddressingMode::kNone, 1, 8},
      {0x5F, "LD E, A", AddressingMode::kNone, 1, 4},
      {0x60, "LD H, B", AddressingMode::kNone, 1, 4},
      {0x61, "LD H, C", AddressingMode::kNone, 1, 4},
      {0x62, "LD H, D", AddressingMode::kNone, 1, 4},
      {0x63, "LD H, E", AddressingMode::kNone, 1, 4},
      {0x64, "LD H, H", AddressingMode::kNone, 1, 4},
      {0x65, "LD H, L", AddressingMode::kNone, 1, 4},
      {0x66, "LD H, (HL)", AddressingMode::kNone, 1, 8},
      {0x67, "LD H, A", AddressingMode::kNone, 1, 4},
      {0x68, "LD L, B", AddressingMode::kNone, 1, 4},
      {0x69, "LD L, C", AddressingMode::kNone, 1, 4},
      {0x6A, "LD L, D", AddressingMode::kNone, 1, 4},
      {0x6B, "LD L, E", AddressingMode::kNone, 1, 4},
      {0x6C, "LD L, H", AddressingMode::kNone, 1, 4},
      {0x6D, "LD L, L", AddressingMode::kNone, 1, 4},
      {0x6E, "LD L, (HL)", AddressingMode::kNone, 1, 8},
      {0x6F, "LD L, A", AddressingMode::kNone, 1, 4},
      {0x70, "LD (HL), B", AddressingMode::kNone, 1, 8},
      {0x71, "LD (HL), C", AddressingMode::kNone, 1, 8},
      {0x72, "LD (HL), D", AddressingMode::kNone, 1, 8},
      {0x73, "LD (HL), E", AddressingMode::kNone, 1, 8},
      {0x74, "LD (HL), H", AddressingMode::kNone, 1, 8},
      {0x75, "LD (HL), L", AddressingMode::kNone, 1, 8},
      {0x76, "HALT", AddressingMode::kNone, 1, 4},
      {0x77, "LD (HL), A", AddressingMode::kNone, 1, 8},
      {0x78, "LD A, B", AddressingMode::kNone, 1, 4},
      {0x79, "LD A, C", AddressingMode::kNone, 1, 4},
      {0x7A, "LD A, D", AddressingMode::kNone, 1, 4},
      {0x7B, "LD A, E", AddressingMode::kNone, 1, 4},
      {0x7C, "LD A, H", AddressingMode::kNone, 1, 4},
      {0x7D, "LD A, L", AddressingMode::kNone, 1, 4},
      {0x7E, "LD A, (HL)", AddressingMode::kNone, 1, 8},
      {0x7F, "LD A, A", AddressingMode::kNone, 1, 4},
      {0x80, "ADD A, B", AddressingMode::kNone, 1, 4},
      {0x81, "ADD A, C", AddressingMode::kNone, 1, 4},
      {0x82, "ADD A, D", AddressingMode::kNone, 1, 4},
      {0x83, "ADD A, E", AddressingMode::kNone, 1, 4},
      {0x84, "ADD A, H", AddressingMode::kNone, 1, 4},
      {0x85, "ADD A, L", AddressingMode::kNone, 1, 4},
      {0x86, "ADD A, (HL)", AddressingMode::kNone, 1, 8},
      {0x87, "ADD A, A", AddressingMode::kNone, 1, 4},
      {0x88, "ADC A, B", AddressingMode::kNone, 1, 4},
      {0x89, "ADC A, C", AddressingMode::kNone, 1, 4},
      {0x8A, "ADC A, D", AddressingMode::kNone, 1, 4},
      {0x8B, "ADC A, E", AddressingMode::kNone, 1, 4},
      {0x8C, "ADC A, H", AddressingMode::kNone, 1, 4},
      {0x8D, "ADC A, L", AddressingMode::kNone, 1, 4},
      {0x8E, "ADC A, (HL)", AddressingMode::kNone, 1, 8},
      {0x8F, "ADC A, A", AddressingMode::kNone, 1, 4},
      {0x90, "SUB B", AddressingMode::kNone, 1, 4},
      {0x91, "SUB C", AddressingMode::kNone, 1, 4},
      {0x92, "SUB D", AddressingMode::kNone, 1, 4},
      {0x93, "SUB E", AddressingMode::kNone, 1, 4},
      {0x94, "SUB H", AddressingMode::kNone, 1, 4},
      {0x95, "SUB L", AddressingMode::kNone, 1, 4},
      {0x96, "SUB (HL)", AddressingMode::kNone, 1, 8},
      {0x97, "SUB A", AddressingMode::kNone, 1, 4},
      {0x98, "SBC A, B", AddressingMode::kNone, 1, 4},
      {0x99, "SBC A, C", AddressingMode::kNone, 1, 4},
      {0x9A, "SBC A, D", AddressingMode::kNone, 1, 4},
      {0x9B, "SBC A, E", AddressingMode::kNone, 1, 4},
      {0x9C, "SBC A, H", AddressingMode::kNone, 1, 4},
      {0x9D, "SBC A, L", AddressingMode::kNone, 1, 4},
      {0x9E, "SBC A, (HL)", AddressingMode::kNone, 1, 8},
      {0x9F, "SBC A, A", AddressingMode::kNone, 1, 4},
      {0xA0, "AND B", AddressingMode::kNone, 1, 4},
      {0xA1, "AND C", AddressingMode::kNone, 1, 4},
      {0xA2, "AND D", AddressingMode::kNone, 1, 4},
      {0xA3, "AND E", AddressingMode::kNone, 1, 4},
      {0xA4, "AND H", AddressingMode::kNone, 1, 4},
      {0xA5, "AND L", AddressingMode::kNone, 1, 4},
      {0xA6, "AND (HL)", AddressingMode::kNone, 1, 8},
      {0xA7, "AND A", AddressingMode::kNone, 1, 4},
      {0xA8, "XOR B", AddressingMode::kNone, 1, 4},
      {0xA9, "XOR C", AddressingMode::kNone, 1, 4},
      {0xAA, "XOR D", AddressingMode::kNone, 1, 4},
      {0xAB, "XOR E", AddressingMode::kNone, 1, 4},
      {0xAC, "XOR H", AddressingMode::kNone, 1, 4},
      {0xAD, "XOR L", AddressingMode::kNone, 1, 4},
      {0xAE, "XOR (HL)", AddressingMode::kNone, 1, 8},
      {0xAF, "XOR A", AddressingMode::kNone, 1, 4},
      {0xB0, "OR B", AddressingMode::kNone, 1, 4},
      {0xB1, "OR C", AddressingMode::kNone, 1, 4},
      {0xB2, "OR D", AddressingMode::kNone, 1, 4},
      {0xB3, "OR E", AddressingMode::kNone, 1, 4},
      {0xB4, "OR H", AddressingMode::kNone, 1, 4},
      {0xB5, "OR L", AddressingMode::kNone, 1, 4},
      {0xB6, "OR (HL)", AddressingMode::kNone, 1, 8},
      {0xB7, "OR A", AddressingMode::kNone, 1, 4},
      {0xB8, "CP B", AddressingMode::kNone, 1, 4},
      {0xB9, "CP C", AddressingMode::kNone, 1, 4},
      {0xBA, "CP D", AddressingMode::kNone, 1, 4},
      {0xBB, "CP E", AddressingMode::kNone, 1, 4},
      {0xBC, "CP H", AddressingMode::kNone, 1, 4},
      {0xBD, "CP L", AddressingMode::kNone, 1, 4},
      {0xBE, "CP (HL)", AddressingMode::kNone, 1, 8},
      {0xBF, "CP A", AddressingMode::kNone, 1, 4},
      {0xC0, "RET NZ", AddressingMode::kNone, 1,
       8},  // 20 cycles if return taken
      {0xC1, "POP BC", AddressingMode::kNone, 1, 12},
      {0xC2, "JP NZ, a16", AddressingMode::kDirect, 3, 12},  // 16 if jp taken
      {0xC3, "JP a16", AddressingMode::kDirect, 3, 16},
      {0xC4, "CALL NZ, a16", AddressingMode::kDirect, 3,
       12},  // 24 if call taken
      {0xC5, "PUSH BC", AddressingMode::kNone, 1, 16},
      {0xC6, "ADD A, d8", AddressingMode::kImmediate, 2, 8},
      {0xC7, "RST 00h", AddressingMode::kNone, 1, 16},
      {0xC8, "RET Z", AddressingMode::kNone, 1, 8},  // 20 if return taken
      {0xC9, "RET", AddressingMode::kNone, 1, 16},
      {0xCA, "JP Z, a16", AddressingMode::kDirect, 3, 12},  // 16 if taken
      {0xCB, "PREFIX CB", AddressingMode::kNone, 1, 4},
      {0xCC, "CALL Z, a16", AddressingMode::kDirect, 3, 12},  // 24 if taken
      {0xCD, "CALL a16", AddressingMode::kDirect, 3, 24},
      {0xCE, "ADC A, d8", AddressingMode::kImmediate, 2, 8},
      {0xCF, "RST 08H", AddressingMode::kNone, 1, 16},
      {0xD0, "RET NC", AddressingMode::kNone, 1, 8},  // 20 if taken
      {0xD1, "POP DE", AddressingMode::kNone, 1, 12},
      {0xD2, "JP NC, a16", AddressingMode::kDirect, 3, 12},  // 16 if taken
      {0xD3, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xD4, "CALL NC, a16", AddressingMode::kDirect, 3, 12},  // 24 if taken
      {0xD5, "PUSH DE", AddressingMode::kNone, 1, 16},
      {0xD6, "SUB d8", AddressingMode::kImmediate, 2, 8},
      {0xD7, "RST 10H", AddressingMode::kNone, 1, 16},
      {0xD8, "RET C", AddressingMode::kNone, 1, 8},  // 20 if taken
      {0xD9, "RETI", AddressingMode::kNone, 1, 16},
      {0xDA, "JP C, a16", AddressingMode::kDirect, 3, 12},  // 16 if taken
      {0xDB, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xDC, "CALL C, a16", AddressingMode::kDirect, 3, 12},  // 24 if taken
      {0xDD, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xDE, "SBC A, d8", AddressingMode::kImmediate, 2, 8},
      {0xDF, "RST 18h", AddressingMode::kNone, 1, 16},
      {0xE0, "LDH (a8), A", AddressingMode::kIndirect, 2, 12},
      {0xE1, "POP HL", AddressingMode::kNone, 1, 12},
      {0xE2, "LD (c), A", AddressingMode::kIndirect, 2, 8},
      {0xE3, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xE4, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xE5, "PUSH HL", AddressingMode::kNone, 1, 12},
      {0xE6, "AND d8", AddressingMode::kImmediate, 2, 8},
      {0xE7, "RST 20h", AddressingMode::kNone, 1, 16},
      {0xE8, "ADD SP, r8", AddressingMode::kIndirect, 2, 16},
      {0xE9, "JP (HL)", AddressingMode::kNone, 1, 4},
      {0xEA, "LD (a16), A", AddressingMode::kDirect, 3, 16},
      {0xEB, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xEC, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xED, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xEE, "XOR d8", AddressingMode::kImmediate, 2, 8},
      {0xEF, "RST 28h", AddressingMode::kNone, 1, 16},
      {0xF0, "LDH A, (a8)", AddressingMode::kIndirect, 2, 12},
      {0xF1, "POP AF", AddressingMode::kNone, 1, 12},
      {0xF2, "LD A, (c)", AddressingMode::kIndirect, 2, 8},
      {0xF3, "DI", AddressingMode::kNone, 1, 4},
      {0xF4, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xF5, "PUSH AF", AddressingMode::kNone, 1, 12},
      {0xF6, "OR d8", AddressingMode::kImmediate, 2, 8},
      {0xF7, "RST 30h", AddressingMode::kNone, 1, 16},
      {0xF8, "LD HL, SP+r8", AddressingMode::kIndirect, 2, 12},
      {0xF9, "LD SP, HL", AddressingMode::kNone, 1, 8},
      {0xFA, "LD A, (a16)", AddressingMode::kDirect, 3, 16},
      {0xFB, "EI", AddressingMode::kNone, 1, 4},
      {0xFC, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xFD, "INVALID OPCODE", AddressingMode::kNone, 0, 0},
      {0xFE, "CP d8", AddressingMode::kImmediate, 2, 8},
      {0xFF, "RST 38h", AddressingMode::kNone, 1, 16}};

  inline static DecodedInstruction cb_instructions_[]{
      {0x00, "RLC B", AddressingMode::kNone, 2, 8},
      {0x01, "RLC C", AddressingMode::kNone, 2, 8},
      {0x02, "RLC D", AddressingMode::kNone, 2, 8},
      {0x03, "RLC E", AddressingMode::kNone, 2, 8},
      {0x04, "RLC H", AddressingMode::kNone, 2, 8},
      {0x05, "RLC L", AddressingMode::kNone, 2, 8},
      {0x06, "RLC (HL)", AddressingMode::kNone, 2, 16},
      {0x07, "RLC A", AddressingMode::kNone, 2, 8},
      {0x08, "RRC B", AddressingMode::kNone, 2, 8},
      {0x09, "RRC C", AddressingMode::kNone, 2, 8},
      {0x0A, "RRC D", AddressingMode::kNone, 2, 8},
      {0x0B, "RRC E", AddressingMode::kNone, 2, 8},
      {0x0C, "RRC H", AddressingMode::kNone, 2, 8},
      {0x0D, "RRC L", AddressingMode::kNone, 2, 8},
      {0x0E, "RRC (HL)", AddressingMode::kNone, 2, 16},
      {0x0F, "RRC A", AddressingMode::kNone, 2, 8},
      {0x10, "RL B", AddressingMode::kNone, 2, 8},
      {0x11, "RL C", AddressingMode::kNone, 2, 8},
      {0x12, "RL D", AddressingMode::kNone, 2, 8},
      {0x13, "RL E", AddressingMode::kNone, 2, 8},
      {0x14, "RL H", AddressingMode::kNone, 2, 8},
      {0x15, "RL L", AddressingMode::kNone, 2, 8},
      {0x16, "RL (HL)", AddressingMode::kNone, 2, 16},
      {0x17, "RL A", AddressingMode::kNone, 2, 8},
      {0x18, "RR B", AddressingMode::kNone, 2, 8},
      {0x19, "RR C", AddressingMode::kNone, 2, 8},
      {0x1A, "RR D", AddressingMode::kNone, 2, 8},
      {0x1B, "RR E", AddressingMode::kNone, 2, 8},
      {0x1C, "RR H", AddressingMode::kNone, 2, 8},
      {0x1D, "RR L", AddressingMode::kNone, 2, 8},
      {0x1E, "RR (HL)", AddressingMode::kNone, 2, 16},
      {0x1F, "RR A", AddressingMode::kNone, 2, 8},
      {0x20, "SLA B", AddressingMode::kNone, 2, 8},
      {0x21, "SLA C", AddressingMode::kNone, 2, 8},
      {0x22, "SLA D", AddressingMode::kNone, 2, 8},
      {0x23, "SLA E", AddressingMode::kNone, 2, 8},
      {0x24, "SLA H", AddressingMode::kNone, 2, 8},
      {0x25, "SLA L", AddressingMode::kNone, 2, 8},
      {0x26, "SLA (HL)", AddressingMode::kNone, 2, 16},
      {0x27, "SLA A", AddressingMode::kNone, 2, 8},
      {0x28, "SRA B", AddressingMode::kNone, 2, 8},
      {0x29, "SRA C", AddressingMode::kNone, 2, 8},
      {0x2A, "SRA D", AddressingMode::kNone, 2, 8},
      {0x2B, "SRA E", AddressingMode::kNone, 2, 8},
      {0x2C, "SRA H", AddressingMode::kNone, 2, 8},
      {0x2D, "SRA L", AddressingMode::kNone, 2, 8},
      {0x2E, "SRA (HL)", AddressingMode::kNone, 2, 16},
      {0x2F, "SRA A", AddressingMode::kNone, 2, 8},
      {0x30, "SWAP B", AddressingMode::kNone, 2, 8},
      {0x31, "SWAP C", AddressingMode::kNone, 2, 8},
      {0x32, "SWAP D", AddressingMode::kNone, 2, 8},
      {0x33, "SWAP E", AddressingMode::kNone, 2, 8},
      {0x34, "SWAP H", AddressingMode::kNone, 2, 8},
      {0x35, "SWAP L", AddressingMode::kNone, 2, 8},
      {0x36, "SWAP (HL)", AddressingMode::kNone, 2, 16},
      {0x37, "SWAP A", AddressingMode::kNone, 2, 8},
      {0x38, "SRL B", AddressingMode::kNone, 2, 8},
      {0x39, "SRL C", AddressingMode::kNone, 2, 8},
      {0x3A, "SRL D", AddressingMode::kNone, 2, 8},
      {0x3B, "SRL E", AddressingMode::kNone, 2, 8},
      {0x3C, "SRL H", AddressingMode::kNone, 2, 8},
      {0x3D, "SRL L", AddressingMode::kNone, 2, 8},
      {0x3E, "SRL (HL)", AddressingMode::kNone, 2, 16},
      {0x3F, "SRL A", AddressingMode::kNone, 2, 8},
      {0x40, "BIT 0, B", AddressingMode::kNone, 2, 8},
      {0x41, "BIT 0, C", AddressingMode::kNone, 2, 8},
      {0x42, "BIT 0, D", AddressingMode::kNone, 2, 8},
      {0x43, "BIT 0, E", AddressingMode::kNone, 2, 8},
      {0x44, "BIT 0, H", AddressingMode::kNone, 2, 8},
      {0x45, "BIT 0, L", AddressingMode::kNone, 2, 8},
      {0x46, "BIT 0, (HL)", AddressingMode::kNone, 2, 16},
      {0x47, "BIT 0, A", AddressingMode::kNone, 2, 8},
      {0x48, "BIT 1, B", AddressingMode::kNone, 2, 8},
      {0x49, "BIT 1, C", AddressingMode::kNone, 2, 8},
      {0x4A, "BIT 1, D", AddressingMode::kNone, 2, 8},
      {0x4B, "BIT 1, E", AddressingMode::kNone, 2, 8},
      {0x4C, "BIT 1, H", AddressingMode::kNone, 2, 8},
      {0x4D, "BIT 1, L", AddressingMode::kNone, 2, 8},
      {0x4E, "BIT 1, (HL)", AddressingMode::kNone, 2, 16},
      {0x4F, "BIT 1, A", AddressingMode::kNone, 2, 8},
      {0x50, "BIT 2, B", AddressingMode::kNone, 2, 8},
      {0x51, "BIT 2, C", AddressingMode::kNone, 2, 8},
      {0x52, "BIT 2, D", AddressingMode::kNone, 2, 8},
      {0x53, "BIT 2, E", AddressingMode::kNone, 2, 8},
      {0x54, "BIT 2, H", AddressingMode::kNone, 2, 8},
      {0x55, "BIT 2, L", AddressingMode::kNone, 2, 8},
      {0x56, "BIT 2, (HL)", AddressingMode::kNone, 2, 16},
      {0x57, "BIT 2, A", AddressingMode::kNone, 2, 8},
      {0x58, "BIT 3, B", AddressingMode::kNone, 2, 8},
      {0x59, "BIT 3, C", AddressingMode::kNone, 2, 8},
      {0x5A, "BIT 3, D", AddressingMode::kNone, 2, 8},
      {0x5B, "BIT 3, E", AddressingMode::kNone, 2, 8},
      {0x5C, "BIT 3, H", AddressingMode::kNone, 2, 8},
      {0x5D, "BIT 3, L", AddressingMode::kNone, 2, 8},
      {0x5E, "BIT 3, (HL)", AddressingMode::kNone, 2, 16},
      {0x5F, "BIT 3, A", AddressingMode::kNone, 2, 8},
      {0x60, "BIT 4, B", AddressingMode::kNone, 2, 8},
      {0x61, "BIT 4, C", AddressingMode::kNone, 2, 8},
      {0x62, "BIT 4, D", AddressingMode::kNone, 2, 8},
      {0x63, "BIT 4, E", AddressingMode::kNone, 2, 8},
      {0x64, "BIT 4, H", AddressingMode::kNone, 2, 8},
      {0x65, "BIT 4, L", AddressingMode::kNone, 2, 8},
      {0x66, "BIT 4, (HL)", AddressingMode::kNone, 2, 16},
      {0x67, "BIT 4, A", AddressingMode::kNone, 2, 8},
      {0x68, "BIT 5, B", AddressingMode::kNone, 2, 8},
      {0x69, "BIT 5, C", AddressingMode::kNone, 2, 8},
      {0x6A, "BIT 5, D", AddressingMode::kNone, 2, 8},
      {0x6B, "BIT 5, E", AddressingMode::kNone, 2, 8},
      {0x6C, "BIT 5, H", AddressingMode::kNone, 2, 8},
      {0x6D, "BIT 5, L", AddressingMode::kNone, 2, 8},
      {0x6E, "BIT 5, (HL)", AddressingMode::kNone, 2, 16},
      {0x6F, "BIT 5, A", AddressingMode::kNone, 2, 8},
      {0x70, "BIT 6, B", AddressingMode::kNone, 2, 8},
      {0x71, "BIT 6, C", AddressingMode::kNone, 2, 8},
      {0x72, "BIT 6, D", AddressingMode::kNone, 2, 8},
      {0x73, "BIT 6, E", AddressingMode::kNone, 2, 8},
      {0x74, "BIT 6, H", AddressingMode::kNone, 2, 8},
      {0x75, "BIT 6, L", AddressingMode::kNone, 2, 8},
      {0x76, "BIT 6, (HL)", AddressingMode::kNone, 2, 16},
      {0x77, "BIT 6, A", AddressingMode::kNone, 2, 8},
      {0x78, "BIT 7, B", AddressingMode::kNone, 2, 8},
      {0x79, "BIT 7, C", AddressingMode::kNone, 2, 8},
      {0x7A, "BIT 7, D", AddressingMode::kNone, 2, 8},
      {0x7B, "BIT 7, E", AddressingMode::kNone, 2, 8},
      {0x7C, "BIT 7, H", AddressingMode::kNone, 2, 8},
      {0x7D, "BIT 7, L", AddressingMode::kNone, 2, 8},
      {0x7E, "BIT 7, (HL)", AddressingMode::kNone, 2, 16},
      {0x7F, "BIT 7, A", AddressingMode::kNone, 2, 8},
      {0x80, "RES 0, B", AddressingMode::kNone, 2, 8},
      {0x81, "RES 0, C", AddressingMode::kNone, 2, 8},
      {0x82, "RES 0, D", AddressingMode::kNone, 2, 8},
      {0x83, "RES 0, E", AddressingMode::kNone, 2, 8},
      {0x84, "RES 0, H", AddressingMode::kNone, 2, 8},
      {0x85, "RES 0, L", AddressingMode::kNone, 2, 8},
      {0x86, "RES 0, (HL)", AddressingMode::kNone, 2, 16},
      {0x87, "RES 0, A", AddressingMode::kNone, 2, 8},
      {0x88, "RES 1, B", AddressingMode::kNone, 2, 8},
      {0x89, "RES 1, C", AddressingMode::kNone, 2, 8},
      {0x8A, "RES 1, D", AddressingMode::kNone, 2, 8},
      {0x8B, "RES 1, E", AddressingMode::kNone, 2, 8},
      {0x8C, "RES 1, H", AddressingMode::kNone, 2, 8},
      {0x8D, "RES 1, L", AddressingMode::kNone, 2, 8},
      {0x8E, "RES 1, (HL)", AddressingMode::kNone, 2, 16},
      {0x8F, "RES 1, A", AddressingMode::kNone, 2, 8},
      {0x90, "RES 2, B", AddressingMode::kNone, 2, 8},
      {0x91, "RES 2, C", AddressingMode::kNone, 2, 8},
      {0x92, "RES 2, D", AddressingMode::kNone, 2, 8},
      {0x93, "RES 2, E", AddressingMode::kNone, 2, 8},
      {0x94, "RES 2, H", AddressingMode::kNone, 2, 8},
      {0x95, "RES 2, L", AddressingMode::kNone, 2, 8},
      {0x96, "RES 2, (HL)", AddressingMode::kNone, 2, 16},
      {0x97, "RES 2, A", AddressingMode::kNone, 2, 8},
      {0x98, "RES 3, B", AddressingMode::kNone, 2, 8},
      {0x99, "RES 3, C", AddressingMode::kNone, 2, 8},
      {0x9A, "RES 3, D", AddressingMode::kNone, 2, 8},
      {0x9B, "RES 3, E", AddressingMode::kNone, 2, 8},
      {0x9C, "RES 3, H", AddressingMode::kNone, 2, 8},
      {0x9D, "RES 3, L", AddressingMode::kNone, 2, 8},
      {0x9E, "RES 3, (HL)", AddressingMode::kNone, 2, 16},
      {0x9F, "RES 3, A", AddressingMode::kNone, 2, 8},
      {0xA0, "RES 4, B", AddressingMode::kNone, 2, 8},
      {0xA1, "RES 4, C", AddressingMode::kNone, 2, 8},
      {0xA2, "RES 4, D", AddressingMode::kNone, 2, 8},
      {0xA3, "RES 4, E", AddressingMode::kNone, 2, 8},
      {0xA4, "RES 4, H", AddressingMode::kNone, 2, 8},
      {0xA5, "RES 4, L", AddressingMode::kNone, 2, 8},
      {0xA6, "RES 4, (HL)", AddressingMode::kNone, 2, 16},
      {0xA7, "RES 4, A", AddressingMode::kNone, 2, 8},
      {0xA8, "RES 5, B", AddressingMode::kNone, 2, 8},
      {0xA9, "RES 5, C", AddressingMode::kNone, 2, 8},
      {0xAA, "RES 5, D", AddressingMode::kNone, 2, 8},
      {0xAB, "RES 5, E", AddressingMode::kNone, 2, 8},
      {0xAC, "RES 5, H", AddressingMode::kNone, 2, 8},
      {0xAD, "RES 5, L", AddressingMode::kNone, 2, 8},
      {0xAE, "RES 5, (HL)", AddressingMode::kNone, 2, 16},
      {0xAF, "RES 5, A", AddressingMode::kNone, 2, 8},
      {0xB0, "RES 6, B", AddressingMode::kNone, 2, 8},
      {0xB1, "RES 6, C", AddressingMode::kNone, 2, 8},
      {0xB2, "RES 6, D", AddressingMode::kNone, 2, 8},
      {0xB3, "RES 6, E", AddressingMode::kNone, 2, 8},
      {0xB4, "RES 6, H", AddressingMode::kNone, 2, 8},
      {0xB5, "RES 6, L", AddressingMode::kNone, 2, 8},
      {0xB6, "RES 6, (HL)", AddressingMode::kNone, 2, 16},
      {0xB7, "RES 6, A", AddressingMode::kNone, 2, 8},
      {0xB8, "RES 7, B", AddressingMode::kNone, 2, 8},
      {0xB9, "RES 7, C", AddressingMode::kNone, 2, 8},
      {0xBA, "RES 7, D", AddressingMode::kNone, 2, 8},
      {0xBB, "RES 7, E", AddressingMode::kNone, 2, 8},
      {0xBC, "RES 7, H", AddressingMode::kNone, 2, 8},
      {0xBD, "RES 7, L", AddressingMode::kNone, 2, 8},
      {0xBE, "RES 7, (HL)", AddressingMode::kNone, 2, 16},
      {0xBF, "RES 7, A", AddressingMode::kNone, 2, 8},
      {0xC0, "SET 0, B", AddressingMode::kNone, 2, 8},
      {0xC1, "SET 0, C", AddressingMode::kNone, 2, 8},
      {0xC2, "SET 0, D", AddressingMode::kNone, 2, 8},
      {0xC3, "SET 0, E", AddressingMode::kNone, 2, 8},
      {0xC4, "SET 0, H", AddressingMode::kNone, 2, 8},
      {0xC5, "SET 0, L", AddressingMode::kNone, 2, 8},
      {0xC6, "SET 0, (HL)", AddressingMode::kNone, 2, 16},
      {0xC7, "SET 0, A", AddressingMode::kNone, 2, 8},
      {0xC8, "SET 1, B", AddressingMode::kNone, 2, 8},
      {0xC9, "SET 1, C", AddressingMode::kNone, 2, 8},
      {0xCA, "SET 1, D", AddressingMode::kNone, 2, 8},
      {0xCB, "SET 1, E", AddressingMode::kNone, 2, 8},
      {0xCC, "SET 1, H", AddressingMode::kNone, 2, 8},
      {0xCD, "SET 1, L", AddressingMode::kNone, 2, 8},
      {0xCE, "SET 1, (HL)", AddressingMode::kNone, 2, 16},
      {0xCF, "SET 1, A", AddressingMode::kNone, 2, 8},
      {0xD0, "SET 2, B", AddressingMode::kNone, 2, 8},
      {0xD1, "SET 2, C", AddressingMode::kNone, 2, 8},
      {0xD2, "SET 2, D", AddressingMode::kNone, 2, 8},
      {0xD3, "SET 2, E", AddressingMode::kNone, 2, 8},
      {0xD4, "SET 2, H", AddressingMode::kNone, 2, 8},
      {0xD5, "SET 2, L", AddressingMode::kNone, 2, 8},
      {0xD6, "SET 2, (HL)", AddressingMode::kNone, 2, 16},
      {0xD7, "SET 2, A", AddressingMode::kNone, 2, 8},
      {0xD8, "SET 3, B", AddressingMode::kNone, 2, 8},
      {0xD9, "SET 3, C", AddressingMode::kNone, 2, 8},
      {0xDA, "SET 3, D", AddressingMode::kNone, 2, 8},
      {0xDB, "SET 3, E", AddressingMode::kNone, 2, 8},
      {0xDC, "SET 3, H", AddressingMode::kNone, 2, 8},
      {0xDD, "SET 3, L", AddressingMode::kNone, 2, 8},
      {0xDE, "SET 3, (HL)", AddressingMode::kNone, 2, 16},
      {0xDF, "SET 3, A", AddressingMode::kNone, 2, 8},
      {0xE0, "SET 4, B", AddressingMode::kNone, 2, 8},
      {0xE1, "SET 4, C", AddressingMode::kNone, 2, 8},
      {0xE2, "SET 4, D", AddressingMode::kNone, 2, 8},
      {0xE3, "SET 4, E", AddressingMode::kNone, 2, 8},
      {0xE4, "SET 4, H", AddressingMode::kNone, 2, 8},
      {0xE5, "SET 4, L", AddressingMode::kNone, 2, 8},
      {0xE6, "SET 4, (HL)", AddressingMode::kNone, 2, 16},
      {0xE7, "SET 4, A", AddressingMode::kNone, 2, 8},
      {0xE8, "SET 5, B", AddressingMode::kNone, 2, 8},
      {0xE9, "SET 5, C", AddressingMode::kNone, 2, 8},
      {0xEA, "SET 5, D", AddressingMode::kNone, 2, 8},
      {0xEB, "SET 5, E", AddressingMode::kNone, 2, 8},
      {0xEC, "SET 5, H", AddressingMode::kNone, 2, 8},
      {0xED, "SET 5, L", AddressingMode::kNone, 2, 8},
      {0xEE, "SET 5, (HL)", AddressingMode::kNone, 2, 16},
      {0xEF, "SET 5, A", AddressingMode::kNone, 2, 8},
      {0xF0, "SET 6, B", AddressingMode::kNone, 2, 8},
      {0xF1, "SET 6, C", AddressingMode::kNone, 2, 8},
      {0xF2, "SET 6, D", AddressingMode::kNone, 2, 8},
      {0xF3, "SET 6, E", AddressingMode::kNone, 2, 8},
      {0xF4, "SET 6, H", AddressingMode::kNone, 2, 8},
      {0xF5, "SET 6, L", AddressingMode::kNone, 2, 8},
      {0xF6, "SET 6, (HL)", AddressingMode::kNone, 2, 16},
      {0xF7, "SET 6, A", AddressingMode::kNone, 2, 8},
      {0xF8, "SET 7, B", AddressingMode::kNone, 2, 8},
      {0xF9, "SET 7, C", AddressingMode::kNone, 2, 8},
      {0xFA, "SET 7, D", AddressingMode::kNone, 2, 8},
      {0xFB, "SET 7, E", AddressingMode::kNone, 2, 8},
      {0xFC, "SET 7, H", AddressingMode::kNone, 2, 8},
      {0xFD, "SET 7, L", AddressingMode::kNone, 2, 8},
      {0xFE, "SET 7, (HL)", AddressingMode::kNone, 2, 16},
      {0xFF, "SET 7, A", AddressingMode::kNone, 2, 8}};

 public:
  static DecodedInstruction Decode(const uint8_t opcode,
                                   const bool cb = false) {
    if (cb) {
      return cb_instructions_[opcode];
    }
    return instructions_[opcode];
  }
};

#endif  // !INSTRUCTIONS_H
