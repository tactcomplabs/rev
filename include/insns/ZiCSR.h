//
// _CSR_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_REVCPU_ZICSR_H_
#define _SST_REVCPU_ZICSR_H_

#include "../RevInstHelpers.h"
#include "../RevExt.h"

#include <vector>
#include <functional>
#include <type_traits>

namespace SST::RevCPU{

class ZiCSR : public RevExt {

  enum class CSRKind { Write, Set, Clear };

  template<typename T, CSRKind KIND, bool IsImm>
  static bool RevCSR(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    // Get the old value of the CSR
    T old = R->GetCSR(Inst.imm);

    // Operand is an zero-extended 5-bit immediate or register
    T val = IsImm ? ZeroExt(Inst.rs1, 5) : R->GetX<T>(Inst.rs1);

    // Store the old CSR value in rd (ignored if rd == 0)
    R->SetX(Inst.rd, old);

    if constexpr(KIND == CSRKind::Write){
      if(Inst.rd != 0 && Inst.rs1 == 0)
        val = 0;       // If CSRRW rs1 == 0, zero out value
    }else if(Inst.rs1 == 0){
      return true;     // If CSRR[SC] rs1 == 0, do not modify CSR
    }else if constexpr(KIND == CSRKind::Set){
      val = old |  val;
    }else if constexpr(KIND == CSRKind::Clear){
      val = old & ~val;
    }

    R->SetCSR(Inst.imm, val);
    return true;
  }

  static constexpr auto& csrrw  = RevCSR<uint32_t, CSRKind::Write, false>;
  static constexpr auto& csrrs  = RevCSR<uint32_t, CSRKind::Set,   false>;
  static constexpr auto& csrrc  = RevCSR<uint32_t, CSRKind::Clear, false>;
  static constexpr auto& csrrwi = RevCSR<uint32_t, CSRKind::Write,  true>;
  static constexpr auto& csrrsi = RevCSR<uint32_t, CSRKind::Set,    true>;
  static constexpr auto& csrrci = RevCSR<uint32_t, CSRKind::Clear,  true>;

  // ----------------------------------------------------------------------
  //
  // RISC-V CSR Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  std::vector<RevInstEntry> ZiCSRTable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrw %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b001).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrs %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b010).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrs ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrc %rd, %rs1, $imm" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b011).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrc ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrwi %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b101).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrwi ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrsi %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b110).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrsi ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrci %rd, %rs1, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b111).SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeU).SetImplFunc(&csrrci ).InstEntry},
  };

public:
  /// ZiCSR: standard constructor
  ZiCSR( RevFeature *Feature,
         RevMem *RevMem,
         SST::Output *Output )
    : RevExt( "ZiCSR", Feature, RevMem, Output ) {
    SetTable(std::move(ZiCSRTable));
  }

}; // end class ZiCSR

} // namespace SST::RevCPU

#endif
