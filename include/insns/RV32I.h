//
// _RV32I_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_RV32I_H_
#define _SST_REVCPU_RV32I_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>
#include <functional>
#include <type_traits>

namespace SST::RevCPU{

class RV32I : public RevExt {

  // Compressed instructions
  static bool caddi4spn(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.addi4spn rd, $imm == addi rd, x2, $imm
    //Inst.rs1  = 2;  //Removed - Set in Decode
    //Inst.rd   = CRegIdx(Inst.rd);  //Set in Decode

    // if Inst.imm == 0; this is a HINT instruction
    // this is effectively a NOP
    if( Inst.imm == 0x00 ){
      R->AdvancePC(Inst);
      return true;
    }
    //Inst.imm = (Inst.imm & 0b011111111)*4;
    Inst.imm = (Inst.imm & 0b11111111)*4;
    return addi(F, R, M, Inst);
  }

  static bool clwsp(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.lwsp rd, $imm = lw rd, x2, $imm
    //Inst.rs1  = 2; //Removed - set in decode
    //Inst.imm = ((Inst.imm & 0b111111)*4);
    Inst.imm = (Inst.imm & 0b11111111); // Immd is 8 bits -  bits placed correctly in decode, no need to scale

    return lw(F, R, M, Inst);
  }

  static bool cswsp(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.swsp rs2, $imm = sw rs2, x2, $imm
    //Inst.rs1  = 2;  //Removed - set in decode
    //Inst.imm = ((Inst.imm & 0b111111)*4);
    Inst.imm = (Inst.imm & 0b11111111); // Immd is 8 bits - zero extended, bits placed correctly in decode, no need to scale

    return sw(F, R, M, Inst);
  }

  static bool clw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.lw rd, rs1, $imm = lw rd, $imm(rs1)
    //Inst.rd  = CRegIdx(Inst.rd);    //Removed - Scaled in decode
    //Inst.rs1 = CRegIdx(Inst.rs1);   //Removed - Scaled in decode
    //Inst.imm = ((Inst.imm & 0b11111)*4);
    Inst.imm = (Inst.imm & 0b1111111); // Immd is 7 bits, zero extended, bits placed correctly in decode, no need to scale

    return lw(F, R, M, Inst);
  }

  static bool csw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.sw rs2, rs1, $imm = sw rs2, $imm(rs1)
    //Inst.rs2 = CRegIdx(Inst.rd);  //Removed - Scaled in Decode
    //Inst.rs1 = CRegIdx(Inst.rs1); //Removed - Scaled in Decode
    //Inst.imm = ((Inst.imm & 0b11111)*4);
    Inst.imm = (Inst.imm & 0b1111111); //Immd is 7-bits, zero extended, bits placed correctly in decode, no need to scale

    return sw(F, R, M, Inst);
  }

  static bool cj(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.j $imm = jal x0, $imm
    Inst.rd = 0; // x0

    Inst.imm = Inst.jumpTarget;
    Inst.imm = Inst.ImmSignExt(12);
    return jal(F, R, M, Inst);
  }

  static bool cjal(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.jal $imm = jal x0, $imm
    //Inst.rd = 1; // x1 //Removed - set in decode
    Inst.imm = Inst.jumpTarget;

    return jal(F, R, M, Inst);
  }

  static bool CRFUNC_1000(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst){
    if( Inst.rs2 != 0 ){
      return cmv(F, R, M, Inst);
    }
    return cjr(F, R, M, Inst);
  }

  static bool CRFUNC_1001(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst){
    RevInst Inst = CInst;

    if( (Inst.rs1 == 0) && (Inst.rd == 0) ){
      return ebreak(F, R, M, Inst);
    }else if( (Inst.rs2 == 0) && (Inst.rd != 0) ){
      Inst.rd = 1;  //C.JALR expands to jalr x1, 0(rs1), so force update of x1 / ra
      return jalr(F, R, M, Inst);
    }else{
      return add(F, R, M, Inst);
    }
  }

  static bool cjr(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.jr %rs1 = jalr x0, 0(%rs1)
    Inst.rs2 = 0;
    return jalr(F, R, M, Inst);
  }

  static bool cmv(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    //Inst.rs1 = 0;  //Removed - performed in decode // expands to add rd, x0, rs2, so force rs1 to zero
    return add(F, R, M, Inst);
  }

  static bool cadd(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    Inst.rs1 = Inst.rd;
    return add(F, R, M, Inst);
  }

  static bool cjalr(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.jalr %rs1 = jalr x1, 0(%rs1)
    Inst.rs2 = 1;
    return jalr(F, R, M, Inst);
  }

  static bool cbeqz(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.beqz %rs1, $imm = beq %rs1, x0, $imm
    Inst.rs2 = 0;
   // Inst.rs1 = CRegIdx(Inst.rs1); // removed - scaled in decode
    Inst.imm = Inst.offset;
    Inst.imm = Inst.ImmSignExt(9);
    //Inst.imm = Inst.offset & 0b111111;
    //SEXT(Inst.imm, Inst.offset&0b111111111, 9); //Immd is signed 9-bit, scaled in decode
    //SEXT(Inst.imm, Inst.offset, 6);

    return beq(F, R, M, Inst);
  }

  static bool cbnez(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.bnez %rs1, $imm = bne %rs1, x0, $imm
    //Inst.rs2 = 0; //removed - set in decode
   // Inst.rs1 = CRegIdx(Inst.rs1); //removed - scaled in decode
    Inst.imm = Inst.offset;
    Inst.imm = Inst.ImmSignExt(9);  //Immd is signed 9-bit, scaled in decode
    //Inst.imm = Inst.offset & 0b111111;
    //SEXT(Inst.imm, Inst.offset, 6);
    //SEXT(Inst.imm, Inst.offset&0b111111111, 9); //Immd is signed 9-bit, scaled in decode

    return bne(F, R, M, Inst);
  }

  static bool cli(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.li %rd, $imm = addi %rd, x0, $imm
    //Inst.rs1 = 0; //removed - set in decode
    // SEXT(Inst.imm, (Inst.imm & 0b111111), 6);
    Inst.imm = Inst.ImmSignExt(6);
    return addi(F, R, M, Inst);
  }

  static bool CIFUNC(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    if( Inst.rd == 2 ){
      // c.addi16sp
      //SEXT(Inst.imm, (Inst.imm & 0b011111111)*16, 32);
      //SEXT(Inst.imm, (Inst.imm & 0b111111)*16, 6);
      // SEXT(Inst.imm, (Inst.imm & 0b1111111111), 10); // Immd is 10 bits, sign extended and scaled in decode
      Inst.imm = Inst.ImmSignExt(10);
      return addi(F, R, M, Inst);
    }else{
      // c.lui %rd, $imm = addi %rd, x0, $imm
      Inst.imm = Inst.ImmSignExt(17);
      return lui(F, R, M, Inst);
    }
  }

  static bool caddi(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.addi %rd, $imm = addi %rd, %rd, $imm
    // uint32_t tmp = Inst.imm & 0b111111;
    Inst.imm = Inst.ImmSignExt(6);
    //Inst.rs1 = Inst.rd; //Removed, set in decode
    return addi(F, R, M, Inst);
  }

  static bool cslli(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.slli %rd, $imm = slli %rd, %rd, $imm
   // Inst.rs1 = Inst.rd;  //removed - set in decode
    return slli(F, R, M, Inst);
  }

  static bool csrli(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.srli %rd, $imm = srli %rd, %rd, $imm
    //Inst.rd  = CRegIdx(Inst.rd); //removed - set in decode
    Inst.rs1 = Inst.rd;
    return srli(F, R, M, Inst);
  }

  static bool csrai(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.srai %rd, $imm = srai %rd, %rd, $imm
   // Inst.rd  = CRegIdx(Inst.rd); //removed - set in decode
   // Inst.rs1 = Inst.rd; //Removed - set in decode
    return srai(F, R, M, Inst);
  }

  static bool candi(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.andi %rd, $imm = sandi %rd, %rd, $imm
    // Inst.rd  = CRegIdx(Inst.rd); //removed - scaled in decode
    // Inst.rs1 = Inst.rd;          //removed - set in decode
    Inst.imm = Inst.ImmSignExt(6);  //immd is 6 bits, sign extended no scaling needed
    return andi(F, R, M, Inst);
  }

  static bool cand(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.and %rd, %rs2 = and %rd, %rd, %rs2
   // Inst.rd  = CRegIdx(Inst.rd);//removed - scaled in decode
   // Inst.rs1 = Inst.rd;//removed - scaled in decode
   // Inst.rs2 = CRegIdx(Inst.rs2);//removed - scaled in decode
    return f_and(F, R, M, Inst);
  }

  static bool cor(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.or %rd, %rs2 = or %rd, %rd, %rs2
    //Inst.rd  = CRegIdx(Inst.rd);//removed - scaled in decode
    //Inst.rs1 = Inst.rd;//removed - scaled in decode
    //Inst.rs2 = CRegIdx(Inst.rs2);//removed - scaled in decode
    return f_or(F, R, M, Inst);
  }

  static bool cxor(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.xor %rd, %rs2 = xor %rd, %rd, %rs2
    //Inst.rd  = CRegIdx(Inst.rd);//removed - scaled in decode
    //Inst.rs1 = Inst.rd;//removed - scaled in decode
    //Inst.rs2 = CRegIdx(Inst.rs2);//removed - scaled in decode
    return f_xor(F, R, M, Inst);
  }

  static bool csub(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& CInst) {
    RevInst Inst = CInst;

    // c.sub %rd, %rs2 = sub %rd, %rd, %rs2
    //Inst.rd  = CRegIdx(Inst.rd);//removed - scaled in decode
    //Inst.rs1 = Inst.rd;//removed - scaled in decode
    //Inst.rs2  = CRegIdx(Inst.rs2);//removed - scaled in decode
    return sub(F, R, M, Inst);
  }

  // Standard instructions
  static bool lui(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetX(Inst.rd, static_cast<int32_t>(Inst.imm << 12));
    R->AdvancePC(Inst);
    return true;
  }

  static bool auipc(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    auto ui = static_cast<int32_t>(Inst.imm << 12);
    R->SetX(Inst.rd, ui + R->GetPC());
    R->AdvancePC(Inst);
    return true;
  }

  static bool jal(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->SetX(Inst.rd, R->GetPC() + Inst.instSize);
    R->SetPC(R->GetPC() + Inst.ImmSignExt(21));
    return true;
  }

  static bool jalr(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    auto ret = R->GetPC() + Inst.instSize;
    R->SetPC((R->GetX<uint64_t>(Inst.rs1) + Inst.ImmSignExt(12)) & -2);
    R->SetX(Inst.rd, ret);
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

  static bool fence(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    M->FenceMem(F->GetHartToExecID());
    R->AdvancePC(Inst);
    return true;  // temporarily disabled
  }

  static bool ecall(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst){
    /*
     * In reality this should be getting/setting a LOT of bits inside the
     * CSRs however because we are only concerned with ecall right now it's
     * not a concern.
     * NOTE: Normally you would have to check if you are currently executing in
     *       Supervisor mode already and set RV64_MEPC instead but we don't need
     *       to worry about machine mode with the ecalls we are supporting
     */

    R->SetSEPC();    // Save PC of instruction that raised exception
    R->SetSTVAL(0);  // MTVAL/STVAL unused for ecall and is set to 0
    R->SetSCAUSE(EXCEPTION_CAUSE::ECALL_USER_MODE);

    /*
     * Trap Handler is not implemented because we only have one exception
     * So we don't have to worry about setting `mtvec` reg
     */

    R->AdvancePC(Inst);
    return true;
  }

  static bool ebreak(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->AdvancePC(Inst);
    return true;
  }

  static bool csrrw(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->AdvancePC(Inst);
    return true;
  }

  static bool csrrs(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->AdvancePC(Inst);
    return true;
  }

  static bool csrrc(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->AdvancePC(Inst);
    return true;
  }

  static bool csrrwi(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->AdvancePC(Inst);
    return true;
  }

  static bool csrrsi(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->AdvancePC(Inst);
    return true;
  }

  static bool csrrci(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    R->AdvancePC(Inst);
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
    {RevInstDefaults().SetMnemonic("lui %rd, $imm"        ).SetOpcode(0b0110111).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeU).SetImplFunc(lui )},
    {RevInstDefaults().SetMnemonic("auipc %rd, $imm"      ).SetOpcode(0b0010111).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeU).SetImplFunc(auipc )},

    {RevInstDefaults().SetMnemonic("jal %rd, $imm"        ).SetOpcode(0b1101111).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeJ).SetImplFunc(jal )},
    {RevInstDefaults().SetMnemonic("jalr %rd, %rs1, $imm" ).SetOpcode(0b1100111).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(jalr )},

    {RevInstDefaults().SetMnemonic("beq %rs1, %rs2, $imm" ).SetOpcode(0b1100011).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(beq )},
    {RevInstDefaults().SetMnemonic("bne %rs1, %rs2, $imm" ).SetOpcode(0b1100011).SetFunct3(0b001).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(bne )},
    {RevInstDefaults().SetMnemonic("blt %rs1, %rs2, $imm" ).SetOpcode(0b1100011).SetFunct3(0b100).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(blt )},
    {RevInstDefaults().SetMnemonic("bge %rs1, %rs2, $imm" ).SetOpcode(0b1100011).SetFunct3(0b101).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(bge )},
    {RevInstDefaults().SetMnemonic("bltu %rs1, %rs2, $imm").SetOpcode(0b1100011).SetFunct3(0b110).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(bltu )},
    {RevInstDefaults().SetMnemonic("bgeu %rs1, %rs2, $imm").SetOpcode(0b1100011).SetFunct3(0b111).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeB).SetImplFunc(bgeu )},

    {RevInstDefaults().SetMnemonic("lb %rd, $imm(%rs1)"   ).SetOpcode(0b0000011).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(lb )},
    {RevInstDefaults().SetMnemonic("lh %rd, $imm(%rs1)"   ).SetOpcode(0b0000011).SetFunct3(0b001).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(lh )},
    {RevInstDefaults().SetMnemonic("lw %rd, $imm(%rs1)"   ).SetOpcode(0b0000011).SetFunct3(0b010).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(lw )},
    {RevInstDefaults().SetMnemonic("lbu %rd, $imm(%rs1)"  ).SetOpcode(0b0000011).SetFunct3(0b100).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(lbu )},
    {RevInstDefaults().SetMnemonic("lhu %rd, $imm(%rs1)"  ).SetOpcode(0b0000011).SetFunct3(0b101).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(lhu )},

    {RevInstDefaults().SetMnemonic("sb %rs2, $imm(%rs1)"  ).SetOpcode(0b0100011).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(sb )},
    {RevInstDefaults().SetMnemonic("sh %rs2, $imm(%rs1)"  ).SetOpcode(0b0100011).SetFunct3(0b001).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(sh )},
    {RevInstDefaults().SetMnemonic("sw %rs2, $imm(%rs1)"  ).SetOpcode(0b0100011).SetFunct3(0b010).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegIMM).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeS).SetImplFunc(sw )},

    {RevInstDefaults().SetMnemonic("addi %rd, %rs1, $imm" ).SetOpcode(0b0010011).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(addi )},
    {RevInstDefaults().SetMnemonic("slti %rd, %rs1, $imm" ).SetOpcode(0b0010011).SetFunct3(0b010).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(slti )},
    {RevInstDefaults().SetMnemonic("sltiu %rd, %rs1, $imm").SetOpcode(0b0010011).SetFunct3(0b011).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(sltiu )},
    {RevInstDefaults().SetMnemonic("xori %rd, %rs1, $imm" ).SetOpcode(0b0010011).SetFunct3(0b100).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(xori )},
    {RevInstDefaults().SetMnemonic("ori %rd, %rs1, $imm"  ).SetOpcode(0b0010011).SetFunct3(0b110).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(ori )},
    {RevInstDefaults().SetMnemonic("andi %rd, %rs1, $imm" ).SetOpcode(0b0010011).SetFunct3(0b111).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(andi )},

    {RevInstDefaults().SetMnemonic("slli %rd, %rs1, $imm" ).SetOpcode(0b0010011).SetFunct3(0b001).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(slli )},
    {RevInstDefaults().SetMnemonic("srli %rd, %rs1, $imm" ).SetOpcode(0b0010011).SetFunct3(0b101).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(srli )},
    {RevInstDefaults().SetMnemonic("srai %rd, %rs1, $imm" ).SetOpcode(0b0010011).SetFunct3(0b101).SetFunct2or7(0b0010000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FImm).SetFormat(RVTypeI).SetImplFunc(srai )},

    {RevInstDefaults().SetMnemonic("add %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(add )},
    {RevInstDefaults().SetMnemonic("sub %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b000).SetFunct2or7(0b0100000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(sub )},
    {RevInstDefaults().SetMnemonic("sll %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b001).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(sll )},
    {RevInstDefaults().SetMnemonic("slt %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b010).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(slt )},
    {RevInstDefaults().SetMnemonic("sltu %rd, %rs1, %rs2" ).SetOpcode(0b0110011).SetFunct3(0b011).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(sltu )},
    {RevInstDefaults().SetMnemonic("xor %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b100).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(f_xor )},
    {RevInstDefaults().SetMnemonic("srl %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b101).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(srl )},
    {RevInstDefaults().SetMnemonic("sra %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b101).SetFunct2or7(0b0100000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(sra )},
    {RevInstDefaults().SetMnemonic("or %rd, %rs1, %rs2"   ).SetOpcode(0b0110011).SetFunct3(0b110).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(f_or )},
    {RevInstDefaults().SetMnemonic("and %rd, %rs1, %rs2"  ).SetOpcode(0b0110011).SetFunct3(0b111).SetFunct2or7(0b0000000).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FUnk).SetFormat(RVTypeR).SetImplFunc(f_and )},
    {RevInstDefaults().SetMnemonic("fence"  ).SetOpcode(0b0001111).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(fence )},
    {RevInstDefaults().SetMnemonic("ecall" ).SetOpcode(0b1110011).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b000000000000).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(ecall )},
    {RevInstDefaults().SetMnemonic("ebreak").SetOpcode(0b1110011).SetFunct3(0b000).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b000000000001).Setimm(FEnc).SetFormat(RVTypeI).SetImplFunc(ebreak )},

    {RevInstDefaults().SetMnemonic("csrrw %rd, %rs1, $imm" ).SetOpcode(0b1110011).SetFunct3(0b001).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(csrrw )},
    {RevInstDefaults().SetMnemonic("csrrs %rd, %rs1, $imm" ).SetOpcode(0b1110011).SetFunct3(0b010).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(csrrs )},
    {RevInstDefaults().SetMnemonic("csrrc %rd, %rs1, $imm" ).SetOpcode(0b1110011).SetFunct3(0b011).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(csrrc )},
    {RevInstDefaults().SetMnemonic("csrrwi %rd, %rs1, $imm").SetOpcode(0b1110011).SetFunct3(0b101).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(csrrwi )},
    {RevInstDefaults().SetMnemonic("csrrsi %rd, %rs1, $imm").SetOpcode(0b1110011).SetFunct3(0b110).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(csrrsi )},
    {RevInstDefaults().SetMnemonic("csrrci %rd, %rs1, $imm").SetOpcode(0b1110011).SetFunct3(0b111).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(csrrci )},
  };

  // RV32C table
  std::vector<RevInstEntry> RV32ICTable = {
    {RevInstDefaults().SetMnemonic("c.addi4spn %rd, $imm").SetOpcode(0b00).SetFunct3(0b000).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCIW).SetImplFunc(caddi4spn).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.lwsp %rd, $imm").SetOpcode(0b10).SetFunct3(0b010).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(clwsp).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.swsp %rs2, $imm").SetOpcode(0b10).SetFunct3(0b110).Setrs2Class(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCSS).SetImplFunc(cswsp).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.lw %rd, $rs1, $imm").SetOpcode(0b00).SetFunct3(0b010).Setrs1Class(RevRegClass::RegGPR).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCL).SetImplFunc(clw).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.sw %rs2, %rs1, $imm").SetOpcode(0b00).SetFunct3(0b110).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCS).SetImplFunc(csw).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.j $imm").SetOpcode(0b01).SetFunct3(0b101).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCJ).SetImplFunc(cj).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.jr %rs1").SetOpcode(0b10).SetFunct4(0b1000).Setrs1Class(RevRegClass::RegGPR).SetFormat(RVCTypeCR).SetImplFunc(CRFUNC_1000).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.jalr %rs1").SetOpcode(0b10).SetFunct4(0b1001).Setrs1Class(RevRegClass::RegGPR).SetFormat(RVCTypeCR).SetImplFunc(CRFUNC_1001).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.beqz %rs1, $imm").SetOpcode(0b01).SetFunct3(0b110).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(cbeqz).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.bnez %rs1, $imm").SetOpcode(0b01).SetFunct3(0b111).Setrs1Class(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(cbnez).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.li %rd, $imm").SetOpcode(0b01).SetFunct3(0b010).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(cli).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.lui %rd, $imm").SetOpcode(0b01).SetFunct3(0b011).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(CIFUNC).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.addi %rd, $imm").SetOpcode(0b01).SetFunct3(0b000).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(caddi).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.slli %rd, $imm").SetOpcode(0b10).SetFunct3(0b000).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCI).SetImplFunc(cslli).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.srli %rd, $imm").SetOpcode(0b01).SetFunct3(0b100).SetFunct2(0b00).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(csrli).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.srai %rd, $imm").SetOpcode(0b01).SetFunct3(0b100).SetFunct2(0b01).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(csrai).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.andi %rd, $imm").SetOpcode(0b01).SetFunct3(0b100).SetFunct2(0b10).SetrdClass(RevRegClass::RegGPR).Setimm(FVal).SetFormat(RVCTypeCB).SetImplFunc(candi).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.and %rd, %rs1").SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b11).SetrdClass(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).SetFormat(RVCTypeCA).SetImplFunc(cand).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.or %rd, %rs1").SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b10).SetrdClass(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).SetFormat(RVCTypeCA).SetImplFunc(cor).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.xor %rd, %rs1").SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b01).SetrdClass(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).SetFormat(RVCTypeCA).SetImplFunc(cxor).SetCompressed(true)},
    {RevInstDefaults().SetMnemonic("c.sub %rd, %rs1").SetOpcode(0b01).SetFunct6(0b100011).SetFunct2(0b00).SetrdClass(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegGPR).SetFormat(RVCTypeCA).SetImplFunc(csub).SetCompressed(true)},
  };

  // RV32C-Only table
  std::vector<RevInstEntry> RV32ICOTable = {
    { RevCInstDefaults().SetMnemonic("c.jal $imm").SetOpcode(0b01).SetFunct3(0b001).SetrdClass(RevRegClass::RegGPR).SetFormat(RVCTypeCJ).SetImplFunc(cjal) },
  };

public:
  /// RV32I: standard constructor
  RV32I( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "RV32I", Feature, RevMem, Output ) {
    SetTable(std::move(RV32ITable));
    SetCTable(std::move(RV32ICTable));
    SetOTable(std::move(RV32ICOTable));
  }

}; // end class RV32I

} // namespace SST::RevCPU

#endif
