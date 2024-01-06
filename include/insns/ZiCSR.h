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
  static bool RevCSRImpl(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    // CSRRW zero is a NOP
    if(KIND == CSRKind::Write && Inst.rd == 0)
      return true;

    // Get the old value of the CSR
    T old = R->GetCSR<T>(Inst.imm);

    // Operand is an zero-extended 5-bit immediate or register
    T val = IsImm ? ZeroExt(Inst.rs1, 5) : R->GetX<T>(Inst.rs1);

    // Store the old CSR value in rd
    R->SetX(Inst.rd, old);

    if(Inst.rs1 == 0){
      if(KIND == CSRKind::Write){
        val = 0;         // If CSRRW rs1 == 0, store 0 in destination
      }else{
        return true;     // If CSRR[SC] rs1 == 0, do not modify CSR
      }
    }else if(KIND == CSRKind::Set){
      val = old |  val;
    }else if(KIND == CSRKind::Clear){
      val = old & ~val;
    }
    R->SetCSR(Inst.imm, val);
    return true;
  }

  template<CSRKind KIND, bool IsImm>
  static bool RevCSR(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    bool ret;
    if(R->IsRV32){
      ret = RevCSRImpl<uint32_t, KIND, IsImm>(F, R, M, Inst);
    }else{
      ret = RevCSRImpl<uint64_t, KIND, IsImm>(F, R, M, Inst);
    }
    R->AdvancePC(Inst);
    return ret;
  }

  static constexpr auto& csrrw  = RevCSR<CSRKind::Write, false>;
  static constexpr auto& csrrs  = RevCSR<CSRKind::Set,   false>;
  static constexpr auto& csrrc  = RevCSR<CSRKind::Clear, false>;
  static constexpr auto& csrrwi = RevCSR<CSRKind::Write,  true>;
  static constexpr auto& csrrsi = RevCSR<CSRKind::Set,    true>;
  static constexpr auto& csrrci = RevCSR<CSRKind::Clear,  true>;

  // ----------------------------------------------------------------------
  //
  // RISC-V CSR Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
  std::vector<RevInstEntry> ZiCSRTable = {
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrw %csr, %rd, %rs1" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b001).SetRaiseFPE().SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(&csrrw ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrs %csr, %rd, %rs1" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b010).SetRaiseFPE().SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetRaiseFPE().SetImplFunc(&csrrs ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrc %csr, %rd, %rs1" ).SetCost(1).SetOpcode(0b1110011).SetFunct3(0b011).SetRaiseFPE().SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(&csrrc ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrwi %csr, %rd, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b101).SetRaiseFPE().SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(&csrrwi ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrsi %csr, %rd, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b110).SetRaiseFPE().SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(&csrrsi ).InstEntry},
    {RevInstEntryBuilder<RevInstDefaults>().SetMnemonic("csrrci %csr, %rd, $imm").SetCost(1).SetOpcode(0b1110011).SetFunct3(0b111).SetRaiseFPE().SetFunct2or7(0b0).SetrdClass(RevRegClass::RegGPR).Setrs1Class(RevRegClass::RegGPR).Setrs2Class(RevRegClass::RegUNKNOWN).Setrs3Class(RevRegClass::RegUNKNOWN).Setimm12(0b0).Setimm(FVal).SetFormat(RVTypeI).SetImplFunc(&csrrci ).InstEntry},
  };

public:
  /// ZiCSR: standard constructor
  ZiCSR( RevFeature *Feature, RevMem *RevMem, SST::Output *Output )
    : RevExt( "ZiCSR", Feature, RevMem, Output ) {
    SetTable(std::move(ZiCSRTable));
  }

}; // end class ZiCSR

} // namespace SST::RevCPU

#endif
