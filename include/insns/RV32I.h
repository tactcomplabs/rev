//
// _RV32I_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32I_H_
#define _SST_REVCPU_RV32I_H_

#include "../RevInstTable.h"
#include "../RevExt.h"

#include <vector>
#include <functional>
#include <type_traits>

namespace SST::RevCPU{

class RV32I : public RevExt {

  // Compressed instructions
  static bool caddi4spn(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.addi4spn rd, $imm == addi rd, x2, $imm
    Inst.rs1  = 2;
    Inst.rd   = CRegIdx(Inst.rd);

    // if Inst.imm == 0; this is a HINT instruction
    // this is effectively a NOP
    if( Inst.imm == 0x00 ){
      R->AdvancePC(F, Inst.instSize);
      return true;
    }
    //Inst.imm = (Inst.imm & 0b011111111)*4;
    Inst.imm = (Inst.imm & 0b11111111)*4;
    return addi(F, R, M, Inst);
  }

  static bool clwsp(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.lwsp rd, $imm = lw rd, x2, $imm
    Inst.rs1  = 2;
    //Inst.imm = ((Inst.imm & 0b111111)*4);
    Inst.imm = (Inst.imm & 0b11111111); // Immd is 8 bits -  bits placed correctly in decode, no need to scale

    return lw(F, R, M, Inst);
  }

  static bool cswsp(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.swsp rs2, $imm = sw rs2, x2, $imm
    Inst.rs1  = 2;
    //Inst.imm = ((Inst.imm & 0b111111)*4);
    Inst.imm = (Inst.imm & 0b11111111); // Immd is 8 bits - zero extended, bits placed correctly in decode, no need to scale

    return sw(F, R, M, Inst);
  }

  static bool clw(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.lw rd, rs1, $imm = lw rd, $imm(rs1)
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = CRegIdx(Inst.rs1);
    //Inst.imm = ((Inst.imm & 0b11111)*4);
    Inst.imm = (Inst.imm & 0b1111111); // Immd is 7 bits, zero extended, bits placed correctly in decode, no need to scale

    return lw(F, R, M, Inst);
  }

  static bool csw(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.sw rs2, rs1, $imm = sw rs2, $imm(rs1)
    Inst.rs2 = CRegIdx(Inst.rd);
    Inst.rs1 = CRegIdx(Inst.rs1);
    //Inst.imm = ((Inst.imm & 0b11111)*4);
    Inst.imm = (Inst.imm & 0b1111111); //Immd is 7-bits, zero extended, bits placed correctly in decode, no need to scale

    return sw(F, R, M, Inst);
  }

  static bool cj(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.j $imm = jal x0, $imm
    Inst.rd = 0; // x0

    Inst.imm = Inst.jumpTarget;
    Inst.imm = Inst.ImmSignExt(12);
    return jal(F, R, M, Inst);
  }

  static bool cjal(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.jal $imm = jal x0, $imm
    Inst.rd = 1; // x1
    Inst.imm = Inst.jumpTarget;

    return jal(F, R, M, Inst);
  }

  static bool CRFUNC_1000(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst){
    if( Inst.rs2 != 0 ){
      return cmv(F, R, M, Inst);
    }
    return cjr(F, R, M, Inst);
  }

  static bool CRFUNC_1001(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst){
    if( (Inst.rs1 == 0) && (Inst.rd == 0) ){
      return ebreak(F, R, M, Inst);
    }else if( (Inst.rs2 == 0) && (Inst.rd != 0) ){
      Inst.rd = 1;  //C.JALR expands to jalr x1, 0(rs1), so force update of x1 / ra
      return jalr(F, R, M, Inst);
    }else{
      return add(F, R, M, Inst);
    }
  }

  static bool cjr(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.jr %rs1 = jalr x0, 0(%rs1)
    Inst.rs2 = 0;
    return jalr(F, R, M, Inst);
  }

  static bool cmv(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    Inst.rs1 = 0;  // expands to add rd, x0, rs2, so force rs1 to zero
    return add(F, R, M, Inst);
  }

  static bool cadd(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    Inst.rs1 = Inst.rd;
    return add(F, R, M, Inst);
  }

  static bool cjalr(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.jalr %rs1 = jalr x1, 0(%rs1)
    Inst.rs2 = 1;
    return jalr(F, R, M, Inst);
  }

  static bool cbeqz(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.beqz %rs1, $imm = beq %rs1, x0, $imm
    Inst.rs2 = 0;
    Inst.rs1 = CRegIdx(Inst.rs1);
    Inst.imm = Inst.offset;
    Inst.imm = Inst.ImmSignExt(9);
    //Inst.imm = Inst.offset & 0b111111;
    //SEXT(Inst.imm, Inst.offset&0b111111111, 9); //Immd is signed 9-bit, scaled in decode
    //SEXT(Inst.imm, Inst.offset, 6);

    return beq(F, R, M, Inst);
  }

  static bool cbnez(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.bnez %rs1, $imm = bne %rs1, x0, $imm
    Inst.rs2 = 0;
    Inst.rs1 = CRegIdx(Inst.rs1);
    Inst.imm = Inst.offset;
    Inst.imm = Inst.ImmSignExt(9);  //Immd is signed 9-bit, scaled in decode
    //Inst.imm = Inst.offset & 0b111111;
    //SEXT(Inst.imm, Inst.offset, 6);
    //SEXT(Inst.imm, Inst.offset&0b111111111, 9); //Immd is signed 9-bit, scaled in decode

    return bne(F, R, M, Inst);
  }

  static bool cli(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.li %rd, $imm = addi %rd, x0, $imm
    Inst.rs1 = 0;
    // SEXT(Inst.imm, (Inst.imm & 0b111111), 6);
    Inst.imm = Inst.ImmSignExt(6);
    return addi(F, R, M, Inst);
  }

  static bool CIFUNC(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    if( Inst.rd == 2 ){
      // c.addi16sp
      //SEXT(Inst.imm, (Inst.imm & 0b011111111)*16, 32);
      //SEXT(Inst.imm, (Inst.imm & 0b111111)*16, 6);
      // SEXT(Inst.imm, (Inst.imm & 0b1111111111), 10); // Immd is 10 bits, sign extended and scaled in decode
      Inst.imm = Inst.ImmSignExt(10);
      return addi(F, R, M, Inst);
    }else{
      // c.lui %rd, $imm = addi %rd, x0, $imm
      Inst.imm = Inst.ImmSignExt(6);
      return lui(F, R, M, Inst);
    }
  }

  static bool caddi(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.addi %rd, $imm = addi %rd, %rd, $imm
    // uint32_t tmp = Inst.imm & 0b111111;
    Inst.imm = Inst.ImmSignExt(6);
    Inst.rs1 = Inst.rd;
    return addi(F, R, M, Inst);
  }

  static bool cslli(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.slli %rd, $imm = slli %rd, %rd, $imm
    Inst.rs1 = Inst.rd;
    return slli(F, R, M, Inst);
  }

  static bool csrli(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.srli %rd, $imm = srli %rd, %rd, $imm
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = Inst.rd;
    return srli(F, R, M, Inst);
  }

  static bool csrai(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.srai %rd, $imm = srai %rd, %rd, $imm
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = Inst.rd;
    return srai(F, R, M, Inst);
  }

  static bool candi(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.andi %rd, $imm = sandi %rd, %rd, $imm
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = Inst.rd;
    Inst.imm = Inst.ImmSignExt(6);  //immd is 6 bits, sign extended no scaling needed
    return andi(F, R, M, Inst);
  }

  static bool cand(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.and %rd, %rs2 = and %rd, %rd, %rs2
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = Inst.rd;
    Inst.rs2 = CRegIdx(Inst.rs2);
    return f_and(F, R, M, Inst);
  }

  static bool cor(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.or %rd, %rs2 = or %rd, %rd, %rs2
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = Inst.rd;
    Inst.rs2 = CRegIdx(Inst.rs2);
    return f_or(F, R, M, Inst);
  }

  static bool cxor(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.xor %rd, %rs2 = xor %rd, %rd, %rs2
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = Inst.rd;
    Inst.rs2 = CRegIdx(Inst.rs2);
    return f_xor(F, R, M, Inst);
  }

  static bool csub(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    // c.sub %rd, %rs2 = sub %rd, %rd, %rs2
    Inst.rd  = CRegIdx(Inst.rd);
    Inst.rs1 = Inst.rd;
    Inst.rs2  = CRegIdx(Inst.rs2);
    return sub(F, R, M, Inst);
  }

  // Standard instructions
  static bool lui(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->SetX(F, Inst.rd, static_cast<int32_t>(Inst.imm << 12));
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool auipc(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    auto ui = static_cast<int32_t>(Inst.imm << 12);
    if( F->IsRV32() ){
      R->SetX(F, Inst.rd, ui + R->RV32_PC);
    }else{
      R->SetX(F, Inst.rd, ui + R->RV64_PC);
    }
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool jal(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    if( F->IsRV32() ){
      R->SetX(F, Inst.rd, R->RV32_PC + Inst.instSize);
    }else{
      R->SetX(F, Inst.rd, R->RV64_PC + Inst.instSize);
    }
    R->AdvancePC(F, Inst.ImmSignExt(20));
    return true;
  }

  static bool jalr(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    if( F->IsRV32() ){
      auto ret = R->RV32_PC + Inst.instSize;
      R->RV32_PC = (R->GetX<uint32_t>(F, Inst.rs1) + Inst.ImmSignExt(12)) & ~uint32_t{1};
      R->SetX(F, Inst.rd, ret);
    }else{
      auto ret = R->RV64_PC + Inst.instSize;
      R->RV64_PC = (R->GetX<uint64_t>(F, Inst.rs1) + Inst.ImmSignExt(12)) & ~uint64_t{1};
      R->SetX(F, Inst.rd, ret);
    }
    return true;
  }

  // Conditional branch template
  // The first template parameter is the comparison functor
  // The second template parameter is std::make_signed_t or std::make_unsigned_t
  template<template<class> class OP, template<class> class SIGN = std::make_unsigned_t>
  static bool bcond(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    bool cond;
    if( F->IsRV32() ){
      cond = OP()(R->GetX<SIGN<int32_t>>(F, Inst.rs1), R->GetX<SIGN<int32_t>>(F, Inst.rs2));
    }else{
      cond = OP()(R->GetX<SIGN<int64_t>>(F, Inst.rs1), R->GetX<SIGN<int64_t>>(F, Inst.rs2));
    }
    R->AdvancePC(F, cond ? Inst.ImmSignExt(13) : Inst.instSize);
    return true;
  }

  // Conditional branches
  static constexpr auto& beq  = bcond<std::equal_to>;
  static constexpr auto& bne  = bcond<std::not_equal_to>;
  static constexpr auto& blt  = bcond<std::less,          std::make_signed_t>;
  static constexpr auto& bltu = bcond<std::less,          std::make_unsigned_t>;
  static constexpr auto& bge  = bcond<std::greater_equal, std::make_signed_t>;
  static constexpr auto& bgeu = bcond<std::greater_equal, std::make_unsigned_t>;

  // Loads
  static constexpr auto& lb  = load<int8_t>;
  static constexpr auto& lh  = load<int16_t>;
  static constexpr auto& lw  = load<int32_t>;
  static constexpr auto& lbu = load<uint8_t>;
  static constexpr auto& lhu = load<uint16_t>;

  // Stores
  static constexpr auto& sb  = store<uint8_t>;
  static constexpr auto& sh  = store<uint16_t>;
  static constexpr auto& sw  = store<uint32_t>;

  // Arithmetic operators
  static constexpr auto& add   = oper<std::plus,    OpKind::Reg>;
  static constexpr auto& addi  = oper<std::plus,    OpKind::Imm>;
  static constexpr auto& sub   = oper<std::minus,   OpKind::Reg>;
  static constexpr auto& f_xor = oper<std::bit_xor, OpKind::Reg>;
  static constexpr auto& xori  = oper<std::bit_xor, OpKind::Imm>;
  static constexpr auto& f_or  = oper<std::bit_or,  OpKind::Reg>;
  static constexpr auto& ori   = oper<std::bit_or,  OpKind::Imm>;
  static constexpr auto& f_and = oper<std::bit_and, OpKind::Reg>;
  static constexpr auto& andi  = oper<std::bit_and, OpKind::Imm>;

  // Boolean test and set operators
  static constexpr auto& slt   = oper<std::less,    OpKind::Reg>;
  static constexpr auto& slti  = oper<std::less,    OpKind::Imm>;
  static constexpr auto& sltu  = oper<std::less,    OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& sltiu = oper<std::less,    OpKind::Imm, std::make_unsigned_t>;

  // Shift operators
  static constexpr auto& slli = oper<ShiftLeft,     OpKind::Imm, std::make_unsigned_t>;
  static constexpr auto& srli = oper<ShiftRight,    OpKind::Imm, std::make_unsigned_t>;
  static constexpr auto& srai = oper<ShiftRight,    OpKind::Imm>;
  static constexpr auto& sll  = oper<ShiftLeft,     OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& srl  = oper<ShiftRight,    OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& sra  = oper<ShiftRight,    OpKind::Reg>;

  static bool fence(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    M->FenceMem(F->GetHart());
    R->AdvancePC(F, Inst.instSize);
    return true;  // temporarily disabled
  }

  static bool fencei(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    M->FenceMem(F->GetHart());
    R->AdvancePC(F, Inst.instSize);
    return true;  // temporarily disabled
  }

  static bool ecall(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst){
    // Save PC of Ecall to *epc register
    if( F->IsRV32() ){
      R->RV32_SEPC = R->RV32_PC; // Save PC of instruction that raised exception
      R->RV32_STVAL = 0; // MTVAL/STVAL unused for ecall and is set to 0
      R->RV32_SCAUSE = EXCEPTION_CAUSE::ECALL_USER_MODE; // MTVAL/STVAL unused for ecall and is set to 0
    } else {
      /*
       * In reality this should be getting/setting a LOT of bits inside the
       * CSRs however because we are only concerned with ecall right now it's
       * not a concern.
       * NOTE: Normally you would have to check if you are currently executing in
       *       Supervisor mode already and set RV64_MEPC instead but we don't need
       *       to worry about machine mode with the ecalls we are supporting
       */
      // R->RV64_PC += Inst.instSize; // TODO: Verify this needs to happen
      R->RV64_SEPC = R->RV64_PC; // Save PC of instruction that raised exception
      R->RV64_STVAL = 0; // MTVAL/STVAL unused for ecall and is set to 0
      R->RV64_SCAUSE = EXCEPTION_CAUSE::ECALL_USER_MODE; // MTVAL/STVAL unused for ecall and is set to 0
      /*
       * Trap Handler is not implemented because we only have one exception
       * So we don't have to worry about setting `mtvec` reg
       */
    }
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool ebreak(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool csrrw(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool csrrs(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool csrrc(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool csrrwi(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool csrrsi(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  static bool csrrci(RevFeature *F, RevRegFile *R, RevMem *M, RevInst Inst) {
    R->AdvancePC(F, Inst.instSize);
    return true;
  }

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32I Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  std::vector<RevInstEntry> RV32ITable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lui %rd, $imm"  ).SetCost(1).SetOpcode(0b0110111).SetFunct3(0b0).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeU).SetImplFunc(&lui ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("auipc %rd, $imm").SetCost(1).SetOpcode(0b0010111).SetFunct3(0b0).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeU).SetImplFunc(&auipc ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("jal %rd, $imm"       ).SetCost(1).SetOpcode(0b1101111).SetFunct3(0b0  ).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeJ).SetImplFunc(&jal ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("jalr %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1100111).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&jalr ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("beq %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&beq ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bne %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bne ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("blt %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b100).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&blt ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bge %rs1, %rs2, $imm" ).SetCost(1).SetOpcode(0b1100011).SetFunct3(0b101).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bge ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bltu %rs1, %rs2, $imm").SetCost(1).SetOpcode(0b1100011).SetFunct3(0b110).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bltu ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("bgeu %rs1, %rs2, $imm").SetCost(1).SetOpcode(0b1100011).SetFunct3(0b111).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(&bgeu ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lb %rd, $imm(%rs1)" ).SetCost(1).SetOpcode(0b0000011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lb ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lh %rd, $imm(%rs1)" ).SetCost(1).SetOpcode(0b0000011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lh ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lw %rd, $imm(%rs1)" ).SetCost(1).SetOpcode(0b0000011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lbu %rd, $imm(%rs1)").SetCost(1).SetOpcode(0b0000011).SetFunct3(0b100).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lbu ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("lhu %rd, $imm(%rs1)").SetCost(1).SetOpcode(0b0000011).SetFunct3(0b101).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&lhu ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sb %rs2, $imm(%rs1)").SetCost(1).SetOpcode(0b0100011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&sb ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sh %rs2, $imm(%rs1)").SetCost(1).SetOpcode(0b0100011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&sh ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sw %rs2, $imm(%rs1)").SetCost(1).SetOpcode(0b0100011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegIMM).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(&sw ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("addi %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&addi ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slti %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&slti ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sltiu %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b011).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&sltiu ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("xori %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b100).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&xori ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("ori %rd, %rs1, $imm"  ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b110).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&ori ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("andi %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b0010011).SetFunct3(0b111).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&andi ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slli %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&slli ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srli %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&srli ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srai %rd, %rs1, $imm").SetCost(1).SetOpcode(0b0010011).SetFunct3(0b101).SetFunct7(0b0010000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(&srai ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("add %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&add ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sub %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sub ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sll %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b001).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sll ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("slt %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b010).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&slt ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sltu %rd, %rs1, %rs2").SetCost(1).SetOpcode(0b0110011).SetFunct3(0b011).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sltu ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("xor %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b100).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&f_xor ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("srl %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b101).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&srl ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("sra %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b101).SetFunct7(0b0100000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&sra ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("or %rd, %rs1, %rs2"  ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b110).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&f_or ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("and %rd, %rs1, %rs2" ).SetCost(1).SetOpcode(0b0110011).SetFunct3(0b111).SetFunct7(0b0000000).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(&f_and ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("fence"  ).SetCost(1).SetOpcode(0b0001111).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(&fence ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("fence.i").SetCost(1).SetOpcode(0b0001111).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeI).SetImplFunc(&fencei ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("ecall" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b000000000000).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(&ecall ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("ebreak").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b000).SetFunct7(0b0).SetrdClass(RegUNKNOWN).Setrs1Class(RegUNKNOWN).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b000000000001).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(&ebreak ).InstEntry},

    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrw %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b001).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrs %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b010).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrs ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrc %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b011).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrc ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrwi %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b101).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrwi ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrsi %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b110).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrsi ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrci %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b111).SetFunct7(0b0).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setrs2Class(RegUNKNOWN).Setrs3Class(RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrci ).InstEntry},
  };

  // RV32C table
  std::vector<RevInstEntry> RV32ICTable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.addi4spn %rd, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b000).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCIW).SetImplFunc(&caddi4spn).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.lwsp %rd, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b010).SetrdClass(RegGPR).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&clwsp).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.swsp %rs2, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b110).Setrs2Class(RegGPR).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCSS).SetImplFunc(&cswsp).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.lw %rd, $rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b010).Setrs1Class(RegGPR).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCL).SetImplFunc(&clw).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.sw %rs2, %rs1, $imm").SetCost(1).SetOpcode(0b00).SetFunct3(0b110).Setrs1Class(RegGPR).Setrs2Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCS).SetImplFunc(&csw).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.j $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b101).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCJ).SetImplFunc(&cj).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.jr %rs1").SetCost(1).SetOpcode(0b10).SetFunct4(0b1000).Setrs1Class(RegGPR).SetFormat(RVCTypeCR).SetImplFunc(&CRFUNC_1000).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.jalr %rs1").SetCost(1).SetOpcode(0b10).SetFunct4(0b1001).Setrs1Class(RegGPR).SetFormat(RVCTypeCR).SetImplFunc(&CRFUNC_1001).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.beqz %rs1, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b110).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(&cbeqz).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.bnez %rs1, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b111).Setrs1Class(RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(&cbnez).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.li %rd, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b010).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&cli).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.lui %rd, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b011).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&CIFUNC).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.addi %rd, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b000).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&caddi).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.slli %rd, $imm").SetCost(1).SetOpcode(0b10).SetFunct3(0b000).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(&cslli).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.srli %rd, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b100).SetFunct2(0b00).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(&csrli).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.srai %rd, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b100).SetFunct2(0b01).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(&csrai).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.andi %rd, $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b100).SetFunct2(0b10).SetrdClass(RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(&candi).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.and %rd, %rs1").SetCost(1).SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b11).SetrdClass(RegGPR).Setrs2Class(RegGPR).SetFormat(RVCTypeCA).SetImplFunc(&cand).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.or %rd, %rs1").SetCost(1).SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b10).SetrdClass(RegGPR).Setrs2Class(RegGPR).SetFormat(RVCTypeCA).SetImplFunc(&cor).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.xor %rd, %rs1").SetCost(1).SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b01).SetrdClass(RegGPR).Setrs2Class(RegGPR).SetFormat(RVCTypeCA).SetImplFunc(&cxor).SetCompressed(true).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.sub %rd, %rs1").SetCost(1).SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b00).SetrdClass(RegGPR).Setrs2Class(RegGPR).SetFormat(RVCTypeCA).SetImplFunc(&csub).SetCompressed(true).InstEntry},
  };

  // RV32C-Only table
  std::vector<RevInstEntry> RV32ICOTable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("c.jal $imm").SetCost(1).SetOpcode(0b01).SetFunct3(0b001).SetrdClass(RegGPR).SetFormat(RVCTypeCJ).SetImplFunc(&cjal).SetCompressed(true).InstEntry},
  };

public:
  /// RV32I: standard constructor
  RV32I( RevFeature *Feature,
         RevRegFile *RegFile,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV32I", Feature, RegFile, RevMem, Output ) {
    this->SetTable(RV32ITable);
    this->SetCTable(RV32ICTable);
    this->SetOTable(RV32ICOTable);
  }

  /// RV32I: standard destructor
  ~RV32I() = default;

}; // end class RV32I

} // namespace SST::RevCPU

#endif
