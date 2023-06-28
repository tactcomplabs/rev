//
// _RevExt_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "../include/RevExt.h"

RevExt::RevExt( std::string Name,
                RevFeature *Feature,
                RevRegFile *RegFile,
                RevMem *RevMem,
                SST::Output *Output )
  : feature(Feature), mem(RevMem), name(Name),
    output(Output) {
  regFile = RegFile;
}

RevExt::~RevExt(){
}

void RevExt::SetTable(std::vector<RevInstEntry> InstVect){
  table = InstVect;
}

void RevExt::SetCTable(std::vector<RevInstEntry> InstVect){
  ctable = InstVect;
}

void RevExt::SetOTable(std::vector<RevInstEntry> InstVect){
  otable = InstVect;
}

bool RevExt::Execute(unsigned Inst, RevInst payload, uint16_t HartID){

  // ensure that the target instruction is within scope
  if( Inst > (table.size()-1) ){
    output->fatal(CALL_INFO, -1,
                  "Error: instruction at index=%d does not exist in extension=%s",
                  Inst,
                  name.c_str());
  }

  bool (*func)(RevFeature *,
               RevRegFile *,
               RevMem *,
               RevInst) = nullptr;
  if( payload.compressed ){
#if 0
    if( feature->IsRV32() ){
      std::cout << "EXECUTING COMPRESSED INSTRUCTION: " << ctable[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV32_PC << std::dec
                << "; instSize = " << payload.instSize << std::endl;
    }else{
      std::cout << "EXECUTING COMPRESSED INSTRUCTION: " << ctable[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV64_PC << std::dec
                << "; instSize = " << payload.instSize << std::endl;
    }
#endif
    // this is a compressed instruction, grab the compressed trampoline function
    func = ctable[Inst].func;
  }else{
#if 0
    if( feature->IsRV32() ){
      std::cout << "EXECUTING INSTRUCTION: " << table[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV32_PC << std::dec
                << "; instSize = " << payload.instSize << std::endl;
    }else{
      std::cout << "EXECUTING INSTRUCTION: " << table[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV64_PC << std::dec
                << "; instSize = " << payload.instSize << std::endl;
    }
#endif
    // retrieve the function pointer for this instruction
    func = table[Inst].func;
  }

  // execute the instruction
  if( !(*func)(feature,(regFile),mem,payload) )
    return false;

#if 0
  if( payload.compressed ){
    if( feature->IsRV32() ){
      std::cout << "COMPLETING INSTRUCTION: " << ctable[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV32_PC-payload.instSize << std::dec << std::endl;
    }else{
      std::cout << "COMPLETING INSTRUCTION: " << ctable[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV64_PC-payload.instSize << std::dec << std::endl;
    }
  }else{
    if( feature->IsRV32() ){
      std::cout << "COMPLETING INSTRUCTION: " << table[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV32_PC-payload.instSize << std::dec << std::endl;
    }else{
      std::cout << "COMPLETING INSTRUCTION: " << table[Inst].mnemonic
                << " @ 0x" << std::hex << regFile[threadID].RV64_PC-payload.instSize << std::dec << std::endl;
    }
  }
#endif

  return true;
}

// EOF
