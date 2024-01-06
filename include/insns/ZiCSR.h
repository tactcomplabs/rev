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

  /// Modify a CSR Register according to CSRRW, CSRRS, or CSRRC
  // Because CSR has a 32/64-bit width, this function is templatized
  template<typename T, CSRKind KIND, bool IsImm>
  static bool ModCSR(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    T old = 0;

    // CSRRW with rd == zero does not read CSR
    if(KIND != CSRKind::Write || Inst.rd != 0){
      old = R->GetCSR<T>(Inst.imm);
    }

    // Operand is an zero-extended 5-bit immediate or register
    T val = IsImm ? ZeroExt(Inst.rs1, 5) : R->GetX<T>(Inst.rs1);

    // Store the old CSR value in rd
    if(Inst.rd != 0)
      R->SetX(Inst.rd, old);

    // Handle CSRRS/CSRRC
    if(KIND != CSRKind::Write){
      if(Inst.rs1 == 0){
        return true;     // If CSRRS/CSRRC rs1 == 0, do not modify CSR
      }else if(KIND == CSRKind::Set){
        val = old |  val;
      }else{
        val = old & ~val;
      }
    }

    // Write the new CSR value
    R->SetCSR(Inst.imm, val);
    return true;
  }

  /// Modify a CSR Register according to CSRRW, CSRRS, or CSRRC
  // This calls the 32/64-bit ModCSR depending on the current XLEN
  template<CSRKind KIND, bool IsImm>
  static bool RevModCSR(RevFeature *F, RevRegFile *R, RevMem *M, const RevInst& Inst) {
    bool ret;
    if(R->IsRV32){
      ret = ModCSR<uint32_t, KIND, IsImm>(F, R, M, Inst);
    }else{
      ret = ModCSR<uint64_t, KIND, IsImm>(F, R, M, Inst);
    }
    R->AdvancePC(Inst);
    return ret;
  }

  static constexpr auto& csrrw  = RevModCSR<CSRKind::Write, false>;
  static constexpr auto& csrrs  = RevModCSR<CSRKind::Set,   false>;
  static constexpr auto& csrrc  = RevModCSR<CSRKind::Clear, false>;
  static constexpr auto& csrrwi = RevModCSR<CSRKind::Write,  true>;
  static constexpr auto& csrrsi = RevModCSR<CSRKind::Set,    true>;
  static constexpr auto& csrrci = RevModCSR<CSRKind::Clear,  true>;

  // ----------------------------------------------------------------------
  //
  // RISC-V CSR Instructions
  //
  // Format:
  // <mnemonic> <cost> <opcode> <funct3> <funct7> <rdClass> <rs1Class>
  //            <rs2Class> <rs3Class> <format> <func> <nullEntry>
  // ----------------------------------------------------------------------
#define INIT                                    \
  SetOpcode(0b1110011).                         \
  SetRaiseFPE().                                \
  SetFunct2or7(0).                              \
  SetrdClass(RevRegClass::RegGPR).              \
  Setrs2Class(RevRegClass::RegUNKNOWN).         \
  Setimm12(0b0).                                \
  Setimm(FVal).                                 \
  SetFormat(RVTypeI)

  std::vector<RevInstEntry> ZiCSRTable = {
    { RevInstEntryBuilder<RevInstDefaults>().INIT.SetMnemonic("csrrw %csr, %rd, %rs1" ).SetFunct3(0b001).SetImplFunc(csrrw ).InstEntry },
    { RevInstEntryBuilder<RevInstDefaults>().INIT.SetMnemonic("csrrs %csr, %rd, %rs1" ).SetFunct3(0b010).SetImplFunc(csrrs ).InstEntry },
    { RevInstEntryBuilder<RevInstDefaults>().INIT.SetMnemonic("csrrc %csr, %rd, %rs1" ).SetFunct3(0b011).SetImplFunc(csrrc ).InstEntry },
    { RevInstEntryBuilder<RevInstDefaults>().INIT.SetMnemonic("csrrwi %csr, %rd, $imm").SetFunct3(0b101).SetImplFunc(csrrwi).InstEntry },
    { RevInstEntryBuilder<RevInstDefaults>().INIT.SetMnemonic("csrrsi %csr, %rd, $imm").SetFunct3(0b110).SetImplFunc(csrrsi).InstEntry },
    { RevInstEntryBuilder<RevInstDefaults>().INIT.SetMnemonic("csrrci %csr, %rd, $imm").SetFunct3(0b111).SetImplFunc(csrrci).InstEntry },
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
