#include "cpu.h"
#include "bit_utility.h"
#include "gb.h"
#include "instructions.h"
#include "mmu.h"
#include "spdlog/spdlog.h"
#include <cstdint>

CPU::CPU(MMU &mmu) : sp_(0xFFFE), mmu_(mmu) {
  registers_.af = 0x01B0;
  registers_.bc = 0x0013;
  registers_.de = 0x00D8;
  registers_.hl = 0x014D;
  mmu_.boot_rom_enabled ? pc_ = 0 : pc_ = 0x100;
  cycles = 0;
}

void CPU::HandleInterrupts() {
  uint8_t if_reg = mmu_.ReadByte(IF);
  const uint8_t ie_reg = mmu_.ReadByte(IE);

  if (!ime_ || (if_reg & 0x1F) == 0 || (ie_reg & 0x1F) == 0)
    return; // interrupts globally disabled or nothing to handle
  // check register 0xFF0F to see which interrupt was generated
  constexpr uint16_t offset[]{0x0040, 0x0048, 0x0050, 0x0058, 0x0060};
  for (uint8_t i = 0; i < 5; ++i) {
    if (bit_check(if_reg, i) && bit_check(ie_reg, i)) {
      // put current pc on stack and head to the proper service routine
      mmu_.WriteByte(--sp_, pc_ >> 8);
      mmu_.WriteByte(--sp_, static_cast<uint8_t>(pc_));
      pc_ = offset[i];
      // clear teh bit
      bit_clear(if_reg, i);
      mmu_.WriteByte(IF, if_reg);
      return;
    }
  }
}

DecodedInstruction CPU::Execute() {
  // auto opcode = static_cast<Instruction>(mmu_.ReadByte(pc_));
  const auto opcode = mmu_.ReadByte(pc_);
  auto decoded = Instructions::Decode(opcode);
  bool debug = false;
  // executed_instructions_.push_back(decoded);
  switch (decoded.mode) {
  case AddressingMode::kDirect:
    decoded.operand = (mmu_.ReadByte(pc_ + 2) << 8) | mmu_.ReadByte(pc_ + 1);
    if (debug) {
      spdlog::get("file logger")
          ->debug("0x{3:04X}: {0:02X} {1:04X} | {2}", decoded.opcode,
                  decoded.operand.value(), decoded.name, pc_);
    }
    break;
  case AddressingMode::kImmediate:
    decoded.operand = mmu_.ReadByte(pc_ + 1);
    if (debug) {
      spdlog::get("file logger")
          ->debug("0x{3:04X}: {0:02X} {1:02X}   | {2}", decoded.opcode,
                  decoded.operand.value(), decoded.name, pc_);
    }
    break;
  case AddressingMode::kIndirect:
    decoded.operand = static_cast<int8_t>(mmu_.ReadByte(pc_ + 1));
    if (debug) {
      spdlog::get("file logger")
          ->debug("0x{2:04X}: {0:02X}      | {1}", decoded.opcode, decoded.name,
                  pc_);
    }
    break;
  default:
    if (debug) {
      spdlog::get("file logger")
          ->debug("0x{2:04X}: {0:02x}      | {1}", decoded.opcode, decoded.name,
                  pc_);
    }
    break;
  }

  executed_instructions_.push_back(decoded);
  while (executed_instructions_.size() > 5) {
    executed_instructions_.pop_front();
  }
  switch (static_cast<Instruction>(decoded.opcode)) {
    // CB prefixed opcodes
  case Instruction::prefix_cb: {
    decoded = Instructions::Decode(mmu_.ReadByte(pc_ + 1), true);
    executed_instructions_.push_back(decoded);
    switch (static_cast<PrefixCB>(decoded.opcode)) {
    case PrefixCB::rlc_b:
      rlc(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rlc_c:
      rlc(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rlc_d:
      rlc(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rlc_e:
      rlc(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rlc_h:
      rlc(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rlc_l:
      rlc(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rlc_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      rlc(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::rlc_a:
      rlc(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rrc_b:
      rrc(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rrc_c:
      rrc(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rrc_d:
      rrc(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rrc_e:
      rrc(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rrc_h:
      rrc(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rrc_l:
      rrc(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rrc_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      rrc(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::rrc_a:
      rrc(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rl_b:
      rl(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rl_c:
      rl(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rl_d:
      rl(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rl_e:
      rl(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rl_h:
      rl(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rl_l:
      rl(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rl_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      rl(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::rl_a:
      rl(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rr_b:
      rr(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rr_c:
      rr(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rr_d:
      rr(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rr_e:
      rr(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rr_h:
      rr(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rr_l:
      rr(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::rr_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      rr(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::rr_a:
      rr(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sla_b:
      sla(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sla_c:
      sla(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sla_d:
      sla(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sla_e:
      sla(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sla_h:
      sla(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sla_l:
      sla(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sla_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      sla(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::sla_a:
      sla(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sra_b:
      sra(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sra_c:
      sra(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sra_d:
      sra(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sra_e:
      sra(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sra_h:
      sra(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sra_l:
      sra(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::sra_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      sra(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::sra_a:
      sra(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::swap_b:
      swap(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::swap_c:
      swap(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::swap_d:
      swap(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::swap_e:
      swap(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::swap_h:
      swap(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::swap_l:
      swap(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::swap_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      swap(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::swap_a:
      swap(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::srl_b:
      srl(registers_.b);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::srl_c:
      srl(registers_.c);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::srl_d:
      srl(registers_.d);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::srl_e:
      srl(registers_.e);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::srl_h:
      srl(registers_.h);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::srl_l:
      srl(registers_.l);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::srl_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      srl(val);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::srl_a:
      srl(registers_.a);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_b:
      flags_.z = !bit_check(registers_.b, 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_c:
      flags_.z = !bit_check(registers_.c, 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_d:
      flags_.z = !bit_check(registers_.d, 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_e:
      flags_.z = !bit_check(registers_.e, 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_h:
      flags_.z = !bit_check(registers_.h, 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_l:
      flags_.z = !bit_check(registers_.l, 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_0_a:
      flags_.z = !bit_check(registers_.a, 0);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_b:
      flags_.z = !bit_check(registers_.b, 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_c:
      flags_.z = !bit_check(registers_.c, 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_d:
      flags_.z = !bit_check(registers_.d, 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_e:
      flags_.z = !bit_check(registers_.e, 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_h:
      flags_.z = !bit_check(registers_.h, 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_l:
      flags_.z = !bit_check(registers_.l, 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_1_a:
      flags_.z = !bit_check(registers_.a, 1);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_b:
      flags_.z = !bit_check(registers_.b, 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_c:
      flags_.z = !bit_check(registers_.c, 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_d:
      flags_.z = !bit_check(registers_.d, 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_e:
      flags_.z = !bit_check(registers_.e, 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_h:
      flags_.z = !bit_check(registers_.h, 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_l:
      flags_.z = !bit_check(registers_.l, 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_2_a:
      flags_.z = !bit_check(registers_.a, 2);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_b:
      flags_.z = !bit_check(registers_.b, 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_c:
      flags_.z = !bit_check(registers_.c, 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_d:
      flags_.z = !bit_check(registers_.d, 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_e:
      flags_.z = !bit_check(registers_.e, 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_h:
      flags_.z = !bit_check(registers_.h, 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_l:
      flags_.z = !bit_check(registers_.l, 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_3_a:
      flags_.z = !bit_check(registers_.a, 3);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_b:
      flags_.z = !bit_check(registers_.b, 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_c:
      flags_.z = !bit_check(registers_.c, 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_d:
      flags_.z = !bit_check(registers_.d, 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_e:
      flags_.z = !bit_check(registers_.e, 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_h:
      flags_.z = !bit_check(registers_.h, 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_l:
      flags_.z = !bit_check(registers_.l, 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_4_a:
      flags_.z = !bit_check(registers_.a, 4);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_b:
      flags_.z = !bit_check(registers_.b, 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_c:
      flags_.z = !bit_check(registers_.c, 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_d:
      flags_.z = !bit_check(registers_.d, 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_e:
      flags_.z = !bit_check(registers_.e, 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_h:
      flags_.z = !bit_check(registers_.h, 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_l:
      flags_.z = !bit_check(registers_.l, 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_5_a:
      flags_.z = !bit_check(registers_.a, 5);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_b:
      flags_.z = !bit_check(registers_.b, 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_c:
      flags_.z = !bit_check(registers_.c, 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_d:
      flags_.z = !bit_check(registers_.d, 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_e:
      flags_.z = !bit_check(registers_.e, 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_h:
      flags_.z = !bit_check(registers_.h, 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_l:
      flags_.z = !bit_check(registers_.l, 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_6_a:
      flags_.z = !bit_check(registers_.a, 6);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_b:
      flags_.z = !bit_check(registers_.b, 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_c:
      flags_.z = !bit_check(registers_.c, 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_d:
      flags_.z = !bit_check(registers_.d, 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_e:
      flags_.z = !bit_check(registers_.e, 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_h:
      flags_.z = !bit_check(registers_.h, 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_l:
      flags_.z = !bit_check(registers_.l, 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_hl:
      flags_.z = !bit_check(mmu_.ReadByte(registers_.hl), 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::bit_7_a:
      flags_.z = !bit_check(registers_.a, 7);
      flags_.n = false;
      flags_.h = true;
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_0_b:
      bit_clear(registers_.b, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_0_c:
      bit_clear(registers_.c, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_0_d:
      bit_clear(registers_.d, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_0_e:
      bit_clear(registers_.e, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_0_h:
      bit_clear(registers_.h, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_0_l:
      bit_clear(registers_.l, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_0_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 0);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_0_a:
      bit_clear(registers_.a, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_1_b:
      bit_clear(registers_.b, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_1_c:
      bit_clear(registers_.c, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_1_d:
      bit_clear(registers_.d, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_1_e:
      bit_clear(registers_.e, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_1_h:
      bit_clear(registers_.h, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_1_l:
      bit_clear(registers_.l, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_1_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 1);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_1_a:
      bit_clear(registers_.a, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_2_b:
      bit_clear(registers_.b, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_2_c:
      bit_clear(registers_.c, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_2_d:
      bit_clear(registers_.d, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_2_e:
      bit_clear(registers_.e, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_2_h:
      bit_clear(registers_.h, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_2_l:
      bit_clear(registers_.l, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_2_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 2);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_2_a:
      bit_clear(registers_.a, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_3_b:
      bit_clear(registers_.b, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_3_c:
      bit_clear(registers_.c, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_3_d:
      bit_clear(registers_.d, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_3_e:
      bit_clear(registers_.e, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_3_h:
      bit_clear(registers_.h, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_3_l:
      bit_clear(registers_.l, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_3_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 3);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_3_a:
      bit_clear(registers_.a, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_4_b:
      bit_clear(registers_.b, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_4_c:
      bit_clear(registers_.c, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_4_d:
      bit_clear(registers_.d, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_4_e:
      bit_clear(registers_.e, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_4_h:
      bit_clear(registers_.h, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_4_l:
      bit_clear(registers_.l, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_4_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 4);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_4_a:
      bit_clear(registers_.a, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_5_b:
      bit_clear(registers_.b, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_5_c:
      bit_clear(registers_.c, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_5_d:
      bit_clear(registers_.d, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_5_e:
      bit_clear(registers_.e, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_5_h:
      bit_clear(registers_.h, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_5_l:
      bit_clear(registers_.l, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_5_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 5);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_5_a:
      bit_clear(registers_.a, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_6_b:
      bit_clear(registers_.b, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_6_c:
      bit_clear(registers_.c, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_6_d:
      bit_clear(registers_.d, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_6_e:
      bit_clear(registers_.e, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_6_h:
      bit_clear(registers_.h, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_6_l:
      bit_clear(registers_.l, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_6_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 6);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_6_a:
      bit_clear(registers_.a, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_7_b:
      bit_clear(registers_.b, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_7_c:
      bit_clear(registers_.c, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_7_d:
      bit_clear(registers_.d, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_7_e:
      bit_clear(registers_.e, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_7_h:
      bit_clear(registers_.h, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_7_l:
      bit_clear(registers_.l, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::res_7_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_clear(val, 7);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::res_7_a:
      bit_clear(registers_.a, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_0_b:
      bit_set(registers_.b, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_0_c:
      bit_set(registers_.c, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_0_d:
      bit_set(registers_.d, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_0_e:
      bit_set(registers_.e, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_0_h:
      bit_set(registers_.h, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_0_l:
      bit_set(registers_.l, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_0_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 0);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_0_a:
      bit_set(registers_.a, 0);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_1_b:
      bit_set(registers_.b, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_1_c:
      bit_set(registers_.c, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_1_d:
      bit_set(registers_.d, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_1_e:
      bit_set(registers_.e, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_1_h:
      bit_set(registers_.h, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_1_l:
      bit_set(registers_.l, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_1_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 1);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_1_a:
      bit_set(registers_.a, 1);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_2_b:
      bit_set(registers_.b, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_2_c:
      bit_set(registers_.c, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_2_d:
      bit_set(registers_.d, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_2_e:
      bit_set(registers_.e, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_2_h:
      bit_set(registers_.h, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_2_l:
      bit_set(registers_.l, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_2_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 2);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_2_a:
      bit_set(registers_.a, 2);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_3_b:
      bit_set(registers_.b, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_3_c:
      bit_set(registers_.c, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_3_d:
      bit_set(registers_.d, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_3_e:
      bit_set(registers_.e, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_3_h:
      bit_set(registers_.h, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_3_l:
      bit_set(registers_.l, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_3_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 3);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_3_a:
      bit_set(registers_.a, 3);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_4_b:
      bit_set(registers_.b, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_4_c:
      bit_set(registers_.c, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_4_d:
      bit_set(registers_.d, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_4_e:
      bit_set(registers_.e, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_4_h:
      bit_set(registers_.h, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_4_l:
      bit_set(registers_.l, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_4_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 4);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_4_a:
      bit_set(registers_.a, 4);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_5_b:
      bit_set(registers_.b, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_5_c:
      bit_set(registers_.c, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_5_d:
      bit_set(registers_.d, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_5_e:
      bit_set(registers_.e, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_5_h:
      bit_set(registers_.h, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_5_l:
      bit_set(registers_.l, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_5_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 5);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_5_a:
      bit_set(registers_.a, 5);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_6_b:
      bit_set(registers_.b, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_6_c:
      bit_set(registers_.c, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_6_d:
      bit_set(registers_.d, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_6_e:
      bit_set(registers_.e, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_6_h:
      bit_set(registers_.h, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_6_l:
      bit_set(registers_.l, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_6_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 6);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_6_a:
      bit_set(registers_.a, 6);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_7_b:
      bit_set(registers_.b, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_7_c:
      bit_set(registers_.c, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_7_d:
      bit_set(registers_.d, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_7_e:
      bit_set(registers_.e, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_7_h:
      bit_set(registers_.h, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_7_l:
      bit_set(registers_.l, 7);
      pc_ += 2;
      cycles = 8;
      break;
    case PrefixCB::set_7_hl: {
      uint8_t val = mmu_.ReadByte(registers_.hl);
      bit_set(val, 7);
      mmu_.WriteByte(registers_.hl, val);
      pc_ += 2;
      cycles = 16;
      break;
    }
    case PrefixCB::set_7_a:
      bit_set(registers_.a, 7);
      pc_ += 2;
      cycles = 8;
      break;
    default:
      // printf("Unknown opcode: 0x%0.2X 0x%0.2X\n", opcode, opcode2);
      spdlog::get("stdout")->error(
          "Unknown opcode 0x{0:02X} 0x{1:02X} at 0x{2:04X}", (uint8_t)opcode,
          (uint8_t)mmu_.ReadByte(pc_ + 1), pc_);
      break;
    }
    break;
  }
  case Instruction::nop:
    ++pc_;
    cycles = 4;
    break;
  case Instruction::stop:
    // TODO: Implement
    pc_ += 2;
    cycles = 4;
    break;
    // 8 bit loads
  case Instruction::ld_b_d8:
    registers_.b = mmu_.ReadByte(pc_ + 1);
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::ld_c_d8: // LD C,n
    registers_.c = mmu_.ReadByte(pc_ + 1);
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::ld_d_d8: // LD D,n
    registers_.d = mmu_.ReadByte(pc_ + 1);
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::ld_e_d8: // LD E,n
    registers_.e = mmu_.ReadByte(pc_ + 1);
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::ld_h_d8: // LD H,n
    registers_.h = mmu_.ReadByte(pc_ + 1);
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::ld_l_d8: // LD L,n
    registers_.l = mmu_.ReadByte(pc_ + 1);
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::ld_a_a:
    // ??
    registers_.a = registers_.a;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_b:
    registers_.a = registers_.b;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_c:
    registers_.a = registers_.c;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_d:
    registers_.a = registers_.d;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_e:
    registers_.a = registers_.e;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_h:
    registers_.a = registers_.h;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_l:
    registers_.a = registers_.l;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_bc:
    registers_.a = mmu_.ReadByte(registers_.bc);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_a_de:
    registers_.a = mmu_.ReadByte(registers_.de);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_a_at_hl:
    registers_.a = mmu_.ReadByte(registers_.hl);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_a_a16: {
    uint16_t address = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
    registers_.a = mmu_.ReadByte(address);
    pc_ += 3;
    cycles = 16;
    break;
  }
  case Instruction::ld_b_a:
    registers_.b = registers_.a;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_b_b:
    registers_.b = registers_.b;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_b_c:
    registers_.b = registers_.c;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_b_d:
    registers_.b = registers_.d;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_b_e:
    registers_.b = registers_.e;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_b_h:
    registers_.b = registers_.h;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_b_l:
    registers_.b = registers_.l;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_b_hl:
    registers_.b = mmu_.ReadByte(registers_.hl);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_c_a:
    registers_.c = registers_.a;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_c_b:
    registers_.c = registers_.b;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_c_c:
    registers_.c = registers_.c;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_c_d:
    registers_.c = registers_.d;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_c_e:
    registers_.c = registers_.e;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_c_h:
    registers_.c = registers_.h;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_c_l:
    registers_.c = registers_.l;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_c_hl:
    registers_.c = mmu_.ReadByte(registers_.hl);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_d_a:
    registers_.d = registers_.a;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_d_b:
    registers_.d = registers_.b;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_d_c:
    registers_.d = registers_.c;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_d_d:
    registers_.d = registers_.d;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_d_e:
    registers_.d = registers_.e;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_d_h:
    registers_.d = registers_.h;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_d_l:
    registers_.d = registers_.l;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_d_hl:
    registers_.d = mmu_.ReadByte(registers_.hl);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_e_a:
    registers_.e = registers_.a;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_e_b:
    registers_.e = registers_.b;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_e_c:
    registers_.e = registers_.c;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_e_d:
    registers_.e = registers_.d;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_e_e:
    registers_.e = registers_.e;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_e_h:
    registers_.e = registers_.h;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_e_l:
    registers_.e = registers_.l;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_e_hl:
    registers_.e = mmu_.ReadByte(registers_.hl);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_h_a:
    registers_.h = registers_.a;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_h_b:
    registers_.h = registers_.b;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_h_c:
    registers_.h = registers_.c;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_h_d:
    registers_.h = registers_.d;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_h_e:
    registers_.h = registers_.e;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_h_h:
    registers_.h = registers_.h;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_h_l:
    registers_.h = registers_.l;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_h_hl:
    registers_.h = mmu_.ReadByte(registers_.hl);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_l_a:
    registers_.l = registers_.a;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_l_b:
    registers_.l = registers_.b;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_l_c:
    registers_.l = registers_.c;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_l_d:
    registers_.l = registers_.d;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_l_e:
    registers_.l = registers_.e;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_l_h:
    registers_.l = registers_.h;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_l_l:
    registers_.l = registers_.l;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_l_hl:
    registers_.l = mmu_.ReadByte(registers_.hl);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_a16_a: {
    uint16_t address = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
    mmu_.WriteByte(address, registers_.a);
    cycles = 16;
    pc_ += 3;
    break;
  }
  case Instruction::ldd_hl_a: // ld (HL-), A
  {
    mmu_.WriteByte(registers_.hl, registers_.a);
    --registers_.hl;
    ++pc_;
    cycles = 8;
    break;
  }
  case Instruction::ld_a_d8: // ld A, #
    registers_.a = mmu_.ReadByte(pc_ + 1);
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::ld_at_c_a: // ld (C), a
  {
    // TODO: check this
    uint16_t address = 0xFF00 + registers_.c;
    mmu_.WriteByte(address, registers_.a);
    ++pc_;
    cycles = 8;
    break;
  }
  case Instruction::ld_bc_a:
    mmu_.WriteByte(registers_.bc, registers_.a);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_de_a:
    mmu_.WriteByte(registers_.de, registers_.a);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_a:
    mmu_.WriteByte(registers_.hl, registers_.a);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_b:
    mmu_.WriteByte(registers_.hl, registers_.b);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_c:
    mmu_.WriteByte(registers_.hl, registers_.c);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_d:
    mmu_.WriteByte(registers_.hl, registers_.d);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_e:
    mmu_.WriteByte(registers_.hl, registers_.e);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_h:
    mmu_.WriteByte(registers_.hl, registers_.h);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_l:
    mmu_.WriteByte(registers_.hl, registers_.l);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ldh_a8_a: {
    uint16_t address = 0xFF00 + mmu_.ReadByte(pc_ + 1);
    mmu_.WriteByte(address, registers_.a);
    pc_ += 2;
    cycles = 12;
    break;
  }
  case Instruction::ldh_a_a8: {
    uint16_t address = 0xFF00 + mmu_.ReadByte(pc_ + 1);
    registers_.a = mmu_.ReadByte(address);
    pc_ += 2;
    cycles = 12;
    break;
  }
  case Instruction::ldi_hl_a:
    mmu_.WriteByte(registers_.hl, registers_.a);
    ++registers_.hl;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ld_hl_d8: {
    uint8_t val = mmu_.ReadByte(pc_ + 1);
    mmu_.WriteByte(registers_.hl, val);
    pc_ += 2;
    cycles = 12;
    break;
  }
  case Instruction::ldi_a_hl:
    registers_.a = mmu_.ReadByte(registers_.hl);
    ++registers_.hl;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::ldd_a_hl:
    registers_.a = mmu_.ReadByte(registers_.hl--);
    ++pc_;
    cycles = 8;
    break;
    // 16 bit loads
  case Instruction::ld_a16_sp: {
    uint16_t address = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
    mmu_.WriteByte(address, static_cast<uint8_t>(sp_)); // LSB
    mmu_.WriteByte(address + 1, sp_ >> 8);              // MSB
    pc_ += 3;
    cycles = 20;
    break;
  }
  case Instruction::ld_sp_d16:
    sp_ = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
    pc_ += 3;
    cycles = 12;
    break;
  case Instruction::ld_bc_d16:
    registers_.bc = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
    pc_ += 3;
    cycles = 12;
    break;
  case Instruction::ld_hl_d16:
    registers_.h = mmu_.ReadByte(pc_ + 2);
    registers_.l = mmu_.ReadByte(pc_ + 1);
    pc_ += 3;
    cycles = 12;
    break;
  case Instruction::ld_de_d16:
    registers_.d = mmu_.ReadByte(pc_ + 2);
    registers_.e = mmu_.ReadByte(pc_ + 1);
    pc_ += 3;
    cycles = 12;
    break;
  case Instruction::push_af:
    // since we don't use the bits of the F register
    // and we need to save the flags_ state, set them now
    flags_.z ? SetZ() : ResetZ();
    flags_.n ? SetN() : ResetN();
    flags_.h ? SetH() : ResetH();
    flags_.c ? SetC() : ResetC();
    // reset unused bits 0 - 3
    bitmask_clear(registers_.f, 0x0F);

    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.a);
    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.f);
    ++pc_;
    cycles = 16;
    break;
  case Instruction::push_bc:
    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.b);
    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.c);
    ++pc_;
    cycles = 16;
    break;
  case Instruction::push_de:
    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.d);
    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.e);
    ++pc_;
    cycles = 16;
    break;
  case Instruction::push_hl:
    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.h);
    sp_ -= 1;
    mmu_.WriteByte(sp_, registers_.l);
    ++pc_;
    cycles = 16;
    break;
  case Instruction::pop_af:
    registers_.f = mmu_.ReadByte(sp_);
    ++sp_;
    registers_.a = mmu_.ReadByte(sp_);
    ++sp_;
    // reset our flags_
    bit_check(registers_.f, 7) ? flags_.z = true : flags_.z = false;
    bit_check(registers_.f, 6) ? flags_.n = true : flags_.n = false;
    bit_check(registers_.f, 5) ? flags_.h = true : flags_.h = false;
    bit_check(registers_.f, 4) ? flags_.c = true : flags_.c = false;

    ++pc_;
    cycles = 12;
    break;
  case Instruction::pop_bc:
    registers_.c = mmu_.ReadByte(sp_);
    ++sp_;
    registers_.b = mmu_.ReadByte(sp_);
    ++sp_;
    ++pc_;
    cycles = 12;
    break;
  case Instruction::pop_de:
    registers_.e = mmu_.ReadByte(sp_);
    ++sp_;
    registers_.d = mmu_.ReadByte(sp_);
    ++sp_;
    ++pc_;
    cycles = 12;
    break;
  case Instruction::pop_hl:
    registers_.l = mmu_.ReadByte(sp_++);
    registers_.h = mmu_.ReadByte(sp_++);
    ++pc_;
    cycles = 12;
    break;
  case Instruction::ld_hl_sp_R8: {
    // Add the signed value R8 to SP and store the result in HL.
    int8_t offset = mmu_.ReadByte(pc_ + 1);
    registers_.hl = sp_ + offset;
    // carry and half carry are computed on the lower 8 bits
    // TODO: verify carry!
    (sp_ ^ offset ^ registers_.hl) & 0x10 ? flags_.h = true : flags_.h = false;
    (sp_ ^ offset ^ registers_.hl) & 0x100 ? flags_.c = true : flags_.c = false;
    flags_.z = false;
    flags_.n = false;
    pc_ += 2;
    cycles = 12;
    break;
  }
  case Instruction::ld_sp_hl:
    sp_ = registers_.hl;
    cycles = 8;
    ++pc_;
    break;
    // 8 bit ALU
  case Instruction::add_a_a:
    Add(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::add_a_b:
    Add(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::add_a_c:
    Add(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::add_a_d:
    Add(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::add_a_e:
    Add(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::add_a_h:
    Add(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::add_a_l:
    Add(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::add_a_hl:
    Add(mmu_.ReadByte(registers_.hl));
    ++pc_;
    cycles = 8;
    break;
  case Instruction::add_a_d8:
    Add(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::add_sp_r8: {
    int8_t value = mmu_.ReadByte(pc_ + 1);
    uint16_t res = sp_ + value;

    //(res & 0xFF) == 0 ? flags_.z = true : flags_.z = false;
    flags_.z = false;
    (sp_ ^ value ^ res) & 0x100 ? flags_.c = true : flags_.c = false; // bit 7
    (sp_ ^ value ^ res) & 0x10 ? flags_.h = true : flags_.h = false;  // bit 3
    flags_.n = false;

    sp_ = res;
    pc_ += 2;
    cycles = 16;
    break;
  }
  case Instruction::adc_a_a:
    Adc(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::adc_a_b:
    Adc(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::adc_a_c:
    Adc(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::adc_a_d:
    Adc(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::adc_a_e:
    Adc(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::adc_a_h:
    Adc(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::adc_a_hl:
    Adc(mmu_.ReadByte(registers_.hl));
    ++pc_;
    cycles = 8;
    break;
  case Instruction::adc_a_l:
    Adc(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::adc_a_d8: {
    Adc(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  }
  case Instruction::sbc_a_d8:
    Sbc(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::sbc_a_a:
    Sbc(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sbc_a_b:
    Sbc(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sbc_a_c:
    Sbc(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sbc_a_d:
    Sbc(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sbc_a_e:
    Sbc(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sbc_a_h:
    Sbc(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sbc_a_l:
    Sbc(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sbc_a_hl:
    Sbc(mmu_.ReadByte(registers_.hl));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::and_d8:
    And(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::and_a:
    And(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::and_b:
    And(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::and_c:
    And(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::and_d:
    And(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::and_e:
    And(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::and_h:
    And(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::and_l:
    And(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::and_hl:
    And(mmu_.ReadByte(registers_.hl));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::sub_d8:
    Sub(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::sub_a:
    Sub(registers_.a);
    // flags are a special case here
    flags_.c = false;
    flags_.h = false;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sub_b:
    Sub(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sub_c:
    Sub(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sub_d:
    Sub(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sub_e:
    Sub(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sub_h:
    Sub(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sub_l:
    Sub(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::sub_hl:
    Sub(mmu_.ReadByte(registers_.hl));
    ++pc_;
    cycles = 4;
    break;
  case Instruction::xor_a:
    Xor(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::xor_b:
    Xor(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::xor_c:
    Xor(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::xor_d: {
    Xor(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::xor_e: {
    Xor(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::xor_h: {
    Xor(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::xor_l: {
    Xor(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::xor_hl:
    Xor(mmu_.ReadByte(registers_.hl));
    ++pc_;
    cycles = 4;
    break;
  case Instruction::xor_d8:
    Xor(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::or_a:
    Or(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::or_b:
    Or(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::or_c:
    Or(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::or_d:
    Or(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::or_e:
    Or(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::or_h:
    Or(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::or_l:
    Or(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::or_hl:
    Or(mmu_.ReadByte(registers_.hl));
    ++pc_;
    cycles = 8;
    break;
  case Instruction::or_d8:
    Or(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::inc_a:
    Increment(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::inc_b:
    Increment(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::inc_c:
    Increment(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::inc_d:
    Increment(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::inc_e:
    Increment(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::inc_h:
    Increment(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::inc_l:
    Increment(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::inc_at_hl: {
    uint8_t val = mmu_.ReadByte(registers_.hl);
    Increment(val);
    mmu_.WriteByte(registers_.hl, val);
    ++pc_;
    cycles = 12;
    break;
  }
  case Instruction::dec_a: {
    uint8_t res = registers_.a - 1;

    res == 0 ? flags_.z = true : flags_.z = false;
    (registers_.a ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    registers_.a = res;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::dec_b: {
    uint8_t res = registers_.b - 1;

    res == 0 ? flags_.z = true : flags_.z = false;
    (registers_.b ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    registers_.b = res;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::dec_c: {
    uint8_t res = registers_.c - 1;

    res == 0 ? flags_.z = true : flags_.z = false;
    (registers_.c ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    registers_.c = res;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::dec_d: {
    uint8_t res = registers_.d - 1;

    res == 0 ? flags_.z = true : flags_.z = false;
    (registers_.d ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    registers_.d = res;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::dec_e: {
    uint8_t res = registers_.e - 1;

    res == 0 ? flags_.z = true : flags_.z = false;
    (registers_.e ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    registers_.e = res;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::dec_h: {
    uint8_t res = registers_.h - 1;

    res == 0 ? flags_.z = true : flags_.z = false;
    (registers_.h ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    registers_.h = res;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::dec_l: {
    uint8_t res = registers_.l - 1;

    res == 0 ? flags_.z = true : flags_.z = false;
    (registers_.l ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    registers_.l = res;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::dec_at_hl: {
    uint8_t val = mmu_.ReadByte(registers_.hl);
    uint8_t res = val - 1;
    mmu_.WriteByte(registers_.hl, res);
    res == 0 ? flags_.z = true : flags_.z = false;
    (val ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
    flags_.n = true;

    ++pc_;
    cycles = 12;
    break;
  }
  case Instruction::cp_a:
    Compare(registers_.a);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::cp_b:
    Compare(registers_.b);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::cp_c:
    Compare(registers_.c);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::cp_d:
    Compare(registers_.d);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::cp_e:
    Compare(registers_.e);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::cp_h:
    Compare(registers_.h);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::cp_l:
    Compare(registers_.l);
    ++pc_;
    cycles = 4;
    break;
  case Instruction::cp_d8:
    Compare(mmu_.ReadByte(pc_ + 1));
    pc_ += 2;
    cycles = 8;
    break;
  case Instruction::cp_hl:
    Compare(mmu_.ReadByte(registers_.hl));
    ++pc_;
    cycles = 8;
    break;
    // 16 bit arithmetic
  case Instruction::inc_bc:
    ++registers_.bc;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::inc_de:
    ++registers_.de;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::inc_hl:
    ++registers_.hl;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::inc_sp:
    ++sp_;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::dec_bc:
    --registers_.bc;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::dec_de:
    --registers_.de;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::dec_hl:
    --registers_.hl;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::dec_sp:
    --sp_;
    ++pc_;
    cycles = 8;
    break;
  case Instruction::add_hl_bc: {
    uint32_t res = registers_.hl + registers_.bc;

    res > UINT16_MAX ? flags_.c = true : flags_.c = false;
    (registers_.hl ^ registers_.bc ^ res) & 0x1000 ? flags_.h = true
                                                   : flags_.h = false;
    flags_.n = false;

    registers_.hl = static_cast<uint16_t>(res);
    ++pc_;
    cycles = 8;
    break;
  }
  case Instruction::add_hl_de: {
    uint32_t res = registers_.hl + registers_.de;

    res > UINT16_MAX ? flags_.c = true : flags_.c = false;
    (registers_.hl ^ registers_.de ^ res) & 0x1000 ? flags_.h = true
                                                   : flags_.h = false;
    flags_.n = false;

    registers_.hl = static_cast<uint16_t>(res);
    ++pc_;
    cycles = 8;
    break;
  }
  case Instruction::add_hl_hl: {
    uint32_t res = registers_.hl + registers_.hl;

    res > UINT16_MAX ? flags_.c = true : flags_.c = false;
    (registers_.hl ^ registers_.hl ^ res) & 0x1000 ? flags_.h = true
                                                   : flags_.h = false;
    flags_.n = false;

    registers_.hl = static_cast<uint16_t>(res);
    ++pc_;
    cycles = 8;
    break;
  }
  case Instruction::add_hl_sp: {
    uint32_t res = registers_.hl + sp_;

    res > UINT16_MAX ? flags_.c = true : flags_.c = false;
    (registers_.hl ^ sp_ ^ res) & 0x1000 ? flags_.h = true : flags_.h = false;
    flags_.n = false;

    registers_.hl = static_cast<uint16_t>(res);
    ++pc_;
    cycles = 8;
    break;
  }
    // Jumps
  case Instruction::jp_a16: {
    uint8_t c = mmu_.ReadByte(pc_ + 1);
    uint8_t d = mmu_.ReadByte(pc_ + 2);
    pc_ = (d << 8) + c;
    cycles = 16;
    break;
  }
  case Instruction::jp_z_a16: {
    if (flags_.z) {
      uint8_t c = mmu_.ReadByte(pc_ + 1);
      uint8_t d = mmu_.ReadByte(pc_ + 2);
      pc_ = (d << 8) + c;
      cycles = 16;
    } else {
      pc_ += 3;
      cycles = 12;
    }
    break;
  }
  case Instruction::jp_hl:
    pc_ = registers_.hl;
    cycles = 16;
    break;
  case Instruction::jp_c_a16:
    if (!flags_.c) {
      pc_ += 3;
      cycles = 12;
    } else {
      pc_ = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
      cycles = 16;
    }
    break;
  case Instruction::jp_nc_a16:
    if (flags_.c) {
      pc_ += 3;
      cycles = 12;
    } else {
      pc_ = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
      cycles = 16;
    }
    break;
  case Instruction::jp_nz_a16:
    if (flags_.z) {
      pc_ += 3;
      cycles = 12;
    } else {
      pc_ = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
      cycles = 16;
    }
    break;
  case Instruction::jr_r8:
    // must jump from the address of the NEXT instruction (ie 2 bytes after
    // this one)
    pc_ += (int8_t)(mmu_.ReadByte(pc_ + 1) + 2);
    cycles = 12;
    break;
  case Instruction::jr_c_r8:
    if (flags_.c) {
      pc_ += (int8_t)(mmu_.ReadByte(pc_ + 1) + 2);
      cycles = 12;
    } else {
      pc_ += 2;
      cycles = 8;
    }
    break;
  case Instruction::jr_nc_r8:
    if (flags_.c) {
      pc_ += 2;
      cycles = 8;
    } else {
      auto offset = (int8_t)mmu_.ReadByte(pc_ + 1);
      pc_ += (2 + offset);
      cycles = 12;
    }
    break;
  case Instruction::jr_nz_r8:
    if (flags_.z) {
      pc_ += 2;
      cycles = 8;
    } else {
      auto offset = (int8_t)mmu_.ReadByte(pc_ + 1);
      pc_ += (2 + offset);
      cycles = 12;
    }
    break;
  case Instruction::jr_z_r8:
    if (flags_.z) {
      auto offset = (int8_t)mmu_.ReadByte(pc_ + 1);
      pc_ += (2 + offset);
      cycles = 12;
    } else {
      pc_ += 2;
      cycles = 8;
    }
    break;
    // Rotates and shifts
  case Instruction::rla: {
    rl(registers_.a);
    // this instruction has a hardware bug
    flags_.z = false;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::rra: {
    rr(registers_.a);
    // hardware bug
    flags_.z = false;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::rrca: {
    uint8_t bit = bit_check(registers_.a, 0);
    registers_.a >>= 1;
    registers_.a ^= (-bit ^ registers_.a) & (1U << 7);
    // flags_.c = bit;
    bit == 0 ? flags_.c = false : flags_.c = true;
    // unsure whether Z flag should be modified here
    flags_.z = false;
    flags_.n = false;
    flags_.h = false;
    ++pc_;
    cycles = 4;
    break;
  }
  case Instruction::rlca: {
    uint8_t bit = bit_check(registers_.a, 7);
    registers_.a <<= 1;
    registers_.a ^= (-bit ^ registers_.a) & (1U << 0);
    // flags_.c = bit;
    bit == 0 ? flags_.c = false : flags_.c = true;
    // unsure whether Z flag should be modified here
    flags_.z = false;
    flags_.n = false;
    flags_.h = false;
    ++pc_;
    cycles = 4;
    break;
  }
    // Calls
  case Instruction::call_a16:
    mmu_.WriteByte(--sp_, (pc_ + 3) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 3);
    pc_ = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
    cycles = 12;
    break;
  case Instruction::call_nz_a16:
    if (!flags_.z) {
      uint16_t address = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
      mmu_.WriteByte(--sp_, (pc_ + 3) >> 8);
      mmu_.WriteByte(--sp_, pc_ + 3);
      cycles = 24;
      pc_ = address;
    } else {
      pc_ += 3;
      cycles = 12;
    }
    break;
  case Instruction::call_z_a16:
    if (flags_.z) {
      uint16_t address = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
      mmu_.WriteByte(--sp_, (pc_ + 3) >> 8);
      mmu_.WriteByte(--sp_, pc_ + 3);
      cycles = 24;
      pc_ = address;
    } else {
      pc_ += 3;
      cycles = 12;
    }
    break;
  case Instruction::call_nc_a16:
    if (!flags_.c) {
      uint16_t address = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
      mmu_.WriteByte(--sp_, (pc_ + 3) >> 8);
      mmu_.WriteByte(--sp_, pc_ + 3);
      cycles = 24;
      pc_ = address;
    } else {
      pc_ += 3;
      cycles = 12;
    }
    break;
  case Instruction::call_c_a16:
    if (flags_.c) {
      uint16_t address = (mmu_.ReadByte(pc_ + 2) << 8) + mmu_.ReadByte(pc_ + 1);
      mmu_.WriteByte(--sp_, (pc_ + 3) >> 8);
      mmu_.WriteByte(--sp_, pc_ + 3);
      cycles = 24;
      pc_ = address;
    } else {
      pc_ += 3;
      cycles = 12;
    }
    break;
    // Restarts
  case Instruction::rst_00h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0000;
    cycles = 16;
    break;
  case Instruction::rst_08h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0008;
    cycles = 16;
    break;
  case Instruction::rst_10h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0010;
    cycles = 16;
    break;
  case Instruction::rst_18h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0018;
    cycles = 16;
    break;
  case Instruction::rst_20h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0020;
    cycles = 16;
    break;
  case Instruction::rst_28h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0028;
    cycles = 16;
    break;
  case Instruction::rst_30h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0030;
    cycles = 16;
    break;
  case Instruction::rst_38h:
    mmu_.WriteByte(--sp_, (pc_ + 1) >> 8);
    mmu_.WriteByte(--sp_, pc_ + 1);
    pc_ = 0x0038;
    cycles = 16;
    break;
    // Returns
  case Instruction::ret:
    pc_ = (mmu_.ReadByte(sp_ + 1) << 8) + mmu_.ReadByte(sp_);
    sp_ += 2;
    cycles = 8;
    break;
  case Instruction::reti:
    pc_ = (mmu_.ReadByte(sp_ + 1) << 8) + mmu_.ReadByte(sp_);
    ime_ = true;
    sp_ += 2;
    cycles = 16;
    break;
  case Instruction::ret_z:
    if (flags_.z) {
      pc_ = (mmu_.ReadByte(sp_ + 1) << 8) + mmu_.ReadByte(sp_);
      sp_ += 2;
      cycles = 20;
    } else {
      ++pc_;
      cycles = 8;
    }
    break;
  case Instruction::ret_nz:
    if (!flags_.z) {
      pc_ = (mmu_.ReadByte(sp_ + 1) << 8) + mmu_.ReadByte(sp_);
      sp_ += 2;
      cycles = 20;
    } else {
      ++pc_;
      cycles = 8;
    }
    break;
  case Instruction::ret_c:
    if (flags_.c) {
      pc_ = (mmu_.ReadByte(sp_ + 1) << 8) + mmu_.ReadByte(sp_);
      sp_ += 2;
      cycles = 20;
    } else {
      ++pc_;
      cycles = 8;
    }
    break;
  case Instruction::ret_nc:
    if (!flags_.c) {
      pc_ = (mmu_.ReadByte(sp_ + 1) << 8) + mmu_.ReadByte(sp_);
      sp_ += 2;
      cycles = 20;
    } else {
      ++pc_;
      cycles = 8;
    }
    break;
    // Misc
  case Instruction::daa:
    //  Decimal adjust register A to get a correct BCD representation after an
    //  arithmetic instruction.
    if (!flags_.n) {
      if (flags_.c || registers_.a > 0x99) {
        registers_.a += 0x60;
        flags_.c = true;
      }
      if (flags_.h || (registers_.a & 0x0f) > 0x09) {
        registers_.a += 0x06;
      }
    } else {
      if (flags_.c) {
        registers_.a -= 0x60;
      }
      if (flags_.h) {
        registers_.a -= 0x06;
      }
    }

    registers_.a == 0 ? flags_.z = true : flags_.z = false;
    flags_.h = false;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ld_a_at_c:
    registers_.a = mmu_.ReadByte(0xFF00 + registers_.c);
    ++pc_;
    cycles = 8;
    break;
  case Instruction::cpl:
    registers_.a = ~registers_.a;
    flags_.n = true;
    flags_.h = true;
    ++pc_;
    break;
  case Instruction::scf:
    flags_.c = true;
    flags_.n = false;
    flags_.h = false;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ccf:
    flags_.c = !flags_.c;
    flags_.n = false;
    flags_.h = false;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::di:
    // TODO: disable interrupts after the next instruction
    ime_ = false;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::ei:
    // TODO: enable interrupts after the next instruction
    ime_ = true;
    ++pc_;
    cycles = 4;
    break;
  case Instruction::halt: {
    // suspend and wait for interrupts
    // TODO - Fix / Actually implement
    halted_ = true;
    uint8_t ie = mmu_.ReadByte(IE);
    uint8_t int_flag = mmu_.ReadByte(IF);
    // enter halt mode normally
    if (ime_ && ((ie & int_flag & 0x1f) != 0)) {
      halted_ = false;
      // push address of next instruction to the stack in interrupt handler
      ++pc_;
      // HandleInterrupts(); // or return?
    } else if (!ime_ && ((ie & int_flag & 0x1f) == 0)) {
      halted_ = false;
      ++pc_;
    } else if (!ime_ && ((ie & int_flag & 0x1f) != 0)) {
      // HALT bug basically, execute the next instruction twice
      spdlog::get("stdout")->debug("HALT bug");
      halt_bug_occurred_ = true;
      ++pc_;
    }
    cycles = 4;
    break;
  }
  default:
    spdlog::get("stdout")->error("Unknown opcode 0x{0:02x} at PC 0x{1:04x}",
                                 (uint8_t)opcode, pc_);
    // printf("Unknown opcode 0x%0.2X at PC 0x%0.4X\n", opcode, pc);
    // std::cerr << "Unknown Opcode: " << std::hex << opcode << std::dec <<
    // std::endl;
    break;
  }

  // Update flags register
  flags_.z ? bit_set(registers_.f, 7) : bit_clear(registers_.f, 7);
  flags_.n ? bit_set(registers_.f, 6) : bit_clear(registers_.f, 6);
  flags_.h ? bit_set(registers_.f, 5) : bit_clear(registers_.f, 5);
  flags_.c ? bit_set(registers_.f, 4) : bit_clear(registers_.f, 4);

  // deal with halt bug
  if (halt_bug_occurred_ && (Instruction)decoded.opcode != Instruction::halt) {
    // fail to increase pc
    pc_ -= decoded.length;
    halt_bug_occurred_ = false;
  }

  return decoded;
}

constexpr void CPU::Adc(const uint8_t operand) {
  const uint16_t res = registers_.a + operand + flags_.c;

  (res & 0xFF) == 0 ? flags_.z = true : flags_.z = false;
  res > UINT8_MAX ? flags_.c = true : flags_.c = false;
  (registers_.a ^ operand ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
  flags_.n = false;

  registers_.a = static_cast<uint8_t>(res);
}

constexpr void CPU::Sub(const uint8_t operand) {
  const uint8_t res = registers_.a - operand;

  res == 0 ? flags_.z = true : flags_.z = false;
  operand > registers_.a ? flags_.c = true : flags_.c = false;
  (registers_.a ^ operand ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
  flags_.n = true;

  registers_.a = res;
}

constexpr void CPU::Sbc(const uint8_t operand) {
  const uint8_t res = registers_.a - operand - flags_.c;

  res == 0 ? flags_.z = true : flags_.z = false;
  operand + flags_.c > registers_.a ? flags_.c = true : flags_.c = false;
  (registers_.a ^ operand ^ res) & 0x10 ? flags_.h = true : flags_.h = false;

  flags_.n = true;

  registers_.a = res;
}

constexpr void CPU::And(const uint8_t operand) {
  registers_.a &= operand;
  registers_.a == 0 ? flags_.z = true : flags_.z = false;
  flags_.n = false;
  flags_.c = false;
  flags_.h = true;
}

constexpr void CPU::Xor(const uint8_t operand) {
  registers_.a ^= operand;
  registers_.a == 0 ? flags_.z = true : flags_.z = false;
  flags_.c = false;
  flags_.h = false;
  flags_.n = false;
}

constexpr void CPU::Or(const uint8_t operand) {
  registers_.a |= operand;
  registers_.a == 0 ? flags_.z = true : flags_.z = false;
  flags_.c = false;
  flags_.h = false;
  flags_.n = false;
}

constexpr void CPU::Compare(const uint8_t operand) {
  const uint8_t res = registers_.a - operand;

  res == 0 ? flags_.z = true : flags_.z = false;
  (registers_.a ^ operand ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
  operand > registers_.a ? flags_.c = true : flags_.c = false;
  flags_.n = true;
}

constexpr void CPU::Increment(uint8_t &operand) {
  const uint8_t res = operand + 1;

  res == 0 ? flags_.z = true : flags_.z = false;
  (operand ^ 1 ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
  flags_.n = false;

  operand = res;
}

constexpr void CPU::Add(const uint8_t operand) {
  const uint16_t res = registers_.a + operand;

  // test only the lower 8 bits, otherwise we'll get a wrong result if
  // there's a carry. This applies to every instruction we used a 16bit
  // container for the 8 bit result
  (res & 0xFF) == 0 ? flags_.z = true : flags_.z = false;
  res > UINT8_MAX ? flags_.c = true : flags_.c = false;
  (registers_.a ^ operand ^ res) & 0x10 ? flags_.h = true : flags_.h = false;
  flags_.n = false;

  registers_.a = static_cast<uint8_t>(res);
}
constexpr void CPU::rlc(uint8_t &reg) {
  const uint8_t bit = bit_check(reg, 7);
  reg <<= 1;
  // put the bit we shifted out back in the 0th position
  reg ^= (-bit ^ reg) & (1U << 0);
  // carry flag holds old bit 7
  flags_.c = bit;
  // update Z flag
  reg == 0 ? flags_.z = true : flags_.z = false;
  // reset subtract flag
  flags_.n = false;
  // reset half carry flag
  flags_.h = false;
}

constexpr void CPU::rrc(uint8_t &reg) {
  const uint8_t bit = bit_check(reg, 0);
  reg >>= 1;
  // return the old 0th back to bit 7, and put our new bit in the carry
  bit == 1U ? bit_set(reg, 7) : bit_clear(reg, 7);
  // reg == 0 ? flags_.z = true : flags_.z = false;
  flags_.z = !reg;
  flags_.c = bit;
  flags_.h = false;
  flags_.n = false;
}

/*     -----------------------v
                 ^-- CY <-- [7 <-- 0] <---
*/
constexpr void CPU::rl(uint8_t &reg) {
  const uint8_t bit = bit_check(reg, 7);
  reg <<= 1;
  // put the previous carry back in the 0th position
  reg ^= (-static_cast<uint8_t>(flags_.c) ^ reg) & (1U << 0);
  // carry flag holds old bit 7
  flags_.c = bit;
  // bit == 1U ? flags_.c = true : flags_.c = false;
  // update Z flag
  reg == 0 ? flags_.z = true : flags_.z = false;
  // reset subtract flag
  flags_.n = false;
  // reset half carry flag
  flags_.h = false;
}

constexpr void CPU::srl(uint8_t &reg) {
  // shift right into carry
  const uint8_t bit = bit_check(reg, 0);
  reg >>= 1;
  bit == 0 ? flags_.c = false : flags_.c = true;
  reg == 0 ? flags_.z = true : flags_.z = false;
  flags_.h = false;
  flags_.n = false;
}

/*	  *	v------------------------
** RR * --> CY --> [7 --> 0] ---^
*/
constexpr void CPU::rr(uint8_t &reg) {
  const uint8_t bit = bit_check(reg, 0);
  reg >>= 1;
  // return the old carry back to bit 7, and put our new bit in the carry
  flags_.c ? bit_set(reg, 7) : bit_clear(reg, 7);
  reg == 0 ? flags_.z = true : flags_.z = false;
  // update carry with the new bit
  bit == 0 ? flags_.c = false : flags_.c = true;
  flags_.h = false;
  flags_.n = false;
}

constexpr void CPU::sla(uint8_t &reg) {
  flags_.c = bit_check(reg, 7);
  reg <<= 1;
  reg == 0 ? flags_.z = true : flags_.z = false;
  flags_.h = false;
  flags_.n = false;
}

constexpr void CPU::sra(uint8_t &reg) {
  const uint8_t bit = bit_check(reg, 7);
  flags_.c = bit_check(reg, 0);
  reg >>= 1;
  reg ^= (-bit ^ reg) & (1U << 7);
  reg == 0 ? flags_.z = true : flags_.z = false;
  flags_.h = false;
  flags_.n = false;
}

constexpr void CPU::swap(uint8_t &reg) {
  uint8_t temp = reg;
  reg <<= 4;
  temp >>= 4;
  reg &= 0xF0;
  temp &= 0x0F;
  reg |= temp;
  reg == 0 ? flags_.z = true : flags_.z = false;
  flags_.n = false;
  flags_.h = false;
  flags_.c = false;
}
