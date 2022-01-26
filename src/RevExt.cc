//
// _RevExt_cc_
//
// Copyright (C) 2017-2021 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RevExt.h"

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

bool RevExt::Execute(unsigned Inst, RevInst payload, uint8_t threadID){

  // ensure that the target instruction is within scope
  if( Inst > (table.size()-1) ){
    output->fatal(CALL_INFO, -1,
                  "Error: instruction at index=%d does not exist in extension=%s",
                  Inst,
                  name.c_str());
  }

#if 0
  std::cout << "EXECUTING INSTRUCTION: " << table[Inst].mnemonic
            << " @ 0x" << std::hex << regFile[threadID].RV32_PC << std::dec
            << "; instSize = " << payload.instSize << std::endl;
#endif

  // retrieve the function pointer for this instruction
  bool (*func)(RevFeature *, RevRegFile *, RevMem *, RevInst) = table[Inst].func;

  // execution the instruction
  if( !(*func)(feature,&(regFile[threadID]),mem,payload) )
    return false;

#if 0
  std::cout << "COMPLETING INSTRUCTION: " << table[Inst].mnemonic
            << " @ 0x" << std::hex << regFile[threadID].RV32_PC << std::dec << std::endl;
#endif

  return true;
}

// EOF
