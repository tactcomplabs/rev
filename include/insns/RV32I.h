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

#include "../RevExt.h"
#include "../RevInstHelpers.h"

#include <functional>
#include <type_traits>
#include <vector>

namespace SST::RevCPU {

class RV32I : public RevExt {
  // Standard instructions
  static bool nop( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->AdvancePC( Inst );
    return true;
  }

  static bool lui( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetX( Inst.rd, static_cast<int32_t>( Inst.imm << 12 ) );
    R->AdvancePC( Inst );
    return true;
  }

  static bool auipc( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    auto ui = static_cast<int32_t>( Inst.imm << 12 );
    R->SetX( Inst.rd, ui + R->GetPC() );
    R->AdvancePC( Inst );
    return true;
  }

  static bool jal( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    R->SetX( Inst.rd, R->GetPC() + Inst.instSize );
    R->SetPC( R->GetPC() + Inst.ImmSignExt( 21 ) );
    return true;
  }

  static bool jalr( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    auto ret = R->GetPC() + Inst.instSize;
    R->SetPC( ( R->GetX<uint64_t>( Inst.rs1 ) + Inst.ImmSignExt( 12 ) ) & -2 );
    R->SetX( Inst.rd, ret );
    return true;
  }

  // Conditional branches
  static constexpr auto& beq   = bcond<std::equal_to>;
  static constexpr auto& bne   = bcond<std::not_equal_to>;
  static constexpr auto& blt   = bcond<std::less, std::make_signed_t>;
  static constexpr auto& bltu  = bcond<std::less, std::make_unsigned_t>;
  static constexpr auto& bge   = bcond<std::greater_equal, std::make_signed_t>;
  static constexpr auto& bgeu  = bcond<std::greater_equal, std::make_unsigned_t>;

  // Loads
  static constexpr auto& lb    = load<int8_t>;
  static constexpr auto& lh    = load<int16_t>;
  static constexpr auto& lw    = load<int32_t>;
  static constexpr auto& lbu   = load<uint8_t>;
  static constexpr auto& lhu   = load<uint16_t>;

  // Stores
  static constexpr auto& sb    = store<uint8_t>;
  static constexpr auto& sh    = store<uint16_t>;
  static constexpr auto& sw    = store<uint32_t>;

  // Arithmetic operators
  static constexpr auto& add   = oper<std::plus, OpKind::Reg>;
  static constexpr auto& addi  = oper<std::plus, OpKind::Imm>;
  static constexpr auto& sub   = oper<std::minus, OpKind::Reg>;
  static constexpr auto& f_xor = oper<std::bit_xor, OpKind::Reg>;
  static constexpr auto& xori  = oper<std::bit_xor, OpKind::Imm>;
  static constexpr auto& f_or  = oper<std::bit_or, OpKind::Reg>;
  static constexpr auto& ori   = oper<std::bit_or, OpKind::Imm>;
  static constexpr auto& f_and = oper<std::bit_and, OpKind::Reg>;
  static constexpr auto& andi  = oper<std::bit_and, OpKind::Imm>;

  // Boolean test and set operators
  static constexpr auto& slt   = oper<std::less, OpKind::Reg>;
  static constexpr auto& slti  = oper<std::less, OpKind::Imm>;
  static constexpr auto& sltu  = oper<std::less, OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& sltiu = oper<std::less, OpKind::Imm, std::make_unsigned_t>;

  // Shift operators
  static constexpr auto& slli  = oper<ShiftLeft, OpKind::Imm, std::make_unsigned_t>;
  static constexpr auto& srli  = oper<ShiftRight, OpKind::Imm, std::make_unsigned_t>;
  static constexpr auto& srai  = oper<ShiftRight, OpKind::Imm>;
  static constexpr auto& sll   = oper<ShiftLeft, OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& srl   = oper<ShiftRight, OpKind::Reg, std::make_unsigned_t>;
  static constexpr auto& sra   = oper<ShiftRight, OpKind::Reg>;

  static bool fence( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    M->FenceMem( F->GetHartToExecID() );
    R->AdvancePC( Inst );
    return true;  // temporarily disabled
  }

  static bool ecall( RevFeature* F, RevRegFile* R, RevMem* M, const RevInst& Inst ) {
    /*
     * In reality this should be getting/setting a LOT of bits inside the
     * CSRs however because we are only concerned with ecall right now it's
     * not a concern.
     * NOTE: Normally you would have to check if you are currently executing in
     *       Supervisor mode already and set RV64_MEPC instead but we don't need
     *       to worry about machine mode with the ecalls we are supporting
     */

    R->SetSEPC();      // Save PC of instruction that raised exception
    R->SetSTVAL( 0 );  // MTVAL/STVAL unused for ecall and is set to 0
    R->SetSCAUSE( RevExceptionCause::ECALL_USER_MODE );

    /*
     * Trap Handler is not implemented because we only have one exception
     * So we don't have to worry about setting `mtvec` reg
     */

    R->AdvancePC( Inst );
    return true;
  }

  static constexpr auto& ebreak    = nop;

  // Compressed instructions

  // c.addi4spn %rd, $imm == addi %rd, x2, $imm, $imm != 0
  static constexpr auto& caddi4spn = addi;

  // c.mv and c.jr. If c.mv %rd == x0 it is a HINT instruction
  static constexpr auto& cmv       = add;
  static constexpr auto& cjr       = jalr;

  // c.add, c.jalr and c.ebreak. If c.add %rd == x0 then it is a HINT instruction
  static constexpr auto& cadd      = add;
  static constexpr auto& cjalr     = jalr;
  static constexpr auto& cebreak   = ebreak;

  // c.lui and c.addi16sp
  static constexpr auto& clui      = lui;
  static constexpr auto& caddi16sp = addi;

  static constexpr auto& clwsp     = lw;
  static constexpr auto& cswsp     = sw;
  static constexpr auto& clw       = lw;
  static constexpr auto& csw       = sw;
  static constexpr auto& cj        = jal;
  static constexpr auto& cjal      = jal;
  static constexpr auto& cbeqz     = beq;
  static constexpr auto& cbnez     = bne;
  static constexpr auto& cli       = addi;
  static constexpr auto& caddi     = addi;
  static constexpr auto& cslli     = slli;
  static constexpr auto& csrli     = srli;
  static constexpr auto& csrai     = srai;
  static constexpr auto& candi     = andi;
  static constexpr auto& cand      = f_and;
  static constexpr auto& cor       = f_or;
  static constexpr auto& cxor      = f_xor;
  static constexpr auto& csub      = sub;

  // ----------------------------------------------------------------------
  //
  // RISC-V RV32I Instructions
  //
  // ----------------------------------------------------------------------

  // clang-format off
  std::vector<RevInstEntry> RV32ITable = {
    { RevInstDefaults().SetMnemonic("lui %rd, $imm"        ).SetFunct3(   0b000).SetImplFunc(lui   ).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeU).SetOpcode(0b0110111) },
    { RevInstDefaults().SetMnemonic("auipc %rd, $imm"      ).SetFunct3(   0b000).SetImplFunc(auipc ).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeU).SetOpcode(0b0010111) },
    { RevInstDefaults().SetMnemonic("jal %rd, $imm"        ).SetFunct3(   0b000).SetImplFunc(jal   ).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeJ).SetOpcode(0b1101111).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_RD( Inst ) != 1; } ) },
    { RevInstDefaults().SetMnemonic("jal $imm"             ).SetFunct3(   0b000).SetImplFunc(jal   ).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeJ).SetOpcode(0b1101111).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0 && DECODE_RD( Inst ) == 1; } ) },
    { RevInstDefaults().SetMnemonic("j $imm"               ).SetFunct3(   0b000).SetImplFunc(jal   ).Setrs1Class(RevRegClass::RegUNKNOWN).Setrs2Class(RevRegClass::RegUNKNOWN).SetFormat(RVTypeJ).SetOpcode(0b1101111).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },
    { RevInstDefaults().SetMnemonic("jalr %rd, %rs1, $imm" ).SetFunct3(   0b000).SetImplFunc(jalr  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b1100111).SetPredicate( []( uint32_t Inst ){ return DECODE_IMM12( Inst ) != 0 || DECODE_RD( Inst )  > 1; } ) },
    { RevInstDefaults().SetMnemonic("jalr %rs1"            ).SetFunct3(   0b000).SetImplFunc(jalr  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b1100111).SetPredicate( []( uint32_t Inst ){ return DECODE_IMM12( Inst ) == 0 && DECODE_RD( Inst ) == 1; } ) },
    { RevInstDefaults().SetMnemonic("jr %rs1"              ).SetFunct3(   0b000).SetImplFunc(jalr  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b1100111).SetPredicate( []( uint32_t Inst ){ return DECODE_IMM12( Inst ) == 0 && DECODE_RD( Inst ) == 0 && DECODE_RS1( Inst ) != 1; } ) },
    { RevInstDefaults().SetMnemonic("ret"                  ).SetFunct3(   0b000).SetImplFunc(jalr  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b1100111).SetPredicate( []( uint32_t Inst ){ return DECODE_IMM12( Inst ) == 0 && DECODE_RD( Inst ) == 0 && DECODE_RS1( Inst ) == 1; } ) },
    { RevInstDefaults().SetMnemonic("beq %rs1, %rs2, $imm" ).SetFunct3(   0b000).SetImplFunc(beq   ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeB).SetOpcode(0b1100011) },
    { RevInstDefaults().SetMnemonic("bne %rs1, %rs2, $imm" ).SetFunct3(   0b001).SetImplFunc(bne   ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeB).SetOpcode(0b1100011) },
    { RevInstDefaults().SetMnemonic("blt %rs1, %rs2, $imm" ).SetFunct3(   0b100).SetImplFunc(blt   ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeB).SetOpcode(0b1100011) },
    { RevInstDefaults().SetMnemonic("bge %rs1, %rs2, $imm" ).SetFunct3(   0b101).SetImplFunc(bge   ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeB).SetOpcode(0b1100011) },
    { RevInstDefaults().SetMnemonic("bltu %rs1, %rs2, $imm").SetFunct3(   0b110).SetImplFunc(bltu  ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeB).SetOpcode(0b1100011) },
    { RevInstDefaults().SetMnemonic("bgeu %rs1, %rs2, $imm").SetFunct3(   0b111).SetImplFunc(bgeu  ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeB).SetOpcode(0b1100011) },

    { RevInstDefaults().SetMnemonic("lb %rd, $imm(%rs1)"   ).SetFunct3(   0b000).SetImplFunc(lb    ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0000011) },
    { RevInstDefaults().SetMnemonic("lh %rd, $imm(%rs1)"   ).SetFunct3(   0b001).SetImplFunc(lh    ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0000011) },
    { RevInstDefaults().SetMnemonic("lw %rd, $imm(%rs1)"   ).SetFunct3(   0b010).SetImplFunc(lw    ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0000011) },
    { RevInstDefaults().SetMnemonic("lbu %rd, $imm(%rs1)"  ).SetFunct3(   0b100).SetImplFunc(lbu   ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0000011) },
    { RevInstDefaults().SetMnemonic("lhu %rd, $imm(%rs1)"  ).SetFunct3(   0b101).SetImplFunc(lhu   ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0000011) },

    { RevInstDefaults().SetMnemonic("sb %rs2, $imm(%rs1)"  ).SetFunct3(   0b000).SetImplFunc(sb    ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeS).SetOpcode(0b0100011) },
    { RevInstDefaults().SetMnemonic("sh %rs2, $imm(%rs1)"  ).SetFunct3(   0b001).SetImplFunc(sh    ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeS).SetOpcode(0b0100011) },
    { RevInstDefaults().SetMnemonic("sw %rs2, $imm(%rs1)"  ).SetFunct3(   0b010).SetImplFunc(sw    ).SetrdClass(RevRegClass::RegIMM)                                          .SetFormat(RVTypeS).SetOpcode(0b0100011) },

    { RevInstDefaults().SetMnemonic("addi %rd, %rs1, $imm" ).SetFunct3(   0b000).SetImplFunc(addi  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011).SetPredicate( []( uint32_t Inst ){ return DECODE_IMM12( Inst ) != 0; } ) },
    { RevInstDefaults().SetMnemonic("mv %rd, %rs1"         ).SetFunct3(   0b000).SetImplFunc(addi  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011).SetPredicate( []( uint32_t Inst ){ return DECODE_IMM12( Inst ) == 0 && ( DECODE_RS1( Inst ) != 0 || DECODE_RD( Inst ) != 0 ); } ) },
    { RevInstDefaults().SetMnemonic("nop"                  ).SetFunct3(   0b000).SetImplFunc(addi  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011).SetPredicate( []( uint32_t Inst ){ return DECODE_IMM12( Inst ) == 0 && ( DECODE_RS1( Inst ) == 0 && DECODE_RD( Inst ) == 0 ); } ) },
    { RevInstDefaults().SetMnemonic("slti %rd, %rs1, $imm" ).SetFunct3(   0b010).SetImplFunc(slti  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011) },
    { RevInstDefaults().SetMnemonic("sltiu %rd, %rs1, $imm").SetFunct3(   0b011).SetImplFunc(sltiu ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011).SetPredicate( []( uint32_t Inst ) { return DECODE_IMM12( Inst ) != 1; } ) },
    { RevInstDefaults().SetMnemonic("seqz %rd, %rs"        ).SetFunct3(   0b011).SetImplFunc(sltiu ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011).SetPredicate( []( uint32_t Inst ) { return DECODE_IMM12( Inst ) == 1; } ) },
    { RevInstDefaults().SetMnemonic("xori %rd, %rs1, $imm" ).SetFunct3(   0b100).SetImplFunc(xori  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011).SetPredicate( []( uint32_t Inst ) { return SignExt( DECODE_IMM12( Inst ), 12 ) != -1; } ) },
    { RevInstDefaults().SetMnemonic("not %rd, %rs"         ).SetFunct3(   0b100).SetImplFunc(xori  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011).SetPredicate( []( uint32_t Inst ) { return SignExt( DECODE_IMM12( Inst ), 12 ) == -1; } ) },
    { RevInstDefaults().SetMnemonic("ori %rd, %rs1, $imm"  ).SetFunct3(   0b110).SetImplFunc(ori   ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011) },
    { RevInstDefaults().SetMnemonic("andi %rd, %rs1, $imm" ).SetFunct3(   0b111).SetImplFunc(andi  ).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm)                        .SetFormat(RVTypeI).SetOpcode(0b0010011) },

    { RevInstDefaults().SetMnemonic("slli %rd, %rs1, $imm" ).SetFunct3(   0b001).SetImplFunc(slli  ).SetFunct2or7(0b0000000).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0010011) },
    { RevInstDefaults().SetMnemonic("srli %rd, %rs1, $imm" ).SetFunct3(   0b101).SetImplFunc(srli  ).SetFunct2or7(0b0000000).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0010011) },
    { RevInstDefaults().SetMnemonic("srai %rd, %rs1, $imm" ).SetFunct3(   0b101).SetImplFunc(srai  ).SetFunct2or7(0b0010000).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FImm).SetFormat(RVTypeI).SetOpcode(0b0010011) },

    { RevInstDefaults().SetMnemonic("add %rd, %rs1, %rs2"  ).SetFunct3(   0b000).SetImplFunc(add   ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("sub %rd, %rs1, %rs2"  ).SetFunct3(   0b000).SetImplFunc(sub   ).SetFunct2or7(0b0100000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("sll %rd, %rs1, %rs2"  ).SetFunct3(   0b001).SetImplFunc(sll   ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("slt %rd, %rs1, %rs2"  ).SetFunct3(   0b010).SetImplFunc(slt   ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("sltu %rd, %rs1, %rs2" ).SetFunct3(   0b011).SetImplFunc(sltu  ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) != 0; } ) },
    { RevInstDefaults().SetMnemonic("snez %rd, %rs"        ).SetFunct3(   0b011).SetImplFunc(sltu  ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011).SetPredicate( []( uint32_t Inst ){ return DECODE_RS1( Inst ) == 0; } ) },
    { RevInstDefaults().SetMnemonic("xor %rd, %rs1, %rs2"  ).SetFunct3(   0b100).SetImplFunc(f_xor ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("srl %rd, %rs1, %rs2"  ).SetFunct3(   0b101).SetImplFunc(srl   ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("sra %rd, %rs1, %rs2"  ).SetFunct3(   0b101).SetImplFunc(sra   ).SetFunct2or7(0b0100000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("or %rd, %rs1, %rs2"   ).SetFunct3(   0b110).SetImplFunc(f_or  ).SetFunct2or7(0b0000000)                                                  .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("and %rd, %rs1, %rs2"  ).SetFunct3(   0b111).SetImplFunc(f_and ).SetFunct2or7(0b0000000)
                          .SetFormat(RVTypeR).SetOpcode(0b0110011) },
    { RevInstDefaults().SetMnemonic("fence"                ).SetFunct3(   0b000).SetImplFunc(fence ).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegUNKNOWN) .SetFormat(RVTypeI).SetOpcode(0b0001111).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm(FVal) },
    { RevInstDefaults().SetMnemonic("ecall"                ).SetFunct3(   0b000).SetImplFunc(ecall ).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegUNKNOWN) .SetFormat(RVTypeI).SetOpcode(0b1110011).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm12(0b000000000000).Setimm(FEnc) },
    { RevInstDefaults().SetMnemonic("ebreak"               ).SetFunct3(   0b000).SetImplFunc(ebreak).SetrdClass(RevRegClass::RegUNKNOWN).Setrs1Class(RevRegClass::RegUNKNOWN) .SetFormat(RVTypeI).SetOpcode(0b1110011).Setrs2Class(RevRegClass::RegUNKNOWN).Setimm12(0b000000000001).Setimm(FEnc) },
  };

  // RV32C table
  std::vector<RevInstEntry> RV32ICTable = {
    { RevCInstDefaults().SetMnemonic("c.addi4spn %rd, $imm" ).SetFunct3(   0b000).SetImplFunc(caddi4spn  ).Setimm(FVal).SetFormat(RVCTypeCIW).SetOpcode(0b00).SetPredicate( []( uint32_t Inst ){ return ( ( Inst >> 5 ) & 0xff ) != 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.lwsp %rd, $imm"     ).SetFunct3(   0b010).SetImplFunc(clwsp      ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b10).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.swsp %rs2, $imm"    ).SetFunct3(   0b110).SetImplFunc(cswsp      ).Setimm(FVal).SetFormat(RVCTypeCSS).SetOpcode(0b10) },
    { RevCInstDefaults().SetMnemonic("c.lw %rd, $rs1, $imm" ).SetFunct3(   0b010).SetImplFunc(clw        ).Setimm(FVal).SetFormat(RVCTypeCL ).SetOpcode(0b00) },
    { RevCInstDefaults().SetMnemonic("c.sw %rs2, %rs1, $imm").SetFunct3(   0b110).SetImplFunc(csw        ).Setimm(FVal).SetFormat(RVCTypeCS ).SetOpcode(0b00) },
    { RevCInstDefaults().SetMnemonic("c.j $imm"             ).SetFunct3(   0b101).SetImplFunc(cj         ).Setimm(FVal).SetFormat(RVCTypeCJ ).SetOpcode(0b01) },
    { RevCInstDefaults().SetMnemonic("c.jr %rs1"            ).SetFunct4(  0b1000).SetImplFunc(cjr        )             .SetFormat(RVCTypeCR ).SetOpcode(0b10).SetPredicate( []( uint32_t Inst ){ return DECODE_LOWER_CRS2( Inst ) == 0 && DECODE_RD( Inst ) != 0 && DECODE_RD( Inst ) != 1; } ) },
    { RevCInstDefaults().SetMnemonic("ret"                  ).SetFunct4(  0b1000).SetImplFunc(cjr        )             .SetFormat(RVCTypeCR ).SetOpcode(0b10).SetPredicate( []( uint32_t Inst ){ return DECODE_LOWER_CRS2( Inst ) == 0 && DECODE_RD( Inst ) == 1; } ) },
    { RevCInstDefaults().SetMnemonic("c.mv %rd, %rs2"       ).SetFunct4(  0b1000).SetImplFunc(cmv        )             .SetFormat(RVCTypeCR ).SetOpcode(0b10).SetPredicate( []( uint32_t Inst ){ return DECODE_LOWER_CRS2( Inst ) != 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.add %rd, %rs1"      ).SetFunct4(  0b1001).SetImplFunc(cadd       )             .SetFormat(RVCTypeCR ).SetOpcode(0b10).SetPredicate( []( uint32_t Inst ){ return DECODE_LOWER_CRS2( Inst ) != 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.jalr %rs1"          ).SetFunct4(  0b1001).SetImplFunc(cjalr      )             .SetFormat(RVCTypeCR ).SetOpcode(0b10).SetPredicate( []( uint32_t Inst ){ return DECODE_LOWER_CRS2( Inst ) == 0 && DECODE_RD(Inst) != 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.ebreak"             ).SetFunct4(  0b1001).SetImplFunc(cebreak    )             .SetFormat(RVCTypeCR ).SetOpcode(0b10).SetPredicate( []( uint32_t Inst ){ return DECODE_LOWER_CRS2( Inst ) == 0 && DECODE_RD(Inst) == 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.beqz %rs1, $imm"    ).SetFunct3(   0b110).SetImplFunc(cbeqz      ).Setimm(FVal).SetFormat(RVCTypeCB ).SetOpcode(0b01) },
    { RevCInstDefaults().SetMnemonic("c.bnez %rs1, $imm"    ).SetFunct3(   0b111).SetImplFunc(cbnez      ).Setimm(FVal).SetFormat(RVCTypeCB ).SetOpcode(0b01) },
    { RevCInstDefaults().SetMnemonic("c.li %rd, $imm"       ).SetFunct3(   0b010).SetImplFunc(cli        ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b01) },
    { RevCInstDefaults().SetMnemonic("c.lui %rd, $imm"      ).SetFunct3(   0b011).SetImplFunc(clui       ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b01).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 2; } ) },
    { RevCInstDefaults().SetMnemonic("c.addi16sp $imm"      ).SetFunct3(   0b011).SetImplFunc(caddi16sp  ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b01).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 2; } ) },
    { RevCInstDefaults().SetMnemonic("c.addi %rd, $imm"     ).SetFunct3(   0b000).SetImplFunc(caddi      ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b01).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) != 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.nop"                ).SetFunct3(   0b000).SetImplFunc(nop        ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b01).SetPredicate( []( uint32_t Inst ){ return DECODE_RD( Inst ) == 0; } ) },
    { RevCInstDefaults().SetMnemonic("c.slli %rd, $imm"     ).SetFunct3(   0b000).SetImplFunc(cslli      ).Setimm(FVal).SetFormat(RVCTypeCI ).SetOpcode(0b10) },
    { RevCInstDefaults().SetMnemonic("c.srli %rd, $imm"     ).SetFunct3(   0b100).SetImplFunc(csrli      ).Setimm(FVal).SetFormat(RVCTypeCB ).SetOpcode(0b01).SetFunct2(0b00) },
    { RevCInstDefaults().SetMnemonic("c.srai %rd, $imm"     ).SetFunct3(   0b100).SetImplFunc(csrai      ).Setimm(FVal).SetFormat(RVCTypeCB ).SetOpcode(0b01).SetFunct2(0b01) },
    { RevCInstDefaults().SetMnemonic("c.andi %rd, $imm"     ).SetFunct3(   0b100).SetImplFunc(candi      ).Setimm(FVal).SetFormat(RVCTypeCB ).SetOpcode(0b01).SetFunct2(0b10) },
    { RevCInstDefaults().SetMnemonic("c.and %rd, %rs1"      ).SetFunct6(0b100011).SetImplFunc(cand       )             .SetFormat(RVCTypeCA ).SetOpcode(0b01).SetFunct2(0b11) },
    { RevCInstDefaults().SetMnemonic("c.or %rd, %rs1"       ).SetFunct6(0b100011).SetImplFunc(cor        )             .SetFormat(RVCTypeCA ).SetOpcode(0b01).SetFunct2(0b10) },
    { RevCInstDefaults().SetMnemonic("c.xor %rd, %rs1"      ).SetFunct6(0b100011).SetImplFunc(cxor       )             .SetFormat(RVCTypeCA ).SetOpcode(0b01).SetFunct2(0b01) },
    { RevCInstDefaults().SetMnemonic("c.sub %rd, %rs1"      ).SetFunct6(0b100011).SetImplFunc(csub       )             .SetFormat(RVCTypeCA ).SetOpcode(0b01).SetFunct2(0b00) },
  };

  // RV32C-Only table
  std::vector<RevInstEntry> RV32ICOTable = {
    { RevCInstDefaults().SetMnemonic("c.jal $imm").SetOpcode(0b01).SetFunct3(0b001).SetFormat(RVCTypeCJ).SetImplFunc(cjal) },
  };
  // clang-format on

public:
  /// RV32I: standard constructor
  RV32I( RevFeature* Feature, RevMem* RevMem, SST::Output* Output ) : RevExt( "RV32I", Feature, RevMem, Output ) {
    SetTable( std::move( RV32ITable ) );
    SetCTable( std::move( RV32ICTable ) );
    SetOTable( std::move( RV32ICOTable ) );
  }

};  // end class RV32I

}  // namespace SST::RevCPU

#endif
