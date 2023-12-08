
//
// _RDB_cc_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "RDB.h"


namespace SST::RevCPU{
  RDB::RDB(SST::Cycle_t firstBreak):
    breakCycle(firstBreak), cmdLine() {

    proc = nullptr;
    cmdLine.clear();
    
  }

  RDB::~RDB(){

  }

  void RDB::PrintHelp(){
    std::cout << "Supported commands:" << std::endl;
    std::cout << "\treg <hart> <num>  //Print Register value" << std::endl;
    std::cout << "\ts <num>           //Step forward num cycles - default is one cycle" << std::endl;
  }

  bool RDB::GetCommand(){

    //Supported commands:
    // reg <hart> <num>  //Print Register value
    // s <num>    //Step forward num cycles - default is one cycle

    std::string input;
    std::stringstream line;
    std::string field;
    bool rtn = true;
    std::cout << "Core " << proc->id << " PC: " << std::hex << proc->GetPC() << std::dec << " rdb% ";
    std::getline(std::cin, input);
    line.str(input);
    for(field; std::getline(line, field, ' ');){
      cmdLine.push_back(field);
    }

    if(cmdLine[0] == "reg"){
      PrintRegister();
      rtn = true;
    }else if(cmdLine[0] == "s"){
      Step();
      rtn = false;
    }else{
      std::cout << "INVALID COMMAND" << std::endl;
      PrintHelp();
      rtn = true;
    }

    cmdLine.clear();
    return rtn;
  }

  void RDB::Step(){
    int stepCount = 1;
    if(cmdLine.size() > 1){
      stepCount = std::stoi(cmdLine[1]);
    }
    breakCycle += stepCount;
  }

  void RDB::PrintRegister(){
    uint64_t val = 0;
    int reg = std::stoi(cmdLine[2]);
    int hart = std::stoi(cmdLine[1]);
    if(reg < _REV_NUM_REGS_){
      proc->DebugReadReg(reg, &val, hart);
      std::cout << "Core " << proc->id << ":" << hart << " x" << reg << " = " << std::hex << val << std::dec << std::endl;
    }else{
      std::cout << "Core " << proc->id << " x" << reg << " is out of range" << std::endl;
      if(proc->RegFile) {std::cout << *(proc->RegFile);}
    }
  }

} //namespace
